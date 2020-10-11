
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
#define min(x,y) (x > y ? y : x)
#define max(x,y) (x < y ? y : x)

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

typedef struct {
    u8 x;
    u8 y;
} Position;

u16 ticks = 0;

#define WINDOW_VERTICAL 48
#define WINDOW_HORIZONTAL 88 

const u8 windowTop = 0 + WINDOW_VERTICAL;
const u8 windowBottom = SCREEN_HEIGHT - 16 - WINDOW_VERTICAL;

const u8 windowLeft = 0 + WINDOW_HORIZONTAL;
const u8 windowRight = SCREEN_WIDTH - 16 - WINDOW_HORIZONTAL;





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

u8 calculateFacing(const Player* player, const Directions* keys){
    u16 facing = player->facing;


    if (keys->up & (player->facing==NORTH))
        facing = NORTH;
    else if (keys->right & (player->facing==EAST))
        facing = EAST;
    else if (keys->down & (player->facing==SOUTH))
        facing = SOUTH;
    else if (keys->left & (player->facing==WEST))
        facing = WEST;
    else {
        if (keys->up)
            facing = NORTH;
        if (keys->down)
            facing = SOUTH;
        if (keys->right)
            facing = EAST;
        if (keys->left)
            facing = WEST;
    }

    return facing;
}

u8 calculateAnimationFrame(const Player* player, const Directions* keys){
    u8 frame = player->animationFrame;
    u8 ticksPerFrame = 10;

    if (ticks == ticksPerFrame) {
        frame += 1;

        if (frame == 3)
            frame = 1;

        else if (!keys->up & !keys->down & !keys->left & !keys->right)
            frame = 0;
    }
    
    ticks += 1;
    if (ticks > ticksPerFrame)
        ticks = 0;

    return frame;
}

Player updatePlayer(const Player* player, const Directions* keys){
    u16 facing = calculateFacing(player, keys);
    u16 animationFrame = calculateAnimationFrame(player, keys);

    int xMove = 0;
    int yMove = 0;

    if (keys->up)
        yMove = -1;
    else if (keys->down)
        yMove = 1;

    if (keys->right)
        xMove = 1;
    else if (keys->left)
        xMove = -1;

    int newX = player->posX + xMove;
    newX = max(windowLeft, newX);
    newX = min(windowRight, newX);

    int newY = player->posY + yMove;
    newY = max(windowTop, newY);
    newY = min(windowBottom, newY);


    Player updatedPlayer;
    updatedPlayer.facing = facing;
    updatedPlayer.posX = newX;
    updatedPlayer.posY = newY;
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

Position calculateBgPos(const Position* bgPos, const Player* player, const Directions* keys){
    Position newPos;
    newPos.x = bgPos->x;
    newPos.y = bgPos->y;


    if (player->posX == windowRight && keys->right)
        newPos.x += 1;
    else if (player->posX == windowLeft && keys->left)
        newPos.x -= 1;

    if (player->posY == windowTop && keys->up)
        newPos.y -= 1;
    else if (player->posY == windowBottom && keys->down)
        newPos.y += 1;

    return newPos;
}

void updateOAM(){
    CpuFastSet(oam_backbuffer, OAM, ((sizeof(OBJATTR)*128)>>2) | COPY32);
}

void updateBgPos(Position* bgPos){
    BG_OFFSET[0].x = bgPos->x;
    BG_OFFSET[0].y = bgPos->y;
}

int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);

    SetMode(MODE_0 | OBJ_ENABLE | BG0_ENABLE | OBJ_1D_MAP);
    setupTiles();

    Player player = initPlayer();
    updatePlayerSprite(&player);

    Position bgPos;
    bgPos.x = 0;
    bgPos.y = 0;

    Directions keys = heldKeys();

    while (1) {
        VBlankIntrWait();
        updateOAM();
        updateBgPos(&bgPos);

        scanKeys();

        keys = heldKeys();
        player = updatePlayer(&player, &keys);
        bgPos = calculateBgPos(&bgPos, &player, &keys);

        updatePlayerSprite(&player);
    }
}


// TODO
// Fix sprites