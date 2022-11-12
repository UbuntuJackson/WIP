#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "asset_manager.h"
#include "entity.h"
#include "actor.h"
#include "pingu.h"
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#undef min
#undef max

struct rect
{
    olc::vf2d pos;
    olc::vf2d size;
    olc::vf2d vel;
    std::array<rect*, 4> contact;
};

class Pingus;

class Actor{
    public:
        olc::vf2d pos;
		olc::vf2d size;
		olc::vf2d vel;

    Actor(olc::vf2d _pos){
        pos = _pos;
        size = {16.0, 24.0};
        vel = { 0.0, 0.0 };
    }
    virtual void update(Pingus* game, float _fElapsedTime);
    virtual void draw(Pingus* game, std::map<std::string, size_t>& texMap);
};

class Pingu : public Actor{
    public:
        rect playerrect;
        float grv;
        Pingu(olc::vf2d _pos) : Actor(_pos){
            grv = 1.0;
            playerrect = { pos, {16, 24}};
            playerrect.vel = { 0, 0 };
            std::cout << "create" << std::endl;
        }
        ~Pingu() {
            std::cout << "destroy" << std::endl;
        }
    virtual void update(Pingus* game, float _fElapsedTime);
    virtual void draw(Pingus* game, std::map<std::string, size_t>& texMap);
};

class Coin : public Actor{
    Coin(olc::vf2d _pos) : Actor(_pos){}
    void update(){}
    void draw(olc::PixelGameEngine* pge, std::map<std::string,size_t>&texMap) {
        auto& assets = AssetManager::Current();
        pge->DrawSprite(pos, assets.GetTexture(texMap["cointexture"])->sprite);
    }
};

class Pingus : public olc::PixelGameEngine
{
    public:
        Pingus()
        {
            sAppName = "Pingus";
        }
    public:
        AssetManager& assets = AssetManager::Current(); // Look here
        
        bool debugMode;
        int action;
        enum actions{
            DIG,
            BOMB,
            BASH
        };
        
        //Pingu* ppingu;

        std::vector<std::unique_ptr<Actor>> levelActors;
        std::vector<int> levelActions;
        olc::Sprite	*spriteMap;
        olc::Sprite	*spriteCollisionMap;
        olc::Sprite *spriteRedCircle;
        olc::Sprite *spriteRedCoin;
        olc::Sprite *spriteCoin;
        std::map<std::string,size_t> textureMap;

        rect lastCell;
        //std::vector<olc::vf2d> snapDownCells;

        bool CompareColour(olc::Pixel colour_a, olc::Pixel colour_b){
            if(colour_a.r == colour_b.r && colour_a.g == colour_b.g && colour_a.b == colour_b.b &&
                colour_a.a == colour_b.a
            ){
                return true;
            }
            return false;
        }

        std::vector<olc::vf2d> collidedPixles;

        void searchDownSlope(rect player, std::vector<olc::vf2d>* cells) {
            for (int y = int(player.pos.y + player.size.y); y < int(player.pos.y + player.size.y + 4.0f); y++) {
                for (int x = int(player.pos.x); x < int(player.pos.x + player.size.x + 1.0f); x++) {
                    if (CompareColour(assets.GetTexture(textureMap["collisionmap"])->sprite->GetPixel(x, y), olc::Pixel(69, 40, 60, 255))) {
                        cells -> push_back({ float(x), float(y)});
                    }
                }
            }
            
            //if()
        }

        bool WorldVsPlayer(rect *player) {
            auto& assets = AssetManager::Current();
            int x = int(player->pos.x)-1;
            for (int y = int(player -> pos.y); y < int(player -> pos.y) + int(player -> size.y); y++) {
                
                    if (CompareColour(assets.GetTexture(textureMap["collisionmap"])->sprite->GetPixel(x, y), olc::Pixel(69, 40, 60, 255))) {
                        collidedPixles.push_back({float(x), float(y)});

                        return true;
                    }
                
            }
            x = int(player->pos.x + player -> size.x);
            for (int y = int(player->pos.y); y < int(player->pos.y) + int(player->size.y); y++) {

                if (CompareColour(assets.GetTexture(textureMap["collisionmap"])->sprite->GetPixel(x, y), olc::Pixel(69, 40, 60, 255))) {
                    collidedPixles.push_back({ float(x), float(y)});

                    return true;
                }

            }
            return false;
        }

        void RemoveCell(int x, int y, olc::Sprite *sprmap, olc::Sprite *sprcollisionMap){
            sprmap -> SetPixel(x, y, olc::Pixel(0, 0, 0, 0));
        }
        void RemoveCircle(int x, int y, olc::Sprite *sprmap, olc::Sprite *sprcollisionMap, olc::Sprite *sprredCircle){
            for(int j = 0; j < sprredCircle -> height; j++){
                for(int i = 0; i < sprredCircle -> width; i++){
                    if(CompareColour(sprredCircle -> GetPixel(i, j), olc::Pixel(255, 0, 0, 255))){
                        RemoveCell(x+i,y+j, sprmap, sprcollisionMap);
                    }
                }
            }
        }

        bool RectVsRect(const rect* r1, const rect* r2)
        {
            return (r1->pos.x < r2->pos.x + r2->size.x && r1->pos.x + r1->size.x > r2->pos.x && r1->pos.y < r2->pos.y + r2->size.y && r1->pos.y + r1->size.y > r2->pos.y);
        }

        bool RayVsRect(const olc::vf2d& ray_origin, const olc::vf2d& ray_dir, const rect* target, olc::vf2d& contact_point, olc::vf2d& contact_normal, float& t_hit_near)
        {
            contact_normal = { 0,0 };
            contact_point = { 0,0 };

            // Cache division
            olc::vf2d invdir = 1.0f / ray_dir;

            // Calculate intersections with rectangle bounding axes
            olc::vf2d t_near = (target->pos - ray_origin) * invdir;
            olc::vf2d t_far = (target->pos + target->size - ray_origin) * invdir;
            if (std::isnan(t_far.y) || std::isnan(t_far.x)) return false;
            if (std::isnan(t_near.y) || std::isnan(t_near.x)) return false;
            
            // Sort distances
            if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
            if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);
            // Early rejection		
            if (t_near.x > t_far.y || t_near.y > t_far.x) return false;

            // Closest 'time' will be the first contact
            t_hit_near = std::max(t_near.x, t_near.y);

            // Furthest 'time' is contact on opposite side of target
            float t_hit_far = std::min(t_far.x, t_far.y);
            // Reject if ray direction is pointing away from object
            if (t_hit_far < 0)
                return false;
            // Contact point of collision from parametric line equation
            contact_point = ray_origin + t_hit_near * ray_dir;

            if (t_near.x > t_near.y)
                if (invdir.x < 0)
                    contact_normal = { 1, 0 };
                else
                    contact_normal = { -1, 0 };
            else if (t_near.x < t_near.y)
                if (invdir.y < 0)
                    contact_normal = { 0, 1 };
                else
                    contact_normal = { 0, -1 };

            // Note if t_near == t_far, collision is principly in a diagonal
            // so pointless to resolve. By returning a CN={0,0} even though its
            // considered a hit, the resolver wont change anything.
            return true;
        }

        bool DynamicRectVsRect(const rect* r_dynamic, const float fTimeStep, const rect& r_static,
            olc::vf2d& contact_point, olc::vf2d& contact_normal, float& contact_time)
        {
            // Check if dynamic rectangle is actually moving - we assume rectangles are NOT in collision to start
            if (r_dynamic->vel.x == 0 && r_dynamic->vel.y == 0)
                return false;

            // Expand target rectangle by source dimensions
            rect expanded_target;
            expanded_target.pos = r_static.pos - r_dynamic->size / 2;
            expanded_target.size = r_static.size + r_dynamic->size;
       
            if (RayVsRect(r_dynamic->pos + r_dynamic->size / 2, r_dynamic->vel * fTimeStep, &expanded_target, contact_point, contact_normal, contact_time))
                return (contact_time >= 0.0f && contact_time < 1.0f);
            else
                return false;
        }

        bool ResolveDynamicRectVsRect(rect* r_dynamic, const float fTimeStep, rect* r_static, std::vector<rect> cells)
        {
            olc::vf2d contact_point, contact_normal;
            float contact_time = 0.0f;
            if (DynamicRectVsRect(r_dynamic, fTimeStep, *r_static, contact_point, contact_normal, contact_time))
            {
                if (contact_normal.y > 0) r_dynamic->contact[0] = r_static; else nullptr;
                if (contact_normal.x < 0) r_dynamic->contact[1] = r_static; else nullptr;
                if (contact_normal.y < 0) r_dynamic->contact[2] = r_static; else nullptr;
                if (contact_normal.x > 0) r_dynamic->contact[3] = r_static; else nullptr;
                r_dynamic->vel += contact_normal * olc::vf2d(std::abs(r_dynamic->vel.x), std::abs(r_dynamic->vel.y)) * (1 - contact_time);
                /*if (r_dynamic->contact[2] != nullptr) {
                    std::cout << r_dynamic->contact[2]->pos.y;
                }

                //return true;
                if (cells[0].pos.y > r_dynamic->pos.y + r_dynamic->size.y / 2 && cells[0].pos.y < r_dynamic -> pos.y + r_dynamic->size.y) {
                    r_dynamic->pos.y = cells[0].pos.y - r_dynamic-> size.y;
                    //std::cout << "In this case, we detected an upward slope" << std::endl;
                }
                else {
                    r_dynamic->vel += contact_normal * olc::vf2d(std::abs(r_dynamic->vel.x), std::abs(r_dynamic->vel.y)) * (1 - contact_time);
                    //std::cout << "In this case, we detected a wall" << std::endl;
                }
                lastCell = cells[0]*/
                return true;
            }

            return false;
        }


        void Digger(){
            RemoveCircle(GetMouseX() - 8, GetMouseY() - 8, spriteMap, spriteCollisionMap, spriteRedCircle);
        }

        void loadLevel(std::string path){
            
        }

        bool OnUserCreate() override
        {
            auto& assets = AssetManager::Current();

            textureMap["spritemap"] = assets.CreateTexture("res/image.png");
            textureMap["collisionmap"] = assets.CreateTexture("res/collisionmap.png");
            textureMap["bombpng"] = assets.CreateTexture("res/circlebox.png");
            textureMap["cointexture"] = assets.CreateTexture("res/coin.png");
            textureMap["pingutexture"] = assets.CreateTexture("res/pingu.png");
            
            debugMode = false;
            action = BOMB;

            spriteMap = new olc::Sprite("res/image.png");
            spriteCollisionMap = new olc::Sprite("res/collisionmap.png");
            spriteRedCircle = new olc::Sprite("res/circlebox.png");
            spriteCoin = new olc::Sprite("res/coin.png");

            levelActors.push_back(std::make_unique<Pingu>(olc::vf2d{ 2.0,0.0 }));

            RemoveCircle(4, 4, assets.GetTexture(textureMap["spritemap"])->sprite, assets.GetTexture(textureMap["collisionmap"])->sprite, assets.GetTexture(textureMap["bombpng"])->sprite);
            return true;
        }
        bool OnUserUpdate(float fElapsedTime) override
        {
            Clear(olc::BLACK);
            auto& assets = AssetManager::Current();
            
            if(debugMode){
                if(action == BOMB){
                    if(GetMouse(0).bPressed){
                        RemoveCircle(
                            GetMouseX() - 8, GetMouseY() - 8,
                            assets.GetTexture(textureMap["spritemap"])->sprite,
                            assets.GetTexture(textureMap["collisionmap"])->sprite,
                            assets.GetTexture(textureMap["bombpng"])->sprite
                        );
                    }
                }
            }
            
            for (auto& act : levelActors){
                act->update(this, fElapsedTime);
                act -> draw(this, textureMap);
            }
            DrawSprite(0, 0, assets.GetTexture(textureMap["collisionmap"])->sprite);
            //DrawRect(0, 0, 16, 24, olc::YELLOW);
            return true;
        }
};

void Actor::update(Pingus* game, float _fElapsedTime){}
void Actor::draw(Pingus* game, std::map<std::string, size_t>& texMap) {
    auto& assets = AssetManager::Current();
    game->DrawSprite(pos, assets.GetTexture(texMap["cointexture"])->sprite);
}

void Pingu::update(Pingus* game, float _fElapsedTime) {
    auto& assets = AssetManager::Current();

    rect imaginaryBox = { {playerrect.pos.x - fabs(playerrect.vel.x) - 10, playerrect.pos.y - fabs(playerrect.vel.y) - 10}, {fabs(playerrect.vel.x) * 2 + playerrect.size.x + 20, fabs(playerrect.vel.y) * 2 + playerrect.size.y + 20} };
    rect temprect;
    olc::vf2d cp, cn;
    float ct = 0, min_t = INFINITY;
    std::vector<std::tuple<int, float, rect>> z;
    std::vector<rect> collidedCells;

    olc::vf2d vMouse = { float(game->GetMouseX()), float(game->GetMouseY()) };
    olc::vf2d ray_point = { playerrect.pos.x, playerrect.pos.y };
    olc::vf2d ray_direction = vMouse - ray_point;
    if(game->GetMouse(0).bHeld){
        playerrect.vel += ray_direction.norm() * 50.0f * _fElapsedTime;
    }
    game->DrawRect(imaginaryBox.pos, imaginaryBox.size, olc::YELLOW);

    playerrect.vel.y += grv;

    //create the vector
    std::vector<olc::vf2d> snapDownCells;
    //find solids
    for (int y = int(playerrect.pos.y + playerrect.size.y); y < int(playerrect.pos.y + playerrect.size.y + 4.0f); y++) {
        for (int x = int(playerrect.pos.x - 1.0f); x < int(playerrect.pos.x + playerrect.size.x); x++) {
            if (game->CompareColour(assets.GetTexture(game->textureMap["collisionmap"])->sprite->GetPixel(x, y), olc::Pixel(69, 40, 60, 255))) {
                snapDownCells.push_back({ float(x), float(y) });
            }
        }
    }
    //sort solids
    std::sort(snapDownCells.begin(), snapDownCells.end(), [](const olc::vf2d& a, const olc::vf2d& b)
    {
        return a.y < b.y;
    });
    //set y position to solid + height if rect's contact[2] != nullptr;
    

    if (playerrect.contact[2] != nullptr && snapDownCells.size() > 0) {
        playerrect.pos.y = snapDownCells[0].y - playerrect.size.y;
        std::cout << snapDownCells[0].y << std::endl;
    }

    for (int y = int(imaginaryBox.pos.y); y < int(imaginaryBox.pos.y + imaginaryBox.size.y); y++) {
        for (int x = int(imaginaryBox.pos.x); x < int(imaginaryBox.pos.x + imaginaryBox.size.x); x++) {
            if (game -> CompareColour((assets.GetTexture(game -> textureMap["collisionmap"])->sprite)->GetPixel(x, y), olc::Pixel(69, 40, 60, 255))) {
                
                temprect = { {float(x), float(y)},{1.0f, 1.0f} };
                if (game->DynamicRectVsRect(&playerrect, _fElapsedTime, temprect, cp, cn, ct))
                {

                    z.push_back(std::make_tuple( y*int(imaginaryBox.size.x)+x , ct , temprect));
                    
                    collidedCells.push_back(temprect);
                }
            }
        }
    }

    std::sort(z.begin(), z.end(), [](const std::tuple<int, float, rect>& a, const std::tuple<int, float, rect>& b)
    {
        return std::get<1>(a) < std::get<1>(b);
    });

    std::sort(collidedCells.begin(), collidedCells.end(), [](const rect& a, const rect& b)
        {
            return a.pos.y < b.pos.y;
        });

    for (auto j : z)
        game ->ResolveDynamicRectVsRect(&playerrect, _fElapsedTime, &std::get<2>(j), collidedCells);

    playerrect.pos += playerrect.vel * _fElapsedTime;
}
void Pingu::draw(Pingus* game, std::map<std::string, size_t>& texMap) {
    auto& assets = AssetManager::Current();
    game->DrawSprite(playerrect.pos, assets.GetTexture(texMap["pingutexture"])->sprite);
}

int main()
{
    Pingus demo;
    demo.SetPixelMode(olc::Pixel::MASK);
    if(demo.Construct(1600, 800, 1, 1))
    {
        demo.Start();
    }
    return 0;
}