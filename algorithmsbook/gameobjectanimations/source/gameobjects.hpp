#include <gba_base.h>
#include <gba_sprites.h>


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
    u16 tileIndex;
    std::vector<AnimationFrame> frames;
    u8 frame;

    Animation(){};

    Animation(u16 tileIndexValue){
        tileIndex = tileIndexValue;
        frame = 0;
    };

    void tick(){
        frame ++;
    }

    u16 tile(){
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

        return tileIndex + (frames[frameIndex].tileOffset * 4);
    }
};


class Sprite {
public:
    u8 x;
    u8 y;
    OBJATTR* oam;
    Animation animation;

    Sprite(OBJATTR* oamRef, u8 xPos, u8 yPos){
        oam = oamRef;
        x = xPos;
        y = yPos;
    };

    void draw(){
        oam->attr0 = OBJ_16_COLOR | ATTR0_SQUARE | OBJ_Y(y);
        oam->attr1 = ATTR1_SIZE_16 | OBJ_X(x);
        oam->attr2 = animation.tile() | ATTR2_PALETTE(0);
    };

    void update(){
        animation.tick();
    };
};


class StaticObject;


class Trigger;

