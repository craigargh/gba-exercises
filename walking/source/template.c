
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include "asuka.h"

#define MEM_TILE ((TileBlock*) VRAM)
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define min(x,y) (x > y ? y : x)
#define max(x,y) (x < y ? y : x)

OBJATTR oam_backbuffer[128];
u16 keysPressed;
u16 prevDirection = SOUTH;
u16 frameTick = 0;

typedef u16 Tile[16];
typedef Tile TileBlock[512];


typedef struct {
    u8 direction;
    u8 x;
    u8 y;
    u8 frame;
    bool walking;
} SpriteObj;


SpriteObj initSprite(){
    SpriteObj sprite;
    sprite.direction = SOUTH;
    sprite.x = 120 - 8;
    sprite.y = 80 - 8;
    sprite.walking = false;
    sprite.frame = 0;

    return sprite;
}


void updateSprite(SpriteObj *sprite, u8 direction, bool walking){
    sprite->direction = direction;
    sprite->walking = walking;


    if (sprite->walking){
        u8 tick_size = 16;

        if (frameTick < tick_size){
            sprite->frame = 1;
        } else if (frameTick < tick_size * 2){
            sprite->frame = 2;
        }

        u8 x = sprite->x;
        u8 y = sprite->y;

        if (sprite->direction == NORTH){
            sprite->y = max(y-1, 0);
        } else if (sprite->direction == SOUTH){
            sprite->y = min(y+1, SCREEN_HEIGHT - 16);
        } else if (sprite->direction == EAST){
            sprite->x = min(x+1, SCREEN_WIDTH - 16);
        } else if (sprite->direction == WEST){
            sprite->x = max(x-1, 0);
        }

        frameTick += 1;

        if (frameTick >= tick_size * 2) {
            frameTick = 0;
        }

        

    } else {
        sprite->frame = 0;
        frameTick = 0;
    }

    
}


void updateObject(SpriteObj *sprite){
    u16 tileBase = 512;
    u8 tileOffset = 0;

    if (sprite->direction == NORTH){
        tileOffset = 24;
    } else if (sprite->direction == SOUTH){
        tileOffset = 0;
    } else if (sprite->direction == EAST){
        tileOffset = 12;
    } else if (sprite->direction == WEST){
        tileOffset = 12;
    }

    u16 tile = tileBase + tileOffset + (sprite->frame * 4);

    volatile OBJATTR *object = &oam_backbuffer[0];
    object->attr0 = OBJ_16_COLOR | ATTR0_SQUARE | OBJ_Y(sprite->y);
    object->attr1 = ATTR1_SIZE_16 | OBJ_X(sprite->x);
    object->attr2 = tile | ATTR2_PALETTE(0);

    if (sprite->direction == EAST){
        object->attr1 |= OBJ_HFLIP;
    }

}

u16 getDirection() {
    u16 newDirection;

    if (keysPressed & KEY_UP){
        newDirection = NORTH;
    } else if (keysPressed & KEY_DOWN) {
        newDirection = SOUTH;
    } else if (keysPressed & KEY_RIGHT) {
        newDirection = EAST;
    } else if (keysPressed & KEY_LEFT){
        newDirection = WEST;
    } else {
        newDirection = prevDirection;
    }

    prevDirection = newDirection;

    return newDirection;
}

u16 getWalking() {
    return keysPressed != 0;
}

int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);

    SetMode(MODE_0 | OBJ_ENABLE | BG0_ENABLE | OBJ_1D_MAP);

    CpuFastSet(asukaTiles, TILE_BASE_ADR(5), (asukaTilesLen >> 2) | COPY32);
    CpuFastSet(asukaPal, SPRITE_PALETTE, (asukaPalLen >> 2) | COPY32);
    

    SpriteObj sprite = initSprite();

    updateObject(&sprite);


    while (1) {
        VBlankIntrWait();
        CpuFastSet(oam_backbuffer, OAM, ((sizeof(OBJATTR)*128)>>2) | COPY32);
        
        scanKeys();

        keysPressed = keysHeld();
        u8 direction = getDirection();
        bool walking = getWalking();
        
        updateSprite(&sprite, direction, walking);
        updateObject(&sprite);
    }
}


