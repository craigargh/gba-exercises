#include "gameobjects.hpp"


EventBinding::EventBinding(std::string ctgry, voidFunction func){
    stateType = ctgry;
    callable = func;
};


InputManager::InputManager(){
    keyMaps.push_back(std::make_tuple(KEY_UP, KEY_HELD, EVENT_UP));
    keyMaps.push_back(std::make_tuple(KEY_DOWN, KEY_HELD, EVENT_DOWN));
    keyMaps.push_back(std::make_tuple(KEY_RIGHT, KEY_HELD, EVENT_RIGHT));
    keyMaps.push_back(std::make_tuple(KEY_LEFT, KEY_HELD, EVENT_LEFT));
    keyMaps.push_back(std::make_tuple(KEY_START, KEY_PRESS, EVENT_PAUSE));
};


void InputManager::registerBinding(std::string eventName, std::string stateType, voidFunction func){
    bindings[eventName].push_back(EventBinding(stateType, func));
};

void InputManager::pollInput(){
    scanKeys();
    u16 heldKeys = keysHeld();
    u16 pressedKeys = keysDown();

    for (u8 i = 0; i < keyMaps.size(); i++){
        u8 keyCode = std::get<0>(keyMaps[i]);
        std::string keyAction = std::get<1>(keyMaps[i]);
        std::string eventKey = std::get<2>(keyMaps[i]);

        if (keyAction == KEY_HELD && (! (heldKeys & keyCode))){
            continue;
        }

        if (keyAction == KEY_PRESS && (! (pressedKeys & keyCode))){
            continue;
        }

        std::vector<EventBinding> allBindings = bindings.find(eventKey)->second;
        std::vector<EventBinding> activeBindings;

        for (u8 i=0; i < allBindings.size(); i++){
            if (state != allBindings[i].stateType){
               continue;
            }

            activeBindings.push_back(allBindings[i]);
        }


        for (u8 i=0; i < activeBindings.size(); i++){
            activeBindings[i].callable();
        }
    }
}


void InputManager::pause(){
    state = GAME_PAUSED;
};


void InputManager::unpause(){
    state = GAME_RUNNING;
};

AnimationFrame::AnimationFrame(u8 tile, u8 length){
    tileOffset = tile;
    duration = length;
}

Animation::Animation(){};

Animation::Animation(u16 baseTileValue){
    baseTile = baseTileValue;
    frame = 0;
    tileSetOffset=0;
};

void Animation::tick(){
    frame ++;
}


AnimationFrame Animation::currentFrame(){
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


u16 Animation::tile(){
    return baseTile + tileSetOffset + (currentFrame().tileOffset * 4);
}




Sprite::Sprite(){};

Sprite::Sprite(OBJATTR* oamRef, u8 xPos, u8 yPos){
    oam = oamRef;
    x = xPos;
    y = yPos;
};

void Sprite::draw(){
    // TODO: Implement width and height for different sized sprites

    oam->attr0 = OBJ_16_COLOR | ATTR0_SQUARE | OBJ_Y(y);
    oam->attr1 = ATTR1_SIZE_16 | OBJ_X(x);
    oam->attr2 = animation.tile() | ATTR2_PALETTE(0);

    if (animation.flip){
        oam->attr1 |= OBJ_HFLIP;
    }
};

void Sprite::update(){
    animation.tick();
};

void Sprite::faceNorth(){
    animation.tileSetOffset = 24;
    animation.flip = false;

    y = max(y - 1, 0);
};

void Sprite::faceSouth(){
    animation.tileSetOffset = 0;
    animation.flip = false;

    y = min(y + 1, SCREEN_HEIGHT - 16);
};

void Sprite::faceEast(){
    animation.tileSetOffset = 12;
    animation.flip = true;

    x = min(x + 1, SCREEN_WIDTH - 16);
};

void Sprite::faceWest(){
    animation.tileSetOffset = 12;
    animation.flip = false;

    x = max(x - 1, 0);
};




void registerInputBindings(InputManager* eventManager, Sprite* player){
    eventManager->registerBinding(EVENT_UP, GAME_RUNNING, std::bind(&Sprite::faceNorth, player));
    eventManager->registerBinding(EVENT_DOWN, GAME_RUNNING, std::bind(&Sprite::faceSouth, player));
    eventManager->registerBinding(EVENT_LEFT, GAME_RUNNING, std::bind(&Sprite::faceWest, player));
    eventManager->registerBinding(EVENT_RIGHT, GAME_RUNNING, std::bind(&Sprite::faceEast, player));

    eventManager->registerBinding(EVENT_PAUSE, GAME_PAUSED, std::bind(&InputManager::unpause, eventManager));
    eventManager->registerBinding(EVENT_PAUSE, GAME_RUNNING, std::bind(&InputManager::pause, eventManager));
}
