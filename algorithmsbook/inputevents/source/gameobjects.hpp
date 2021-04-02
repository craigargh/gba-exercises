#include <gba_base.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <string>
#include <functional>
#include <map>


typedef int (*VoidFunction) ();

#define min(x,y) (x > y ? y : x)
#define max(x,y) (x < y ? y : x)

class InputEvent{
public:
    std::map<std::string, std::function<void()>> listenerMap;

    void registerListener(std::string eventName, std::function<void()> func){
        listenerMap.insert(std::pair<std::string, std::function<void()>>(eventName, func));
    };
};


InputEvent eventManager;


class AnimationFrame{
public:
    u8 duration;
    u8 tileOffset;

    AnimationFrame(u8 tile, u8 length){
        tileOffset = tile;
        duration = length;
    }
};

class Animation {
public:
    u16 baseTile;
    u16 tileSetOffset;
    std::vector<AnimationFrame> frames;
    u8 frame;
    bool flip = false;

    Animation(){};

    Animation(u16 baseTileValue){
        baseTile = baseTileValue;
        frame = 0;
        tileSetOffset=0;
    };

    void tick(){
        frame ++;
    }

    AnimationFrame currentFrame(){
        u8 frameIndex = 0;
        u8 cumulativeFrames = 0;

        for(u8 i = 0; i <= frames.size(); i++){
            if (i == frames.size()){
                frame = 0;
                frameIndex = 0;
                break;
            }

            cumulativeFrames += frames[i].duration;

            if (frame < cumulativeFrames){
                frameIndex = i;
                break;
            }
        }

        return frames[frameIndex];
    };

    u16 tile(){
        return baseTile + tileSetOffset + (currentFrame().tileOffset * 4);
    }
};


class Sprite {
public:
    u8 x;
    u8 y;
    OBJATTR* oam;
    Animation animation;

    Sprite(){};

    Sprite(OBJATTR* oamRef, u8 xPos, u8 yPos){
        oam = oamRef;
        x = xPos;
        y = yPos;
    };

    void draw(){
        oam->attr0 = OBJ_16_COLOR | ATTR0_SQUARE | OBJ_Y(y);
        oam->attr1 = ATTR1_SIZE_16 | OBJ_X(x);
        oam->attr2 = animation.tile() | ATTR2_PALETTE(0);

        if (animation.flip){
            oam->attr1 |= OBJ_HFLIP;
        }
    };

    void update(){
        animation.tick();
    };

    void faceNorth(){
        animation.tileSetOffset = 24;
        animation.flip = false;

        y = max(y - 1, 0);
    };

    void faceSouth(){
        animation.tileSetOffset = 0;
        animation.flip = false;

        y = min(y + 1, SCREEN_HEIGHT - 16);
    };

    void faceEast(){
        animation.tileSetOffset = 12;
        animation.flip = true;

        x = min(x + 1, SCREEN_WIDTH - 16);
    };

    void faceWest(){
        animation.tileSetOffset = 12;
        animation.flip = false;

        x = max(x - 1, 0);
    };
};


class StaticObject;


class Trigger;

