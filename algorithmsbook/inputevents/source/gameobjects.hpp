#include <gba_base.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <string>
#include <functional>
#include <map>
#include <tuple>


typedef std::function<void()> voidFunction;

#define min(x,y) (x > y ? y : x)
#define max(x,y) (x < y ? y : x)


// To refactor:
// - Add category to registerListener to allow for menu, game, etc.
// - Add managerState to allow menu, game etc.


class InputManager{
public:
    std::map<std::string, std::vector<voidFunction>> listenerMap;

    InputManager(){
        listenerMap.emplace("up", std::vector<voidFunction>());
        listenerMap.emplace("down", std::vector<voidFunction>());
        listenerMap.emplace("left", std::vector<voidFunction>());
        listenerMap.emplace("right", std::vector<voidFunction>());
    };

    void registerListener(std::string eventName, voidFunction func){
        listenerMap[eventName].push_back(func);
    };

    void pollInput(){
        scanKeys();
        u16 keysPressed = keysHeld();

        std::vector<std::tuple<u8, std::string>> keyMaps;
        keyMaps.push_back(std::make_tuple(KEY_UP, "up"));
        keyMaps.push_back(std::make_tuple(KEY_DOWN, "down"));
        keyMaps.push_back(std::make_tuple(KEY_RIGHT, "right"));
        keyMaps.push_back(std::make_tuple(KEY_LEFT, "left"));

        for (u8 i = 0; i < keyMaps.size(); i++){
            u8 keyCode = std::get<0>(keyMaps[i]);
            std::string eventKey = std::get<1>(keyMaps[i]);

            if (keysPressed & keyCode){
                std::vector<voidFunction> funcs = listenerMap.find(eventKey)->second;
                
                for (u8 i=0; i < funcs.size(); i++){
                    funcs[i]();
                }
            }    
        }
    }
};


InputManager eventManager;


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

void registerPlayerListeners(Sprite* player){
    eventManager.registerListener("up", std::bind(&Sprite::faceNorth, player));
    eventManager.registerListener("down", std::bind(&Sprite::faceSouth, player));
    eventManager.registerListener("left", std::bind(&Sprite::faceWest, player));
    eventManager.registerListener("right", std::bind(&Sprite::faceEast, player));
}
