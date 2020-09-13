// increase game speed
// add sound effects
// start game when the player presses start

#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>
#include <gba_input.h>
#include <string.h>
#include <vector>


struct Position {
    u8 x;
    u8 y;
};

struct Snake{
    std::vector<Position> positions;
    u8 direction;
    Position oldTail;
    bool eaten;
};


const u16 snakeColour = RGB5(0, 0, 0);
const u16 bgColour = RGB5(31, 31, 31);
const u16 foodColour = RGB5(31, 0, 0);
const u8 squareSize = 10;

u16 keysPressed = 0;
u16 prevDirection = 1;
u16 newDirection = 1;
u16 frameCount = 0;
Position food;




int __qran_seed= 42;

int randomSeed(int seed){   
    int old= __qran_seed;
    __qran_seed= seed; 
    return old; 
}

int random(){   
    __qran_seed= 1664525*__qran_seed+1013904223;
    return (__qran_seed>>16) & 0x7FFF;
}

int randRange(int min, int max){
    return (random()*(max-min)>>15)+min;
}

u8 randX(){
    return randRange(0, 23) * squareSize;
}

u8 randY(){
    return randRange(0, 15) * squareSize;
}

u16 keyPressed(u16 keyCode){
    return keyCode & keysPressed;
}

u16 getDirection(){
    if (keyPressed(KEY_UP) && prevDirection != 2){
        return 0;
    } else if (keyPressed(KEY_RIGHT) && prevDirection != 3){
        return 1;
    } else if (keyPressed(KEY_DOWN) && prevDirection != 0){
        return 2;
    } else if (keyPressed(KEY_LEFT) && prevDirection != 1){
        return 3;
    } else {
        return newDirection;
    }
}

void drawRect(u8 x, u8 y, u8 width, u8 height, u16 colour){
    for (int i=0; i < height; i++){
        for (int j=0; j < width; j++){
            MODE3_FB[i + y][j + x] = colour;
        }
    }
}


Snake initSnake(){
    Snake snake;
    snake.positions = {
        {30, 20},
        {20, 20}
    };
    snake.direction = 1;
    snake.oldTail = {10, 20};
    snake.eaten = false;

    return snake;
}

void drawFullSnake(Snake snake){
    for (u16 i = 0; i < snake.positions.size(); i++){
        u8 x = snake.positions[i].x;
        u8 y = snake.positions[i].y;
        drawRect(x, y, squareSize, squareSize, snakeColour);
    }
}

bool checkFood(Snake snake){
    Position head = snake.positions[0];
    return (head.x == food.x) && (head.y == food.y);
}

Snake moveSnake(Snake snake, u8 direction){
    Snake newSnake;
    newSnake.positions = snake.positions;
    newSnake.direction = direction;
    newSnake.eaten = false;

    Position prevHead = newSnake.positions[0];
    u8 x = prevHead.x;
    u8 y = prevHead.y;

    if (direction == 0){
        y -= squareSize;
    } else if (direction == 1){
        x += squareSize;
    } else if (direction == 2){
        y += squareSize;
    } else if (direction == 3){
        x -= squareSize;
    }

    Position newHead = {x, y};
    newSnake.positions.insert(newSnake.positions.begin(), newHead);
    
    newSnake.eaten = checkFood(newSnake);

    if (!newSnake.eaten){
        newSnake.oldTail = newSnake.positions.back();
        newSnake.positions.pop_back();
    }

    return newSnake;
}

void redrawSnake(Snake snake){
    Position newHead = snake.positions[0]; 
    Position oldTail = snake.oldTail;

    drawRect(newHead.x, newHead.y, squareSize, squareSize, snakeColour);
    drawRect(oldTail.x, oldTail.y, squareSize, squareSize, bgColour);
}

void makeFood(Snake snake){
    bool clashes;
    u8 x, y;

    do {
        clashes = false;
        x = randX();
        y = randY();

        for (u8 i = 0; i < snake.positions.size(); i++){
            Position part = snake.positions[i];

            if (part.x == x && part.y == y){
                clashes = true;
            }
        }
    } while(clashes);

    food = {x, y};
    drawRect(food.x, food.y, squareSize, squareSize, foodColour);
}

Snake resetGame(){
    drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bgColour);

    Snake currentSnake = initSnake();
    drawFullSnake(currentSnake);

    frameCount = 0;
    prevDirection = 1;
    newDirection = 1;
    makeFood(currentSnake);

    return currentSnake;
}

bool deathCollision(Snake snake){
    bool death = false;

    if (snake.positions[0].x >= SCREEN_WIDTH - 1){
        death = true;
    } else if (snake.positions[0].y >= SCREEN_HEIGHT - 1){
        death = true;
    } else if (snake.positions[0].x < 0){
        death = true;
    } else if (snake.positions[0].y < 0){
        death = true;
    }

    Position head = snake.positions[0];
    for (u16 i = 1; i < snake.positions.size(); i++){
        Position body = snake.positions[i];

        bool same_x = body.x == head.x;
        bool same_y = body.y == head.y;

        if (same_x && same_y){
            death = true;
        }
    }

    return death;
}

u8 calculateSpeed(Snake snake){
    u8 length = snake.positions.size();
    if (length > 18)
        return 2;
    else if (length > 28)
        return 1;

    else
        return 20 - length;
}

int main(void) {
    SetMode(MODE_3 |  BG2_ENABLE);

    irqInit();
    irqEnable(IRQ_VBLANK);

    scanKeys();
    
    Snake currentSnake = resetGame();
    
    while (1) {
        VBlankIntrWait();
        scanKeys();
        keysPressed = keysDown();

        newDirection = getDirection();

        if (keyPressed(KEY_START)){
            currentSnake = resetGame();
        }

        if (frameCount == calculateSpeed(currentSnake)){
            Snake newSnake = moveSnake(currentSnake, newDirection);
            
            if (deathCollision(newSnake)){
                currentSnake = resetGame();
            } else {
                redrawSnake(newSnake);

                if (newSnake.eaten){
                    makeFood(newSnake);
                }

                currentSnake = newSnake;
                prevDirection = newDirection;
                frameCount = 0;
            }
        } else {
            frameCount++;
        }

        
    }
}


