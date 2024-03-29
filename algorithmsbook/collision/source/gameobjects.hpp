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


const std::string GAME_RUNNING = "game";
const std::string GAME_PAUSED = "paused";

const std::string KEY_HELD = "held";
const std::string KEY_PRESS = "press";

const std::string EVENT_UP = "up";
const std::string EVENT_DOWN = "down";
const std::string EVENT_LEFT = "left";
const std::string EVENT_RIGHT = "right";
const std::string EVENT_PAUSE = "pause";

// To refactor:
// - Add event for pause and set game state to "paused"




class EventBinding {
public:
    std::string stateType;
    voidFunction callable;

    EventBinding(std::string ctgry, voidFunction func);
};


class InputManager{
public:
    std::map<std::string, std::vector<EventBinding>> bindings;
    std::vector<std::tuple<u8, std::string, std::string>> keyMaps;
    std::string state = GAME_RUNNING;

    InputManager();

    void registerBinding(std::string eventName, std::string stateType, voidFunction func);

    void pollInput();

    void pause();

    void unpause();
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
    u8 spriteId;
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

    bool checkCollision(int x, int y);
};


class StaticObject;

class Trigger;

void registerInputBindings(InputManager* eventManager, Sprite* player);

extern std::vector<Sprite> sprites;
extern u8 spriteIdIndex;


u8 getSpriteId();