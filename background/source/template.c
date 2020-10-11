
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <string.h>

#include "asuka.h"
#include "grass.h"


OBJATTR oam_backbuffer[128];

typedef u16 Tile[32];
typedef Tile TileBlock[256];
typedef u16 ScreenBlock[1024];

#define MEM_SCREENBLOCKS ((ScreenBlock*)VRAM)
#define NORTH 1
#define EAST 2
#define SOUTH 3
#define WEST 4

typedef struct {
    u8 facing;
    u8 animationFrame;
    u8 posX;
    u8 posY;
} Player;

typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
} Directions;

u16 frameTicks = 0;


void uploadPaletteMem(){
	CpuFastSet(asukaPal, SPRITE_PALETTE, (asukaPalLen>>2) | COPY32);
}

void uploadTileMem(){
	CpuFastSet(asukaTiles, TILE_BASE_ADR(5), (asukaTilesLen >> 2) | COPY32);
}

void uploadBgMem(){
	CpuFastSet(grassTiles, TILE_BASE_ADR(0), (grassTilesLen >>2)| COPY32);
}

void uploadBgPalMem(){
	CpuFastSet(grassPal, BG_PALETTE, (grassPalLen >> 2) | COPY32);
}

void uploadTileMap(){
    u16 bgArray[1024];
    for (u16 i =0; i < 1024; i++){
        bgArray[i] = 0x0000;
    }

    CpuFastSet(bgArray, &MEM_SCREENBLOCKS[1], (2048 >> 2) | COPY32);
}

void setupTiles(){
    uploadTileMem();
    uploadPaletteMem();
    uploadBgPalMem();
    uploadBgMem();
    uploadTileMap();

    REG_BG0CNT = BG_SIZE_0 | SCREEN_BASE(1) | BG_16_COLOR | BG_TILE_BASE(0) | BG_PRIORITY(0);
}

Player initPlayer(){
    Player player;
    player.facing = SOUTH;
    player.posX = 112;
    player.posY = 72;
    player.animationFrame = 0;

    return player;
}

u8 calculateFacing(const Player* player, const Directions* newKeys, const Directions* oldKeys){
    u16 facing = player->facing;

    if (((newKeys->up | newKeys->down) & (newKeys->left | newKeys->right))){
        if (newKeys->up & (player->facing==NORTH))
            facing = NORTH;
        if (newKeys->right & (player->facing==EAST))
            facing = EAST;
        if (newKeys->down & (player->facing==SOUTH))
            facing = SOUTH;
        if (newKeys->left & (player->facing==WEST))
            facing = WEST;

    } else {
        if (newKeys->up)
            facing = NORTH;
        if (newKeys->down)
            facing = SOUTH;
        if (newKeys->right)
            facing = EAST;
        if (newKeys->left)
            facing = WEST;
    }


    return facing;
}

u8 calculateAnimationFrame(const Player* player, const Directions* newKeys, const Directions* oldKeys){
    u8 frame = player->animationFrame;
    u8 framesPerTick = 10;

    if (frameTicks == framesPerTick) {
        frame += 1;

        if (frame == 3)
            frame = 1;

        if (!newKeys->up & !newKeys->down & !newKeys->left & !newKeys->right)
            frame = 0;
    }
    
    frameTicks += 1;
    if (frameTicks > framesPerTick)
        frameTicks = 0;

    return frame;
}

Player updatePlayer(const Player* player, const Directions* newKeys, const Directions* oldKeys){
    u16 facing = calculateFacing(player, newKeys, oldKeys);
    u16 animationFrame = calculateAnimationFrame(player, newKeys, oldKeys);

    int xMove = 0;
    int yMove = 0;

    if (newKeys->up)
        yMove = -1;
    else if (newKeys->down)
        yMove = 1;

    if (newKeys->right)
        xMove = 1;
    else if (newKeys->left)
        xMove = -1;


    Player updatedPlayer;
    updatedPlayer.facing = facing;
    updatedPlayer.posX = player->posX + xMove;
    updatedPlayer.posY = player->posY + yMove;
    updatedPlayer.animationFrame = animationFrame;

    return updatedPlayer;
}

u16 calculateTile(const Player* player){
    u16 directionOffest = 0;

    if (player->facing == NORTH)
        directionOffest = 24;
    else if (player->facing == EAST)
        directionOffest = 12;
    else if (player->facing == SOUTH)
        directionOffest = 0;
    else if (player->facing == WEST)
        directionOffest = 12;

    u16 frameOffset = player->animationFrame * 4;

    u16 tileBase = 512;
    u16 tile = tileBase + directionOffest + frameOffset;

    return tile;
}

void updatePlayerSprite(const Player* player){
    u16 tile = calculateTile(player);
    
    volatile OBJATTR *sprite = &oam_backbuffer[0];
    sprite->attr0 = OBJ_16_COLOR | ATTR0_SQUARE | OBJ_Y(player->posY);
    sprite->attr1 = ATTR1_SIZE_16 | OBJ_X(player->posX);
    sprite->attr2 = tile | ATTR2_PALETTE(0);

    if (player->facing == EAST)
        sprite->attr1 |= OBJ_HFLIP;
}

Directions heldKeys(){
    u16 keysPressed = keysHeld();

    Directions keys;
    keys.up = keysPressed & KEY_UP;
    keys.down = keysPressed & KEY_DOWN;
    keys.left = keysPressed & KEY_LEFT;
    keys.right = keysPressed & KEY_RIGHT;

    return keys;
}

void updateOAM(){
    CpuFastSet(oam_backbuffer, OAM, ((sizeof(OBJATTR)*128)>>2) | COPY32);
}

int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);

    SetMode(MODE_0 | OBJ_ENABLE | BG0_ENABLE | OBJ_1D_MAP);
    setupTiles();

    Player player = initPlayer();
    updatePlayerSprite(&player);

    Directions newKeys = heldKeys();
    Directions oldKeys = newKeys;
    

    while (1) {
        VBlankIntrWait();
        updateOAM();
        scanKeys();

        newKeys = heldKeys();
        player = updatePlayer(&player, &newKeys, &oldKeys);

        updatePlayerSprite(&player);

        oldKeys = newKeys;
    }
}


// TODO
// Add windowing
// Fix sprites