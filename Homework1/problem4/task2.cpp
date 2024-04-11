#include <cstdio>
#include <random>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <conio.h>
// #include "opencv2/opencv.hpp"

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
// std::mt19937 rng(0);

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
                  "problem2.bmp";
    std::cout << "Image saved in " << name << std::endl;
    char* fname = new char[100];
    int len = name.length();
    for (int i = 0; i < len; i++) fname[i] = name[i];
    fname[len] = 0;
    saveImg(Width, Height, screen_fb, fname);
    delete[] fname;
}

void SaveImageTmp(){
    time_t rawtime = time(NULL);
    struct tm localtm = *localtime(&rawtime);
    string name = "tmp.bmp";
    // std::cout << "Image saved in " << name << std::endl;
    char* fname = new char[100];
    int len = name.length();
    for (int i = 0; i < len; i++) fname[i] = name[i];
    fname[len] = 0;
    saveImg(Width, Height, screen_fb, fname);
    delete[] fname;
}

// void UpdateWindow(){
//     cv::Mat img = cv::imread("./tmp.bmp");
// 	if (img.empty())
// 		std::cout << "image is empty or the path is invalid!" << std::endl;
// 	cv::imshow("Task3", img);
// 	cv::waitKey(0);
// 	cv::destroyAllWindows();
//     std::cout << "Update Window\n";
// }

int main(){
    std::cout << "Judging Task2\n";
    srand(20240314);
    Rendering device(Width, Height, screen_fb);
    device.switchMode() = 1; // simple texture mapping
    device.renderType = 3; // Ray Tracing

    Texture BlackWhite;
    BlackWhite.LoadBlackWhiteLarge();
    Texture White;
    White.LoadPureColor(Color(1.0, 1.0, 1.0));
    Texture Blue;
    Blue.LoadPureColor(Color(1, 0.4, 0.2));

    vector<Model> Scene; // Initial the Scene
    Matrix4 translate;
    Matrix4 rotate;

    // 为了更好表示Ray Tracing，需要重新布置场景
    // 场景大概类似于Cornell Box，颜色，光源信息不同，而且由于我不太会调模型位置，所以好像最终结果盒子是悬在空中的？
    // 方块1（小的那个）
    Model cube;
    cube.name = "Small Cube";
    cube.InitCube(0.3);
    cube.texture = &Blue; // Cube is BlackAndWhite
    translate.SetTranslate(0.5, 0.0, 1.5);
    rotate.SetRotate(0.0 , 0.5, 0.0, 0.5);
    cube.WorldMat = translate * rotate;
    Scene.push_back(cube);

    // 方块2 （大的那个）
    Model BigCube;
    BigCube.name = "Cube Big";
    BigCube.InitCube(0.4);
    BigCube.texture = &BlackWhite;
    translate.SetTranslate(-0.8, 0.0, 1.5);
    rotate.SetRotate(0.0 , -0.5, 0.0, 0.5);
    BigCube.WorldMat = translate * rotate;
    Scene.push_back(BigCube);

    // // 后面的墙
    Model WallBack;
    WallBack.name = "WallBack";
    WallBack.InitWall(4.0);
    WallBack.texture = &White;
    translate.SetTranslate(0.0, 0.0, 4);
    WallBack.WorldMat = translate;
    Scene.push_back(WallBack);    

    // // 左边的墙
    Model WallLeft;
    WallLeft.name = "WallLeft";
    WallLeft.InitWall(4.0);
    WallLeft.texture = &White;
    translate.SetTranslate(-3.0, 0.0, 5);
    rotate.SetRotate(0.0 , 1.0, 0.0, PI / 2.0);
    WallLeft.WorldMat = translate * rotate;
    Scene.push_back(WallLeft); 

    // // 右边的墙
    Model WallRight;
    WallRight.name = "WallRight";
    WallRight.InitWall(4.0);
    WallRight.texture = &White;
    translate.SetTranslate(3.0, 0.0, 5);
    rotate.SetRotate(0.0 , 1.0, 0.0, -PI / 2.0);
    WallRight.WorldMat = translate * rotate;
    Scene.push_back(WallRight);    

    // 下边的墙
    Model WallBot;
    WallBot.name = "WallBot";
    WallBot.InitWall(3.0);
    WallBot.texture = &White;
    translate.SetTranslate(0.0, -1.0, 4);
    rotate.SetRotate(1.0 , 0.0, 0.0, -PI / 2.0);
    WallBot.WorldMat = translate * rotate;
    Scene.push_back(WallBot);  

    // // 上边的墙
    Model WallTop;
    WallTop.name = "WallTop";
    WallTop.InitWall(3.0);
    WallTop.texture = &White;
    translate.SetTranslate(0.0, 3.0, 4);
    rotate.SetRotate(1.0 , 0.0, 0.0, PI / 2.0);
    WallTop.WorldMat = translate * rotate;
    Scene.push_back(WallTop);     
    
    device.camera.setCamera(Vector4(0.0, 0.5, 0.0, 0.0));
    device.light.Clear(); // 需要重新初始化光源，因为相机位置也改了,而且我们需要重新设置光源位置
    // // device.light.ka = 0.0; // 用于调试，观察点光源和平行光的效果(把环境光去掉)
    device.light.DotLight(Vector4(0.0, 2, 1.0, 0.0));
    device.light.ParallelLight(Vector4(0.0, 1.0, -1.0, 0.0));
    device.refresh(Color(1.0, 0.95, 0.8));
    device.drawScene(Scene);
    device.drawScreen();
    SaveImageTmp();
    // UpdateWindow();
    // int ch;
    // // 键盘调用窗口
    // while (1){
    //     if (_kbhit()){//如果有按键按下，则_kbhit()函数返回真
    //         ch = _getch();//使用_getch()函数获取按下的键值
    //         cout <<"You Enter : " <<char(ch) << std::endl;
    //         if (ch == 27){ break; }//当按下ESC时循环，ESC键的键值时27.
    //         if (ch == int('F')) // 当按下F时，保存当前图片为永久图片
    //         {
    //             SaveImage();
    //         }
    //         string key;
    //         key = char(ch);
    //         // 如果不是退出的话，更新窗口
    //         device.camera.KeyboardUpd(key);
    //         device.refresh(Color(1.0, 0.95, 0.8));
    //         device.drawScene(Scene);
    //         device.drawScreen();
    //         SaveImageTmp();
    //         UpdateWindow();
    //     }
    // }
    // system("pause");
    std::cout << "Finish judging Task2\n";

    return 0;
}


