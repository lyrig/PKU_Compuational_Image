#include "rendering.h"



inline double Dot3(const Vector4 &a, const Vector4& b){
    return a.x * b.x + a.y * b.y + a.z * b.z;
}



/*
    problem 2.1 (10%)
    实现 Phong 着色模中的法线计算
    输入：
        loc: 当前点的位置，在投影后屏幕坐标系下存储
        tri：一个Triangle 对象 `rendering.h:73`。 其中
            tri.vertices 记录了三个顶点的信息，其中
                tri.vertices[i].pos     记录了三个顶点在投影后屏幕坐标系的坐标
                tri.vertices[i].normal  记录了三个顶点在世界坐标系下的法线向量

        上述loc, pos 中 x,y 分量代表屏幕坐标，z分量代表深度

    输出：
        当前点 loc 对应的世界坐标系下的法线向量，该向量会被用于后面的 Blinn-Phong光照计算

    提示：
        你可能需要根据 loc 和三个顶点的 pos 来计算重心坐标
        使用重心坐标插值时注意深度的影响
*/
Vector3 Rendering::InterpolateNormal(const Vector4 &loc, const Triangle &tri)
{
    // problem 2.1: 10%
    // 在此处填写你的代码：

    Vector3 result;

    double alpha, beta, gamma;
    gamma = ((tri.vertices[0].pos.y - tri.vertices[1].pos.y) * loc.x +  
            (tri.vertices[1].pos.x - tri.vertices[0].pos.x) * loc.y + 
            tri.vertices[0].pos.x * tri.vertices[1].pos.y -
            tri.vertices[1].pos.x * tri.vertices[0].pos.y) / 
            ((tri.vertices[0].pos.y - tri.vertices[1].pos.y) * tri.vertices[2].pos.x +  
            (tri.vertices[1].pos.x - tri.vertices[0].pos.x) * tri.vertices[2].pos.y + 
            tri.vertices[0].pos.x * tri.vertices[1].pos.y -
            tri.vertices[1].pos.x * tri.vertices[0].pos.y);
    beta = ((tri.vertices[0].pos.y - tri.vertices[2].pos.y) * loc.x +  
            (tri.vertices[2].pos.x - tri.vertices[0].pos.x) * loc.y + 
            tri.vertices[0].pos.x * tri.vertices[2].pos.y -
            tri.vertices[2].pos.x * tri.vertices[0].pos.y) / 
            ((tri.vertices[0].pos.y - tri.vertices[2].pos.y) * tri.vertices[1].pos.x +  
            (tri.vertices[2].pos.x - tri.vertices[0].pos.x) * tri.vertices[1].pos.y + 
            tri.vertices[0].pos.x * tri.vertices[2].pos.y -
            tri.vertices[2].pos.x * tri.vertices[0].pos.y);
    alpha = 1 - beta - gamma;

    result = ((alpha / tri.vertices[0].pos.z) * tri.vertices[0].normal +
            (beta / tri.vertices[1].pos.z) * tri.vertices[1].normal +
            (gamma / tri.vertices[2].pos.z) * tri.vertices[2].normal) / 
            ((alpha / tri.vertices[0].pos.z)
             + (beta / tri.vertices[1].pos.z)
              + (gamma / tri.vertices[2].pos.z));
    // cout << tri.vertices[1].pos.z << endl;
    result.normalize();
    return result;
}

/*
    problem 2.2 (10%)
    实现 Blinn-Phong 光照模型
    输入：
        col: 当前点本身贴图的颜色，三个通道均为 [0 ~ 1] 的浮点数值。
        loc: 当前点的位置，在原始的三维坐标系下存储（不是相机局部坐标系，也不是投影后的坐标系）
        Norm: 当前点的法向量，在原始的三维坐标系下存储（不是相机局部坐标系，也不是投影后的坐标系），不保证是个单位向量。
    输出:
        当前点在 Blinn-Phong 下光照的颜色。
    使用到的其他参数：
        camera.eyePos: 相机所处的位置
        light.pLightdirect[0]: 点光源的位置
        light.dLightpos[0]: 平行光源的光照方向（是向量，不是标量！）
        light.ka: 环境光强度系数
        light.kd: 漫反射强度系数
        light.ks: 高光强度系数
        light.ns: 高光强度计算时候的幂次系数。
        Color::White(): 返回白光信息
    提示：
        我们设计的光照包含如下几个部分：
            A: 点光源的高光
            B: 点光源的漫反射
            C: 平行光源的漫反射
            D: 强度为 light.ka 的环境光。
*/

Color Rendering::BlinnPhong(Color col, const Vector4 &loc, const Vector4 &Norm){
    // problem 2.2: 10%
    // 在此处填写你的代码：

    // Norm.normalize();
    // A: 点光源的高光
    Color Ida;
    Vector4 light_dira = loc - light.pLightdirect[0]; // 入射光方向
    double fatta = 1 / light_dira.Magnitude() * light_dira.Magnitude(); // 光衰减
    light_dira.normalize();
    Vector4 reflect_dira = 2 * Norm * Dot3(-light_dira, Norm) + light_dira;
    Vector4 camera_dir = camera.eyePos - loc;
    camera_dir.normalize();
    double cos_phi = Dot3(camera_dir, reflect_dira) / (camera_dir.Magnitude() * reflect_dira.Magnitude());
    cos_phi = Clamp(cos_phi, 0.0, 1.0);
    Ida = fatta * light.ks *  Color::white() * pow(cos_phi, light.ns);
    // cout << fatta<<endl;
    // cout << light_dira.Magnitude() << endl;

    // B: 点光源的漫反射
    Color Idb;
    Vector4 light_dirb = loc - light.pLightdirect[0]; // 入射光方向
    double fattb = 1 / light_dirb.Magnitude() * light_dirb.Magnitude(); // 光衰减
    light_dirb.normalize();
    double cos_thetab = Dot3(-light_dirb, Norm) / (light_dirb.Magnitude() * Norm.Magnitude());
    cos_thetab = Clamp(cos_thetab, 0.0, 1.0);
    Idb = fattb * light.kd * col * cos_thetab;

    // C: 平行光源的漫反射
    Color Idc;
    Vector4 light_dirc = light.dLightpos[0]; // 入射光方向
    light_dirc.normalize();
    double cos_thetac = Dot3(-light_dirc, Norm) / (light_dirc.Magnitude() * Norm.Magnitude());
    cos_thetac = Clamp(cos_thetac, 0.0, 1.0);
    Idc = light.kd * col * cos_thetac;

    // D: 强度为 light.ka 的环境光。
    Color Idd;
    Idd = light.ka * col;


    col = Ida + Idb + Idc + Idd;
    // cout<<col << endl;
    // col = Vector3(1, 1, 1);
    return col;
}