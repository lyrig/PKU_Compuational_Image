#include "rendering.h"

/*
Camera 类内包含的变量：

        Vector4 eyePos;      // 相机位置
        Vector4 directFront; // 相机视角下，向前的方向
        Vector4 directUp;    // 相机视角下，向上的方向
        Vector4 directX;     // 相机视角下，向右的方向

        double zNear = 0.1, zFar = 500; // 相机近点，远点
        double FofV = 90;               // 相机视角大小
        double ratio = 4.0 / 3.0;       // 相机画布长宽比
*/

/*
Problem 1.(20%)
实现将三维空间内的任意点 p 转换到相机的局部投影坐标系下的点 q 的相互转换：
转换方式：
    Step 1: 把三维坐标系内的任意点 p 转换到以 directFront 为正上，directUp 为正前，directX 为正右，eyePos 为相机原点的相机坐标系下的点 m。
    Step 2: 把相机坐标系下的点 m 进行投影变换, 转换到相机的投影坐标系下的点 q。
Step 1,2 两步均可以写成矩阵乘法的形式，你需要补充矩阵乘法中的矩阵部分。
*/

Matrix4 Camera::cameraViewTrans(){
    /*
    problem 1.1(10%)
    Step 1: 把三维坐标系内的任意点 p 转换到以 directFront 为正上，directUp 为正前，directX 为正右，eyePos 为相机原点的相机坐标系下的点 m
    要求：输出可逆矩阵 M，满足 M * p = m
    */

    // problem 1.1(10%)
    // 在此处填写你的代码：
    Matrix4 result;
    Matrix4 R, T;
    T.SetTranslate(-eyePos.x, -eyePos.y, -eyePos.z); // 相机平移矩阵
    R = Matrix4({directX.x, directX.y, directX.z, 0, 
                directUp.x, directUp.y, directUp.z, 0,
                directFront.x, directFront.y, directFront.z, 0,
                0, 0, 0, 1});
    result = R * T;
    return result;
}

Matrix4 Camera::cameraProject(){
    /*
    problem 1.2(10%)    
    Step 2: 把相机坐标系下的点 m 进行投影变换, 转换到相机的投影坐标系下的点 q。
    要求：输出矩阵 M，满足 M * m = q
    */
    // problem 1.2(10%)
    // 在此处填写你的代码：
    Matrix4 result;
    double n = zNear, f = zFar;
    double x, y;
    y = 2 * n;
    x = ratio * y;
    double l = - x / 2, r = x / 2;
    double b = - y / 2, t = y / 2;

    result = Matrix4({n / r, 0, 0, 0, 
                        0, n / t, 0, 0, 
                        0, 0, (f+n) / (f-n), -2*n*f/(f-n), 
                        0, 0, 1, 0});
    // result = Matrix4()
    return result;
}

Vector4 Camera::transProject(Vector4 p){
    Matrix4 transmat = cameraProject() * cameraViewTrans();
    return transmat * p;
}

Vector4 Camera::InvtransProject(Vector4 q){
    Matrix4 transmat = cameraProject() * cameraViewTrans();
    return transmat.Inverted() * q;
}