#include <gba_base.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <gba_video.h>

#include <string>
#include <functional>
#include <map>
#include <tuple>
#include <vector>


typedef std::function<void()> voidFunction;

#define min(x,y) (x > y ? y : x)
#define max(x,y) (x < y ? y : x)


// To refactor:
// - Move event keys to constants 
// - Add event for pause and set game state to "paused"


class EventCallable {
public:
    std::string stateType;
    voidFunction callable;

    EventCallable(std::string ctgry, voidFunction func);
};


class InputManager{
public:
    std::map<std::string, std::vector<EventCallable>> listenerMap;
    std::vector<std::tuple<u8, std::string>> keyMaps;
    std::string state = "game";

    InputManager();

    void registerListener(std::string eventName, voidFunction func);

    void pollInput();
};


class AnimationFrame{
public:
    u8 duration;
    u8 tileOffset;

    AnimationFrame(u8 tile, u8 length);
};

class Animation {
public:
    u16 baseTile;
    u16 tileSetOffset;
    std::vector<AnimationFrame> frames;
    u8 frame;
    bool flip = false;

    Animation();

    Animation(u16 baseTileValue);
    
    void tick();
    
    AnimationFrame currentFrame();
    
    u16 tile();
};


class Sprite {
public:
    u8 x;
    u8 y;
    OBJATTR* oam;
    Animation animation;

    Sprite();

    Sprite(OBJATTR* oamRef, u8 xPos, u8 yPos);

    void draw();

    void update();

    void faceNorth();

    void faceSouth();

    void faceEast();

    void faceWest();
};


class StaticObject;

class Trigger;

void registerPlayerListeners(InputManager* eventManager, Sprite* player);