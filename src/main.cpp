#include <raylib.h>
#include <iostream>


using namespace std;

// Define GameState enumeration
enum GameState { MENU, GAME, OPTIONS, EXIT, GAMEOVER };                  //abstract class

class GameObject {
protected:
    Vector2 position;
    Texture2D texture;

public:
                                                      // Constructor to load the texture and initialize the position
    GameObject(const char* texturePath, float x=0, float y=0) {
        texture=LoadTexture(texturePath);
        position={x, y};
    }

    virtual void update()=0;                       //for runtime polymorphism     ( Pure virtual function for updating objects)
    virtual void draw() {
        DrawTexture(texture, position.x, position.y, WHITE);
    }

    Texture2D getTexture() 
    { 
        return texture;
     }
    Vector2& getPosition()
     { 
        return position;
         }
    virtual ~GameObject() {
        UnloadTexture(texture);
    }
};

class Bucket : public GameObject {
private:
    int x_Left_Boundary;
    int x_Right_Boundary;

public:
    Bucket() : GameObject("bucket.png", 450, GetScreenHeight()-100) {
        x_Left_Boundary=0;
        x_Right_Boundary=900;
    }

    void move() {
        if(IsKeyDown(KEY_LEFT)) {
            position.x-=12;
            if(position.x<x_Left_Boundary) {
                position.x=x_Left_Boundary;
            }
        }
        if(IsKeyDown(KEY_RIGHT)) {
            position.x+=12;
            if(position.x+texture.width>x_Right_Boundary) {
                position.x=x_Right_Boundary-texture.width;
            }
        }
    }

    void update() override {
        move();
    }
};

class Apple : public GameObject {
private:
    float speed;

public:
                                                                  // initialize apple with random position and speed
    Apple() : GameObject("apple.png", GetRandomValue(0, 700), GetRandomValue(20, 100)) {
        speed=5.0f;
    }
     void positionReset() {
        position.y=GetRandomValue(20, 100);
        position.x=GetRandomValue(0, 700);
    }

    float getSpeed() const {
        return speed;
    }
    void Resetapple(){
        speed = 5;
    }

    void increaseSpeed(float increment) {
        speed+=increment; 
    }

    void update() override {
        position.y+=speed;
        if(position.y>GetScreenHeight()) {
            positionReset();                                          // Reset position if it goes off-screen
        }
    }
};

class Bomb : public GameObject {
private:
    float speed;

public:
    // Initialize bomb with random position and speed
    Bomb() : GameObject("bomb.png", GetRandomValue(0, 700), -100) {   //-100 to make apple and bobmp appear at different time 
        speed=7;
    }

    void update() override {
        position.y+=speed; // Update bomb position based on speed
        if(position.y>GetScreenHeight()) {
            reset(); // Reset position if it goes off-screen
        }
    }

    void reset() {
        position.y=-100;
        position.x=GetRandomValue(0, 700); // Reset position to random x
    }
};

class CollisionFunction {
private:
    int scoreIncrementThreshold=10;                                       // Points required to increase speed

public:
    int check=0;                                                         // Counter for score-based checks

    void checkCollision(Bucket& bucket, Apple& apple, Bomb& bomb, int& score, int& lives, GameState& currentState) {
                                                                       // Check collision between bucket and apple
        int bucketX=bucket.getPosition().x;
        int bucketWidth=bucket.getTexture().width;
        int bucketY=bucket.getPosition().y;

        int appleX=apple.getPosition().x;
        int appleY=apple.getPosition().y;
        int appleWidth=apple.getTexture().width;
        int appleHeight=apple.getTexture().height;

        if(appleY+appleHeight>=bucketY &&
           appleX+appleWidth>bucketX && appleX<bucketX+bucketWidth) {
            score++;   
            check++;
            apple.positionReset(); // Reset apple position after catching it

            if(check>=scoreIncrementThreshold) {
                apple.increaseSpeed(1.5f); // Increase speed after threshold
                check=0;
            }
        } else if(appleY+appleHeight>GetScreenHeight()) {
            lives--;
            apple.positionReset(); // Reset apple if it falls off the screen
            if(lives<=0) {
                currentState=GAMEOVER; // Switch to GAMEOVER if no lives left
            }
        }

        // Check collision between bucket and bomb
        int bombX=bomb.getPosition().x;
        int bombY=bomb.getPosition().y;
        int bombWidth=bomb.getTexture().width;
        int bombHeight=bomb.getTexture().height;

        if(bombY+bombHeight>=bucketY &&
           bombX+bombWidth>bucketX && bombX<bucketX+bucketWidth) {
            lives--;
            bomb.reset(); // Reset bomb position after collision
            if(lives<=0) {
                currentState=GAMEOVER; // Switch to GAMEOVER if no lives left
            }
        }
    }
};

int main() {
    InitWindow(900, 700, "Apple Catcher");
    InitAudioDevice();                               // Initialize audio device
    SetTargetFPS(60);

    Bomb bomb;
    Bucket bucket;
    Apple apple;
    CollisionFunction collision;
    Texture2D backgroundImage=LoadTexture("tree.png");
    Texture2D menuImage=LoadTexture("menu.png");
    Texture2D gameOverImage=LoadTexture("gameover.png");

    Sound backgroundSound=LoadSound("sound.mp3");                  // Load background sound
    Sound gameOverSound=LoadSound("gameover.mp3");                  // Load game over sound

    PlaySound(backgroundSound);                                    // Start playing background sound

    int score=0;
    int lives=5;
    GameState currentState=MENU;
    bool gameOverSoundPlayed=false;

    Rectangle startButton={300, 280, 300, 65};
    Rectangle optionsButton={300, 375, 300, 65};
    Rectangle exitButton={300, 470, 300, 65};

    while(!WindowShouldClose()) {
        if(currentState!=GAMEOVER && !IsSoundPlaying(backgroundSound)) {
            PlaySound(backgroundSound);                                       // Restart background sound
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch(currentState) {
            case MENU: {
                DrawTexture(menuImage, 0, 0, WHITE);
                Vector2 mousePos=GetMousePosition();

                if(CheckCollisionPointRec(mousePos, startButton))
                    DrawRectangleLinesEx(startButton, 5, BLACK);
                if(CheckCollisionPointRec(mousePos, optionsButton))
                    DrawRectangleLinesEx(optionsButton, 5, BLACK);
                if(CheckCollisionPointRec(mousePos, exitButton))
                    DrawRectangleLinesEx(exitButton, 5, BLACK);

                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if(CheckCollisionPointRec(mousePos, startButton)) {
                        currentState=GAME;
                        score=0;
                        lives=5;
                        apple.Resetapple();
                        apple.positionReset();
                    } else if(CheckCollisionPointRec(mousePos, optionsButton)) {
                        currentState=OPTIONS;
                    } else if(CheckCollisionPointRec(mousePos, exitButton)) {
                        currentState=EXIT;
                    }
                }
                gameOverSoundPlayed=false;
            } break;

            case GAME: {
                DrawTexture(backgroundImage, 0, 0, WHITE);
                bucket.update();
                apple.update();
                bomb.update();

                collision.checkCollision(bucket, apple, bomb, score, lives, currentState);
                bucket.draw();
                apple.draw();
                bomb.draw();
                DrawText(TextFormat("Score: %d", score), 740, 10, 30, WHITE);
                DrawText(TextFormat("Lives: %d", lives), 770, 40, 30, WHITE);

                if(IsKeyPressed(KEY_ESCAPE)) {
                    currentState=MENU;
                }
            } break;

            case OPTIONS: {
                static int highScore=0;
                if(score>highScore) {
                    highScore=score;
                }

                DrawText("Options Menu", 170, 250,70, DARKGRAY);
                DrawText(TextFormat("High Score: %d", highScore), 170, 350, 40, BLACK);
                DrawText("Press  ENTER  to return", 170, 400, 40, DARKGRAY);

                if(IsKeyPressed(KEY_ENTER)) {
                    currentState=MENU;
                }
            } break;

            case GAMEOVER: {
                DrawTexture(gameOverImage, 0, 0, WHITE);
                DrawText("Press ENTER to return to menu", 170, 520, 35, WHITE);

                if(!gameOverSoundPlayed) {
                    StopSound(backgroundSound);
                    PlaySound(gameOverSound);
                    gameOverSoundPlayed=true;
                }

                if(IsKeyPressed(KEY_ENTER)) {
                    currentState=MENU;
                    StopSound(gameOverSound);
                    PlaySound(backgroundSound);
                }
            } break;

            case EXIT: {
               CloseWindow();
                return 0;
            }
        }

        EndDrawing();
    }

    UnloadSound(backgroundSound);
    UnloadSound(gameOverSound);
    UnloadTexture(menuImage);
    UnloadTexture(backgroundImage);
    UnloadTexture(gameOverImage);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}