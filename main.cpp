#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "asset_manager.h"
#include <vector>
#include <iostream>
#include <string>

class Actor{
    public:
        olc::vf2d pos;
		olc::vf2d size;
		olc::vf2d vel;

    Actor(olc::vf2d _pos){
        pos = _pos;
        size = {4, 4};
    }
    virtual void update(){}
    virtual void draw(olc::PixelGameEngine* pge, std::map<std::string,size_t>&texMap) {
        auto& assets = AssetManager::Current();
        pge->DrawSprite(pos, assets.GetTexture(texMap["cointexture"])->sprite);
    }
};

class Pingu : public Actor{
    public:
        Pingu(olc::vf2d _pos) : Actor(_pos){
            size = {3, 4};
        }
    virtual void update(){}
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
        std::vector<Actor*> levelActors;
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
            //std::cout << x << "," << y << std::endl;
        }
        void RemoveCircle(int x, int y, olc::Sprite *sprmap, olc::Sprite *sprcollisionMap, olc::Sprite *sprredCircle){
            std::cout << x << ", " << y << std::endl; // This prints
            for(int j = 0; j < sprredCircle -> height; j++){
                for(int i = 0; i < sprredCircle -> width; i++){
                    if(CompareColour(sprredCircle -> GetPixel(i, j), olc::Pixel(255, 0, 0, 255))){
                        RemoveCell(x+i,y+j, sprmap, sprcollisionMap);
                        //std::cout << x+i << ", " << y+j << std::endl;
                    }
                }
            }
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
            Pingu* ppingu = new Pingu({90, 90});
            Actor* actor = new Actor({0, 0});
            //levelActors.push_back(actor);
            levelActors.push_back(ppingu);
            levelActors.push_back(actor);
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
            for (auto act : levelActors){
                //DrawSprite(act.x, act.y, act.sprite);
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
    if(demo.Construct(100, 100, 8, 8))
    {
        demo.Start();
    }
    return 0;
}