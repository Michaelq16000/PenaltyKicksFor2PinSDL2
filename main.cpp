#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include<GL/glu.h>
#include <iostream>
#include <chrono>
#include <thread>

//controls
//shooter:
//hold space to shoot, lift when the bar goes green to shoot with just enough power
//pick direction with A and D keys
//goalkeeper:
//pick the side to defend with left and right arrow before the shooter kicks the ball
//the one who makes the background their player color first wins, if anyone gets 10 points the game resets

//no way to play single player, only 2P mode, no switching sides in the middle of the game

using namespace std::chrono_literals;

//used to determine if the kick's power is right, change to make it harder to shoot right
//tooWeak must be in the range of 0-31 and tooStrong must be in the range of 33-64
//32-tooWeak should be less than 64-tooStrong, it goes well with power bar's visuals
//could
const int tooWeak = 29, tooStrong = 40;
// SpaceHeld = 0: start, 1: space pressed, 2: space let go
//for directions: -1 means left, 1 means right
int power = 0, spaceHeld = 0, gkDirection = 0, shootingDirection = 0, gkDirectionIntent = 0, shootingDirectionIntent = 0;
float gkScore = 0, shooterScore = 0, r,g,b;
void init()
{
    glClearColor(0.0, 0.6, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45,640.0/480.0,1.0,500.0);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

int powerOk()//used for determining if the power of shooter's kick is right
{
    if(power < tooWeak) return 0;//not enough
    if(power > tooStrong) return 2;//too much
    return 1;//just right
}

void DrawSquare()
{
    glBegin(GL_QUADS);
    glVertex3f(-0.5, -0.5, 0.0);
    glVertex3f(0.5, -0.5, 0.0);
    glVertex3f(0.5, 0.5, 0.0);
    glVertex3f(-0.5, 0.5, 0.0);
    glEnd();
}

void DrawRobot(bool kicker)
{
    // Body
    glPushMatrix();
    glTranslatef(0.0, 0.0, 0.0);
    glScalef(0.5, 1.0, 0.5);
    DrawSquare();
    glPopMatrix();
    // Head
    glPushMatrix();
    glTranslatef(0.0, 0.65, 0.0);
    glScalef(0.3, 0.3, 0.3);
    DrawSquare();
    glPopMatrix();
    // Left arm
    glPushMatrix();
    glTranslatef(-0.35, 0.25, 0.0);
    glScalef(0.2, 0.4, 0.2);
    DrawSquare();
    glPopMatrix();
    // Right arm
    glPushMatrix();
    glTranslatef(0.35, 0.25, 0.0);
    glScalef(0.2, 0.4, 0.2);
    DrawSquare();
    glPopMatrix();
    // Left leg
    glPushMatrix();
    glTranslatef(-0.3, -0.65, 0.0);
    glScalef(0.2, 0.4, 0.2);
    DrawSquare();
    glPopMatrix();
    // Right leg
    if(kicker)
    {
        glPushMatrix();
        glTranslatef(0.3, -0.55, 0.0);
        glRotatef(45, 1.0, 0.0, 1.0);
        glScalef(0.2, 0.4, 0.2);
        DrawSquare();
        glPopMatrix();
    }
    else
    {
        glPushMatrix();
        glTranslatef(0.3, -0.65, 0.0);
        glScalef(0.2, 0.4, 0.2);
        DrawSquare();
        glPopMatrix();
    }
}

void powerBarColor()
{
    float r = 1.0, g = 1.0;

    if (power < 16)// Red to Yellow
    {
        g = power / 15.0;
    }
    else if (power < 32)// Yellow to Green
    {
        r = 1.0 - (power - 16) / 15.0;
    }
    else if (power < 48)// Green to Yellow
    {
        r = (power - 32) / 15.0;
    }
    else if (power <= 64)// Yellow to Red
    {
        g = (64 - power) / 15.0;
    }
    glColor3f(r, g, 0.0);
}

void drawGoalpost()
{
    glPushMatrix();
    glTranslatef(0.0,0.0,-0.5);
    glScalef(1.3,1.3,1.3);
    glPushMatrix();
    glTranslatef(-1, 0.2, 0.0);
    glScalef(0.1, 1.0, 1.0);
    DrawSquare();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0, 0.7, 0.0);
    glScalef(2.0, 0.1, 1.0);
    DrawSquare();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.0, 0.2, 0.0);
    glScalef(0.1, 1.0, 1.0);
    DrawSquare();
    glPopMatrix();
    glPopMatrix();
}

void drawBall()
{
    if(spaceHeld == 0 || spaceHeld == 1)//before kick
    {
        glPushMatrix();
        glTranslatef(0.0,-0.85,1.0);
        glScalef(0.1,0.1,0.1);
        DrawSquare();
        glPopMatrix();
    }
    else//after kick
    {
        switch(powerOk())
        {
        case 0://too weak, ball lands at gk's feet
            glPushMatrix();
            glTranslatef(0.0,-0.55,0.8);
            glScalef(0.1,0.1,0.1);
            DrawSquare();
            glPopMatrix();
            break;
        case 1://just right
            switch(shootingDirection)
            {
            case -1://left
                glPushMatrix();
                glTranslatef(-0.7, 0.17, -0.25);
                glScalef(0.1,0.1,0.1);
                DrawSquare();
                glPopMatrix();
                break;
            case 0://center
                glPushMatrix();
                glTranslatef(0.0, -0.25, -0.25);
                glScalef(0.1,0.1,0.1);
                DrawSquare();
                glPopMatrix();
                break;
            case 1://right
                glPushMatrix();
                glTranslatef(0.7, 0.17, -0.25);
                glScalef(0.1,0.1,0.1);
                DrawSquare();
                glPopMatrix();
                break;
            }
            break;
        case 2://too strong, above the goalpost
            glPushMatrix();
            glTranslatef(0.2,2.0,-4.0);
            glScalef(0.1,0.1,0.1);
            DrawSquare();
            glPopMatrix();
            break;
        }
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glClearColor(r, g, b, 1.0);
    glTranslatef(0.0,0.0,-4.0);
    glColor3f(1.0, 1.0, 1.0);

    drawBall();
    drawGoalpost();

    if(spaceHeld == 1)// Power bar
    {
        glPushMatrix();
        glTranslatef(0.0,-1.5,0.0);
        glScalef(1.0,0.2,1.0);
        powerBarColor();
        DrawSquare();
        glPopMatrix();
    }

    glPushMatrix();// RobotGK
    glTranslatef(0.0,0.0,-0.25);
    glScalef(0.6,0.6,0.6);
    if(r >= 1) glColor3f(1.0, 0.9, 0.0);
    else glColor3f(0.9,0.7,0.7);
    DrawRobot(false);
    glPopMatrix();

    glPushMatrix();// RobotShooter
    glTranslatef(-0.25,-0.5,1.1);
    glScalef(0.5,0.5,0.5);
    if(b >= 1) glColor3f(1.0, 0.9, 0.0);
    else glColor3f(0.7,0.7,0.9);
    if(spaceHeld == 2) DrawRobot(true);
    else DrawRobot(false);
    glPopMatrix();

    if(gkDirection == 1)// RobotGK's arm goes right
    {
        glPushMatrix();
        glTranslatef(0.55, 0.15, -0.25);
        glScalef(0.8, 0.2, 0.2);
        glColor3f(0.8,0.7,0.7);
        DrawSquare();
        glPopMatrix();
    }
    if(gkDirection == -1)// RobotGK's arm goes left
    {
        glPushMatrix();
        glTranslatef(-0.55, 0.15, -0.25);
        glScalef(0.8, 0.2, 0.2);
        glColor3f(0.8,0.7,0.7);
        DrawSquare();
        glPopMatrix();
    }
    glEnd();
}

bool ifWin()
{
    if(r == 1 || b == 1) return true;
    if(gkScore == 10 || shooterScore == 10) return true;
    else return false;
}

int main(int argc, char* args[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

    SDL_Window* window = SDL_CreateWindow("RoboPenaltyKicks 2025 for 2 players Delux Premium (Collector's Edition)",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1280, 960,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glcontext);

    int loop = 1;
    SDL_Event event;
    init();
    while (loop==1)
    {
        r = (gkScore/5)-(shooterScore/10),g = 0.6-(0.6*((gkScore+shooterScore)/5)), b = (shooterScore/5)-(gkScore/10);
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
                loop = 0;
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym==SDLK_SPACE)
                {
                    spaceHeld = 1;
                    if(ifWin()) loop = 0;//end game
                }
                if (event.key.keysym.sym==SDLK_a) shootingDirectionIntent = -1;
                if (event.key.keysym.sym==SDLK_d) shootingDirectionIntent = 1;
                if (event.key.keysym.sym==SDLK_LEFT) gkDirectionIntent = -1;
                if (event.key.keysym.sym==SDLK_RIGHT) gkDirectionIntent = 1;
                break;

            case SDL_KEYUP:
                if(event.key.keysym.sym==SDLK_SPACE) spaceHeld = 2;
                break;
            }
        }

        if(spaceHeld == 1 && power < 64) power++;
        if(spaceHeld == 2)// Space being let go, on kick
        {
            gkDirection=gkDirectionIntent;
            shootingDirection=shootingDirectionIntent;
            switch(powerOk())
            {
            case 0:
                gkScore++;
                break;
            case 1:
                if(shootingDirection==gkDirection) gkScore++;
                else shooterScore++;
                break;
            case 2:
                gkScore++;
                break;
            }
        }
        std::cout <<"gk " << gkScore << " sh " << shooterScore << std::endl;
        display();
        SDL_GL_SwapWindow(window);
        if(spaceHeld == 2)//on kick but after display
        {
            std::this_thread::sleep_for(1500ms);
            spaceHeld = 0;//resetting the game
            power = 0;
            gkDirection = 0;
            gkDirectionIntent = 0;
            shootingDirection = 0;
            shootingDirectionIntent = 0;
        }
    }

    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
