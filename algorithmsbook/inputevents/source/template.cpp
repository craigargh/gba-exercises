#include <vector>

#include <string.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>

#include "asuka.h"
#include "gameobjects.hpp"

OBJATTR oam_backbuffer[128];
Sprite* player;


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

void movePlayerNorth(){
    player->faceNorth();
}

void movePlayerSouth(){
    player->faceSouth();
}

void movePlayerEast(){
    player->faceEast();
}

void movePlayerWest(){
    player->faceWest();
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

void registerPlayerListeners(){
    eventManager.registerListener("up", movePlayerNorth);
    eventManager.registerListener("down", movePlayerSouth);
    eventManager.registerListener("left", movePlayerWest);
    eventManager.registerListener("right", movePlayerEast);
}

void pollInput(){
    scanKeys();
    u16 keysPressed = keysHeld();

    if (keysPressed & KEY_UP){
        eventManager.listenerMap.find("up")->second();
    } 
    
    if (keysPressed & KEY_DOWN) {
        eventManager.listenerMap.find("down")->second();
    } 
    
    if (keysPressed & KEY_RIGHT) {
        eventManager.listenerMap.find("right")->second();
    } 
    
    if (keysPressed & KEY_LEFT){
        eventManager.listenerMap.find("left")->second();
    }
}

int main() {
    initGameboy();

    u8 halfWidth = SCREEN_WIDTH >> 1;
    u8 halfHeight = SCREEN_HEIGHT >> 1;

    std::vector<Sprite> sprites;
    sprites.push_back(makeCharacterSprite(halfWidth - 28, halfHeight - 8, 0));

    player = &sprites[0];

    registerPlayerListeners();

    while(1){
        pollInput();

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