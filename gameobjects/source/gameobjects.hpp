#include <gba_base.h>
#include <gba_sprites.h>

class Sprite {

public:
    u16 tileIndex;
    u8 width;
    u8 height;
    u8 x;
    u8 y;
    OBJATTR* oam;

    Sprite(OBJATTR* oamRef, const u16 tileIndexStart, u8 xPos, u8 yPos){
        oam = oamRef;
        tileIndex = tileIndexStart;
        x = xPos;
        y = yPos;
    };

    void draw(){
        oam->attr0 = OBJ_16_COLOR | ATTR0_SQUARE | OBJ_Y(y);
        oam->attr1 = ATTR1_SIZE_16 | OBJ_X(x);
        oam->attr2 = tileIndex | ATTR2_PALETTE(0);
    };

    void update(){
        x += 1;
    };
};


class StaticObject;


class Trigger;

