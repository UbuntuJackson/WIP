#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "asset_manager.h"
#include "entity.h"
//#include "actor.h"
//#include "pingu.h"
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
        size = {4.0, 4.0};
        vel = { 0.0, 0.0 };
    }
    virtual void update(Pingus* game, float _fElapsedTime){
    }
    virtual void draw(olc::PixelGameEngine* pge, std::map<std::string,size_t>&texMap) {
        auto& assets = AssetManager::Current();
        pge->DrawSprite(pos, assets.GetTexture(texMap["cointexture"])->sprite);
    }
};

class Pingu : public Actor{
    public:
        float grv;
        Pingu(olc::vf2d _pos) : Actor(_pos){
            size = {3.0, 4.0};
            grv = 1.0;
            vel.x = 1.0;
            std::cout << "create" << std::endl;
        }
        ~Pingu() {
            std::cout << "destroy" << std::endl;
        }
    virtual void update(Pingus* game, float _fElapsedTime){
        rect imaginaryBox = {{pos.x - size.x, pos.y - size.y}, {vel.x * 2, vel.y * 2}};
        rect playerBox = { {pos.x, pos.y}, {size.x, size.y} };
        rect temprect;
        olc::vf2d cp, cn;
        float ct = 0; //, min_t = INFINITY;

        for (int y = int(imaginaryBox.pos.y); y < int(imaginaryBox.pos.y + imaginaryBox.size.y); y++) {
            for (int x = int(imaginaryBox.pos.x); x < int(imaginaryBox.pos.x + imaginaryBox.size.x); x++) {
                temprect = { {float(x), float(y)},{1.0f, 1.0f}};
                if (game -> DynamicRectVsRect(&temprect, _fElapsedTime, playerBox, cp, cn, ct))
                {
                    vel = { 0.0, 0.0 };
                }
            }
        }

        std::cout << _fElapsedTime << std::endl;
        vel.y += grv;
        pos += vel * _fElapsedTime;
    }
    virtual void draw(olc::PixelGameEngine* pge, std::map<std::string,size_t>&texMap) {
        auto& assets = AssetManager::Current();
        pge->DrawSprite(pos, assets.GetTexture(texMap["pingutexture"])->sprite);
    }
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
        bool CompareColour(olc::Pixel colour_a, olc::Pixel colour_b){
            if(colour_a.r == colour_b.r && colour_a.g == colour_b.g && colour_a.b == colour_b.b &&
                colour_a.a == colour_b.a
            ){
                return true;
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
                        //std::cout << x+i << ", " << y+j << std::endl;
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

        bool ResolveDynamicRectVsRect(rect* r_dynamic, const float fTimeStep, rect* r_static)
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
            
            debugMode = true;
            action = BOMB;

            spriteMap = new olc::Sprite("res/image.png");
            spriteCollisionMap = new olc::Sprite("res/collisionmap.png");
            spriteRedCircle = new olc::Sprite("res/circlebox.png");
            spriteCoin = new olc::Sprite("res/coin.png");

            //Actor actor(2, 2);
            //Pingu pingu(30, 30);
            //Pingu* ppingu = new Pingu({90, 90});
            //Auto_ptr1<Actor> ppingu(new Pingu({ 90, 90 }));
            //Actor* actor = new Actor({0, 0});
            //levelActors.push_back(actor);
            levelActors.push_back(std::make_unique<Pingu>(olc::vf2d{ 32.0,32.0 }));
            //levelActors.push_back(ppingu);
            //levelActors.push_back(actor);
            //spriteMap -> SetPixel(olc::vi2d(7, 16), olc::Pixel(0, 0, 0, 255));
            RemoveCircle(4, 4, assets.GetTexture(textureMap["spritemap"])->sprite, assets.GetTexture(textureMap["collisionmap"])->sprite, assets.GetTexture(textureMap["bombpng"])->sprite);
            return true;
        }
        bool OnUserUpdate(float fElapsedTime) override
        {
            Clear(olc::BLACK);
            auto& assets = AssetManager::Current();
            /*for(int j = 0; j < spriteCollisionMap -> height; j++){
                for(int i = 0; i < spriteCollisionMap -> width; i++){
                    Draw(i, j, collisionMap[(spriteCollisionMap -> width) * j + i]);
                }
            }*/
            
            if(debugMode){
                if(action == BOMB){
                    if(GetMouse(0).bPressed){
                        //std::cout << GetMouse(0).bPressed;
                        RemoveCircle(
                            GetMouseX() - 8, GetMouseY() - 8,
                            assets.GetTexture(textureMap["spritemap"])->sprite,
                            assets.GetTexture(textureMap["collisionmap"])->sprite,
                            assets.GetTexture(textureMap["bombpng"])->sprite
                        );
                    }
                }
                /*if(action == DIG){
                    if(GetMouse(0).bHeld){}
                }*/
            }
            //std::cout << CompareColour(spriteMap -> GetPixel(7, 16), olc::Pixel(0, 0, 0, 0));
            for (auto& act : levelActors){
                act->update(this, fElapsedTime);
                act -> draw(this, textureMap);
            }
            DrawSprite(0, 0, assets.GetTexture(textureMap["spritemap"])->sprite);
            return true;
        }
};

int main()
{
    Pingus demo;
    demo.SetPixelMode(olc::Pixel::MASK);
    if(demo.Construct(200, 200, 4, 4))
    {
        demo.Start();
    }
    return 0;
}