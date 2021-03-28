#include <vector>

#include <string.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>

#include "asuka.h"
#include "gameobjects.hpp"

OBJATTR oam_backbuffer[128];


void uploadPaletteMem(){
    CpuFastSet(asukaPal, SPRITE_PALETTE, (asukaPalLen >> 2) | COPY32);
}

void uploadTileMem(){
    CpuFastSet(asukaTiles, TILE_BASE_ADR(5), (asukaTilesLen >> 2) | COPY32);
}

void initGameboy(){
    irqInit();
    irqEnable(IRQ_VBLANK);

    SetMode(MODE_0 | OBJ_ENABLE | OBJ_1D_MAP);

    uploadPaletteMem();
    uploadTileMem();
}

Sprite makeCharacterSprite(){
    u16 tileIndex = 512;
    u8 x = 0;
    u8 y = (SCREEN_HEIGHT >> 1) - 8;

    Sprite character (&oam_backbuffer[0], tileIndex, x, y);
    return character;
};

int main() {
    initGameboy();

    std::vector<Sprite> sprites;
    sprites.push_back(makeCharacterSprite());

    while(1){
        for (u8 i = 0; i < sprites.size(); i++) {
            sprites[i].update();
        }
        
        for (u8 i = 0; i < sprites.size(); i++) {
            sprites[i].draw();
        }

        VBlankIntrWait();
        CpuFastSet(oam_backbuffer, OAM, ((sizeof(OBJATTR)*128)>>2) | COPY32);
    }
    return 0;
}