#include <vector>

#include <string.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>

#include "asuka.h"
#include "gameobjects.hpp"

#include "mgba.h"




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


Sprite makeCharacterSprite(u8 x, u8 y, u8 bufferIndex, bool flip=false){
    Animation animation (512);
    animation.frames.push_back(AnimationFrame(0, 8));
    animation.frames.push_back(AnimationFrame(1, 10));
    animation.frames.push_back(AnimationFrame(0, 8));
    animation.frames.push_back(AnimationFrame(2, 10));

    Sprite character (&oam_backbuffer[bufferIndex], x, y);
    character.animation = animation;

    return character;
};

Sprite makeNPCSprite(u8 x, u8 y, u8 bufferIndex, bool flip=false){
    Animation animation (512);
    animation.frames.push_back(AnimationFrame(0, 8));

    Sprite npc (&oam_backbuffer[bufferIndex], x, y);
    npc.animation = animation;

    return npc;
}

int main() {
    initGameboy();

    mgba_console_open();

    u8 halfWidth = SCREEN_WIDTH >> 1;
    u8 halfHeight = SCREEN_HEIGHT >> 1;

    sprites.push_back(makeCharacterSprite(halfWidth - 28, halfHeight - 8, 0));
    sprites.push_back(makeNPCSprite(halfWidth + 20, halfHeight - 8, 1));

    InputManager eventManager;
    registerInputBindings(&eventManager, &sprites[0]);

    while(1){
        eventManager.pollInput();

        for (u8 i = 0; i < sprites.size(); i++) {
            sprites[i].update();
        }
        
        for (u8 i = 0; i < sprites.size(); i++) {
            sprites[i].draw();
        }

        VBlankIntrWait();
        CpuFastSet(oam_backbuffer, OAM, ((sizeof(OBJATTR)*128)>>2) | COPY32);

        printf("%d\n", sprites.size());
    }
    return 0;
}