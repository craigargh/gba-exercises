#include "gameobjects.hpp"


EventCallable::EventCallable(std::string ctgry, voidFunction func){
    stateType = ctgry;
    callable = func;
};


InputManager::InputManager(){
    keyMaps.push_back(std::make_tuple(KEY_UP, "up"));
    keyMaps.push_back(std::make_tuple(KEY_DOWN, "down"));
    keyMaps.push_back(std::make_tuple(KEY_RIGHT, "right"));
    keyMaps.push_back(std::make_tuple(KEY_LEFT, "left"));
};


void InputManager::registerListener(std::string eventName, voidFunction func){
    listenerMap[eventName].push_back(EventCallable("game", func));
};

void InputManager::pollInput(){
    scanKeys();
    u16 keysPressed = keysHeld();

    for (u8 i = 0; i < keyMaps.size(); i++){
        u8 keyCode = std::get<0>(keyMaps[i]);
        std::string eventKey = std::get<1>(keyMaps[i]);

        if (! (keysPressed & keyCode)){
            continue;
        }

        std::vector<EventCallable> eventCallable = listenerMap.find(eventKey)->second;

        for (u8 i=0; i < eventCallable.size(); i++){
            if (state != eventCallable[i].stateType){
               continue;
            }

            eventCallable[i].callable();
        }
        
    }    
}


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








void registerPlayerListeners(InputManager* eventManager, Sprite* player){
    eventManager->registerListener("up", std::bind(&Sprite::faceNorth, player));
    eventManager->registerListener("down", std::bind(&Sprite::faceSouth, player));
    eventManager->registerListener("left", std::bind(&Sprite::faceWest, player));
    eventManager->registerListener("right", std::bind(&Sprite::faceEast, player));
}
