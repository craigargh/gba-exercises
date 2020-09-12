
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_sprites.h>
#include <gba_video.h>
#include <gba_input.h>
#include <string.h>
#include "snek.h"
#include "bg.h"

typedef u16 Tile[32];
typedef Tile TileBlock[256];
typedef u16 ScreenBlock[1024];

#define MEM_TILE ((TileBlock*)VRAM)
#define MEM_SCREENBLOCK ((ScreenBlock*) VRAM)

OBJATTR oam_backbuffer[128];

u16 keysPressed = 0;

struct Position {
    u16 x_pos;
    u16 y_pos;
};

Position positions[600];

u16 snake_length = 2;

// snake tale
// death on collision with walls or self
// random location for food
// eat food
// grow when eat food
// increase game speed
// add sound effects
// start game when the player presses start

void setupSpriteTiles(){
    memcpy(&MEM_TILE[4][1], snekTiles, snekTilesLen);
    memcpy(SPRITE_PALETTE, snekPal, snekPalLen);
}

void setupBackground(){
    memcpy(&MEM_SCREENBLOCK[0], bgBlocks, bgBlocksLen);
    memcpy(&MEM_TILE[0][0], bgTiles, bgTilesLen);
    memcpy(BG_PALETTE, bgPal, bgPalLen);
}


u16 keyPressed(u16 keyCode){
    return keyCode & keysPressed;
}

void updateBuffer(Position *positions){
    for (int i=0; i<snake_length; i++){
        u8 tile_x = positions[i].x_pos;
        u8 tile_y = positions[i].y_pos;

        volatile OBJATTR *snake = &oam_backbuffer[i];
        snake->attr0 = OBJ_Y(tile_y) | ATTR0_SQUARE | ATTR0_COLOR_256;
        snake->attr1 = OBJ_X(tile_x) | ATTR1_SIZE_8;
        snake->attr2 = 2;
    }
}

int main(void) {
    SetMode(MODE_0 | OBJ_ENABLE | BG0_ENABLE | OBJ_1D_MAP);
    REG_BG0CNT = BG_SIZE_0 | SCREEN_BASE(0) | BG_256_COLOR | BG_TILE_BASE(0) | BG_PRIORITY(0);

    irqInit();
    irqEnable(IRQ_VBLANK);

    
    setupBackground();
    setupSpriteTiles();
    scanKeys();
    
    u8 direction = 1;
    u16 x_pos = 48;
    u16 y_pos = 104;

    u16 frame_count = 0;

    positions[0] = {x_pos, y_pos};
    positions[1] = {(u8) (x_pos - 8), y_pos};
    updateBuffer(positions);

    while (1) {
        VBlankIntrWait();
        scanKeys();
        for (int i=0; i<snake_length; i++){
            OAM[i] = oam_backbuffer[i];
        }

        keysPressed = keysDown();

        if (keyPressed(KEY_UP)){
            direction = 0;
        } else if (keyPressed(KEY_RIGHT)){
            direction = 1;
        } else if (keyPressed(KEY_DOWN)){
            direction = 2;
        } else if (keyPressed(KEY_LEFT)){
            direction = 3;
        } else if (keyPressed(KEY_START)){
            snake_length ++;
        }

        if (frame_count == (100 - snake_length))
        {
            switch (direction){
                case 0:
                    y_pos -= 8;
                    break;
                case 1:
                    x_pos += 8;
                    break;
                case 2:
                    y_pos += 8;
                    break;
                case 3:
                    x_pos -= 8;
                    break;
            }
            frame_count = 0;

            for (int i=snake_length; i>=0; i--){
                if (i == 0)
                    positions[i] = {x_pos, y_pos};
                else{
                    Position prev = positions[i-1];
                    positions[i] = {prev.x_pos, prev.y_pos};
                }
            }

            updateBuffer(positions);
            

        } else {
            frame_count ++;
        }
    }
}


