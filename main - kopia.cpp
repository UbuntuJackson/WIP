#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <vector>
#include <iostream>

class Pingus : public olc::PixelGameEngine
{
    public:
        Pingus()
        {
            sAppName = "Pingus";
        }
    public:
        olc::Sprite	*spriteMap;
        olc::Sprite	*spriteCollisionMap;
        std::vector<olc::Pixel> collisionMap;
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

        bool OnUserCreate() override
        {
            spriteMap = new olc::Sprite("res/image.png");
            spriteCollisionMap = new olc::Sprite("res/collisionmap.png");
            for(int j = 0; j < spriteCollisionMap -> height; j++){
                for(int i = 0; i < spriteCollisionMap -> width; i++){
                    olc::Pixel colour = spriteCollisionMap -> GetPixel(i, j);
                    collisionMap.push_back(colour);
                }
            }
            //spriteMap -> SetPixel(olc::vi2d(7, 16), olc::Pixel(0, 0, 0, 255));
            RemoveCell(7, 16, spriteMap, spriteCollisionMap);
            return true;
        }
        bool OnUserUpdate(float fElapsedTime) override
        {
            /*for(int j = 0; j < spriteCollisionMap -> height; j++){
                for(int i = 0; i < spriteCollisionMap -> width; i++){
                    Draw(i, j, collisionMap[(spriteCollisionMap -> width) * j + i]);
                }
            }*/
            std::cout << CompareColour(spriteMap -> GetPixel(7, 16), olc::Pixel(0, 0, 0, 0));
            DrawSprite(0, 0, spriteMap);
            return true;
        }
};

int main()
{
    Pingus demo;
    if(demo.Construct(200, 200, 4, 4))
    {
        demo.Start();
    }
    return 0;
}