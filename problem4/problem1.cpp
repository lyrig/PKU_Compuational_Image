
#include <cstdio>
#include <random>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "rendering.h"
#include "img_saver.h"
#include "problem1.h"
#include "problem2.h"
#include "problem3.h"

const int Width = 800;
const int Height = 600;
unsigned char screen_buffer[Width * Height * 20];
unsigned char* screen_fb = screen_buffer;

std::mt19937 rng(time(NULL));

double My_rand(){
    double x = 0, y = 0;
    for (int i = 0; i < 32; i++)
        x = (x + (rng() & 1)) / 2;
    for (int i = 0; i < 32; i++)
        y = y * 2 + (rng() & 1);
    return x + y;
}

double My_rand(double l, double r){
    double v = My_rand();
    int t = floor(v / (r - l));
    return v - (r - l) * t;
}

void SaveImage(){
    time_t rawtime = time(NULL);
    struct tm localtm = *localtime(&rawtime);
    string name = std::to_string((int)localtm.tm_hour) + "_" +
                std::to_string((int)localtm.tm_min) + "_" + 
                std::to_string((int)localtm.tm_sec) + "_" +
                  "problem1.bmp";
    std::cout << "Image saved in " << name << std::endl;
    char* fname = new char[100];
    int len = name.length();
    for (int i = 0; i < len; i++) fname[i] = name[i];
    fname[len] = 0;
    saveImg(Width, Height, screen_fb, fname);
    delete[] fname;
}

int main(){

    std::cout << "Judging problem1\n";
    srand(20240314);
    Rendering device(Width, Height, screen_fb);
    device.switchMode() = 1; // simple texture mapping
    Model cube;
    cube.InitCube();
    Model circ;
    circ.InitCirc();

    Texture BlackWhite;
    BlackWhite.LoadBlackWhiteLarge();
    cube.texture = &BlackWhite;

    Texture Blue;
    Blue.LoadPureColor(Color::blue());
    circ.texture = &Blue;
    
    vector<Model> Scene;
    for (int i = 0; i < 3; i++)
        for (int j = 1; j < 3; j++){
            Matrix4 translate;
            translate.SetTranslate(My_rand(-1.0, 1.0), 2.0 * j, 2.0 * i);
            Matrix4 rotate;
            rotate.SetRotate(My_rand(-1.0, 1.0), My_rand(-1.0, 1.0), My_rand(-1.0, 1.0), My_rand(-1.0, 1.0));

            if ((i + j + 1) & 1){
                cube.WorldMat = translate * rotate;
                Scene.push_back(cube);
            }
            else{
                circ.WorldMat = translate * rotate;
                Scene.push_back(circ);
            }
        }

    device.refresh(Color(1.0, 0.95, 0.8));
    device.drawScene(Scene);
    device.drawScreen();
    SaveImage();

    std::cout << "Finish judging problem1\n";

    return 0;
}
