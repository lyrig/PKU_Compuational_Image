#ifndef MY_RENDERING_H
#define MY_RENDERING_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>
#include <array>
#include "Linalg.h"
#include <cfloat>

using std::string;
using std::vector;
using std::ifstream;


void BreakPoint(){
    std::cerr << "BP" << std::endl;
    while(1);
}
struct Vertex{
    Vector4 pos;    // 当前点位置，景深
    Vector3 normal; // 顶点法线
    Color col;      // 当前点颜色
    double diffuse; // 漫反射系数
    double u,v;     // 幕布上的投影坐标
    double rhw;     // 1 / pos.w 用于投影时候坐标的插值

    Vertex() = default;

    Vertex(const Vector4 &_pos, const Color &_col):
        pos(_pos),col(_col){}
    Vertex(const Vector4 &_pos, const Color &_col,
           const double _u, const double _v):
           pos(_pos),col(_col),u(_u),v(_v){}
    Vertex(const Vector4 &_pos, const Color &_col,
           const double _diffuse, const double _u,
           const double _v, const double _rhw):
           pos(_pos),col(_col),diffuse(_diffuse),u(_u),v(_v),rhw(_rhw){}

    Vertex operator +(const Vertex &b)const{
        return Vertex(pos + b.pos, col + b.col, diffuse + b.diffuse,
                      u + b.u, v + b.v, rhw + b.rhw);
    }
    Vertex operator -(const Vertex &b)const{
        return Vertex(pos - b.pos, col - b.col, diffuse - b.diffuse,
                      u - b.u, v - b.v, rhw - b.rhw);
    }

    void initRhw(){
        rhw = 1.0 / pos.w;
        u = u * rhw;
        v = v * rhw;
        col = col * rhw;
    }

};

inline Vertex operator *(const Vertex &a, double b){
    return Vertex(a.pos * b, a.col * b, a.diffuse * b,  a.u * b, a.v * b, a.rhw * b);
}
inline Vertex operator *(double b, const Vertex &a){
    return Vertex(a.pos * b, a.col * b, a.diffuse * b,  a.u * b, a.v * b, a.rhw * b);
}
inline Vertex operator /(const Vertex &a, double b){
    return Vertex(a.pos / b, a.col / b, a.diffuse / b,  a.u / b, a.v / b, a.rhw / b);
}

struct Triangle{
    std::array<Vertex, 3> vertices; // 三角形面片的三个顶点

    Triangle() = default;

    Triangle(const Vertex &a,const Vertex &b, const Vertex &c){
        vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;

        if (a.normal.sqrMagnitude() == 0 || b.normal.sqrMagnitude() == 0 || c.normal.sqrMagnitude() == 0)
            UpdateNorm();
    }

    void UpdateNorm(){
        Vector4 A = vertices[0].pos;
        Vector4 B = vertices[1].pos;
        Vector4 C = vertices[2].pos;
        A.divideW();
        B.divideW();
        C.divideW();
        Vector4 faceNormal = Cross(C - B, B - A);
        faceNormal.normalize();

        vertices[0].normal = faceNormal.getVector3();
        vertices[1].normal = faceNormal.getVector3();
        vertices[2].normal = faceNormal.getVector3();
    }

    double getArea() const{
        Vector4 BA = vertices[1].pos - vertices[0].pos;
        Vector4 CA = vertices[2].pos - vertices[0].pos;
        double re = BA.x * CA.y + BA.y * CA.z + BA.z * CA.x -
                    BA.x * CA.z - BA.y * CA.x - BA.z * CA.y;
        if (re < 0.0)
            return -0.5 * re;
        else
            return 0.5 * re;
    }
};

struct Trapzoid{
    /*
    该套框架会将三角形拆分成两个可以退化的梯形，
    梯形的上下边平行于屏幕的 x 轴，来简化光栅化的结果。
    */
    double t, b;
    Vertex topL, topR; // 梯形面片的上边的两个端点
    Vertex botL, botR; // 梯形面片的下边的两个端点
};

ostream& operator << (ostream& os, Trapzoid trap){
    os << "[" << trap.topL.pos << std::endl << trap.topR.pos << std::endl;
    os << trap.botL.pos << std::endl << trap.botR.pos << "]";
    return os;
}

struct Texture{
    public:
    
        int32_t _h,_w; /*贴图尺寸大小*/
        vector<vector<uint8_t>> _bit; /*原始贴图信息，h*w*/
        vector<vector<vector<uint8_t>>> Mipmap; /*Mipmap 分层贴图信息*/
        vector<int32_t> _MipmapH; /*Mipmap 分层贴图中每一层的高度*/
        vector<int32_t> _MipmapW; /*Mipmap 分层贴图中每一层的宽度*/
        
        double alpha = 1.0; /*模糊参数*/
        bool covered; /*贴图是否初始化*/

        Texture(){
            covered = false;
        }
        Texture(int width, int height):_w(width),_h(height){
            _bit.resize(_h);
            covered = true;
            for (int i = 0; i < _h; i++)
                _bit[i].resize(_w * 3);
        }
        void Clear(){
            _bit.resize(0);
            Mipmap.resize(0);
            _MipmapW.resize(0);
            _MipmapH.resize(0);
            covered = false;
        }

        ~Texture(){
            Clear();
        }

        void LoadBlackWhiteLarge(){
            //预定义黑白棋盘格 texture 1
            Clear();
            covered = true;
            _w = _h = 1024;
            _bit.resize(_h);
            for (int i = 0; i < _h; i++)
                _bit[i].resize(_w * 3);
            
            for (int i = 0; i < _h; i++)
                for (int j = 0; j < _w; j++){
                    uint8_t col = ((i ^ j) & 128 ? 255 : 0); 
                    _bit[i][j * 3 + 0] = col;
                    _bit[i][j * 3 + 1] = col;
                    _bit[i][j * 3 + 2] = col;
                }
        }

        
        void LoadBlackWhiteSmall(){
            //预定义黑白棋盘格 texture 2
            Clear();
            covered = true;
            _w = _h = 1024;
            _bit.resize(_h);
            for (int i = 0; i < _h; i++)
                _bit[i].resize(_w * 3);
            
            for (int i = 0; i < _h; i++)
                for (int j = 0; j < _w; j++){
                    uint8_t col = ((i ^ j) & 64 ? 255 : 0); 
                    _bit[i][j * 3 + 0] = col;
                    _bit[i][j * 3 + 1] = col;
                    _bit[i][j * 3 + 2] = col;
                }
        }

        void LoadPureColor(const Color& col){
            //预定义纯色 texture 
            Clear();
            uint8_t colx = (uint8_t)Clamp(col.x * 255.0, 0.0, 255.0);
            uint8_t coly = (uint8_t)Clamp(col.y * 255.0, 0.0, 255.0);
            uint8_t colz = (uint8_t)Clamp(col.z * 255.0, 0.0, 255.0);
            //std::cerr << (int)colx << ' ' << (int)coly << ' ' << (int)colz << std::endl;
            covered = true;
            _w = _h = 1024;
            _bit.resize(_h);
            for (int i = 0; i < _h; i++)
                _bit[i].resize(_w * 3);
            
            for (int i = 0; i < _h; i++)
                for (int j = 0; j < _w; j++){
                    _bit[i][j * 3 + 0] = colx;
                    _bit[i][j * 3 + 1] = coly;
                    _bit[i][j * 3 + 2] = colz;
                }
        }
        void initMipmap(){
            /* 初始化 MipMap */
            const int directX[] = {0, 0, 1, 1};
            const int directY[] = {0, 1, 1, 0};
            Mipmap.push_back(_bit);
            _MipmapW.push_back(_w);
            _MipmapH.push_back(_h);
            int Width = _w >> 1;
            int Height = _h >> 1;
            while (Width && Height){
                vector<vector<uint8_t>> _nbit = vector<vector<uint8_t>>(Height, vector<uint8_t>(Width * 3, 0.0));
                vector<vector<uint8_t>> oldBit = Mipmap.back(); // 上一层的贴图信息
                int oldWidth = _MipmapW.back();  // 上一层的长宽
                int oldHeight = _MipmapH.back(); // 上一层的长宽
                // Width, Height 是这一层的长宽。
                for (int i = 0; i < Height; i++){
                    for (int j = 0; j < Width; j++){
                        double sumR = 0.0;
                        double sumG = 0.0;
                        double sumB = 0.0;
                        int count = 0;
                        // 对于这一层的每一个像素 (i,j), 找到所有的 (ni,nj) = (2i+di[d], 2j+dj[d])，
                        // 最多四个，并且将其加权求和取到平均值，作为下一层 Mipmap 的信息。
                        for (int d = 0; d < 4; d++){
                            int ni = i * 2 + directX[d];
                            int nj = j * 2 + directY[d];
                            if (ni >= oldHeight || nj >= oldWidth) continue;
                            sumR = sumR + oldBit[ni][nj * 3 + 0];
                            sumG = sumG + oldBit[ni][nj * 3 + 1];
                            sumB = sumB + oldBit[ni][nj * 3 + 2];
                            count ++;
                        }

                        _nbit[i][j * 3 + 0] = (uint8_t)Clamp(sumR * 1.0 / count * alpha, 0.0, 255.0);
                        _nbit[i][j * 3 + 1] = (uint8_t)Clamp(sumG * 1.0 / count * alpha, 0.0, 255.0);
                        _nbit[i][j * 3 + 2] = (uint8_t)Clamp(sumB * 1.0 / count * alpha, 0.0, 255.0);
                    }
                }
                Mipmap.push_back(_nbit);
                _MipmapW.push_back(Width);
                _MipmapH.push_back(Height);
                Width >>= 1;
                Height >>= 1;
            }
        }

        Color BilinearFliterColor(double u, double v, int level);

        Color TrilinearFliterColor(double u, double v, double level){
            // 三线性插值，相比于双线性额外增加了对于 level 的插值。
            int intLevel = (int)(floor(level + 1e-9));
            double fracLevel = level - (int)(intLevel);
            if (intLevel == Mipmap.size() - 1)
                return BilinearFliterColor(u, v, intLevel);
            return BilinearFliterColor(u, v, intLevel) * (1.0 - fracLevel) +
                   BilinearFliterColor(u, v, intLevel + 1) * fracLevel;
        }
};

struct Model{
    Matrix4 WorldMat;         // 坐标变换矩阵
    vector<Vector3> vertices; // 物体顶点信息
    vector<Vector3> vertexNormal;   // 物体每个顶点的法向量信息
    vector<double> textureU;  // 物体贴图信息，定点对应的贴图 x 轴坐标 
    vector<double> textureV;  // 物体贴图信息，定点对应的贴图 y 轴坐标 
    vector<vector<vector<int> > > faces; // 物体每个面片的顶点信息 (顶点序号、顶点贴图坐标序号、顶点法线序号)
    Texture* texture;         // 物体纹理贴图信息

    Model() = default;

    void InitCube(){
        // 初始化 正方体的模型，贴图信息
        vertices.push_back(Vector3(-0.5, -0.5, -0.5));
        vertices.push_back(Vector3( 0.5, -0.5, -0.5));
        vertices.push_back(Vector3(-0.5,  0.5, -0.5));
        vertices.push_back(Vector3( 0.5,  0.5, -0.5));
        vertices.push_back(Vector3(-0.5, -0.5,  0.5));
        vertices.push_back(Vector3( 0.5, -0.5,  0.5));
        vertices.push_back(Vector3(-0.5,  0.5,  0.5));
        vertices.push_back(Vector3( 0.5,  0.5,  0.5));

        vertexNormal.push_back(Vector3( 0.0,  0.0, -1.0));
        vertexNormal.push_back(Vector3( 0.0,  0.0,  1.0));
        vertexNormal.push_back(Vector3(-1.0,  0.0,  0.0));
        vertexNormal.push_back(Vector3( 1.0,  0.0,  0.0));
        vertexNormal.push_back(Vector3( 0.0,  1.0,  0.0));
        vertexNormal.push_back(Vector3( 0.0, -1.0,  0.0));

        textureU.push_back(0.0);
        textureV.push_back(0.0);
        textureU.push_back(0.0);
        textureV.push_back(1.0);
        textureU.push_back(1.0);
        textureV.push_back(0.0);
        textureU.push_back(1.0);
        textureV.push_back(1.0);

        faces.push_back(
            vector<vector<int> >({
                vector<int>({0, 1, 0}),
                vector<int>({1, 3, 0}),
                vector<int>({2, 0, 0}),
            })
        );
        faces.push_back(
            vector<vector<int> >({
                vector<int>({3, 2, 0}),
                vector<int>({2, 0, 0}),
                vector<int>({1, 3, 0}),
            })
        );
        // near
        faces.push_back(
            vector<vector<int> >({
                vector<int>({4, 3, 1}),
                vector<int>({6, 2, 1}),
                vector<int>({5, 1, 1}),
            })
        );
        faces.push_back(
            vector<vector<int> >({
                vector<int>({7, 0, 1}),
                vector<int>({5, 1, 1}),
                vector<int>({6, 2, 1}),
            })
        );
        // far
        faces.push_back(
            vector<vector<int> >({
                vector<int>({6, 0, 2}),
                vector<int>({4, 1, 2}),
                vector<int>({2, 2, 2}),
            })
        );
        faces.push_back(
            vector<vector<int> >({
                vector<int>({0, 3, 2}),
                vector<int>({2, 2, 2}),
                vector<int>({4, 1, 2}),
            })
        );
        // left
        faces.push_back(
            vector<vector<int> >({
                vector<int>({1, 1, 3}),
                vector<int>({5, 3, 3}),
                vector<int>({3, 0, 3}),
            })
        );
        faces.push_back(
            vector<vector<int> >({
                vector<int>({7, 2, 3}),
                vector<int>({3, 0, 3}),
                vector<int>({5, 3, 3}),
            })
        );
        //right
        faces.push_back(
            vector<vector<int> >({
                vector<int>({2, 1, 4}),
                vector<int>({3, 3, 4}),
                vector<int>({6, 0, 4}),
            })
        );
        faces.push_back(
            vector<vector<int> >({
                vector<int>({7, 2, 4}),
                vector<int>({6, 0, 4}),
                vector<int>({3, 3, 4}),
            })
        );
        //top
        faces.push_back(
            vector<vector<int> >({
                vector<int>({0, 0, 5}),
                vector<int>({4, 1, 5}),
                vector<int>({1, 2, 5}),
            })
        );
        faces.push_back(
            vector<vector<int> >({
                vector<int>({5, 3, 5}),
                vector<int>({1, 2, 5}),
                vector<int>({4, 1, 5}),
            })
        );

        WorldMat.SetDiag(0.0);
        texture = NULL;
        //bot
    }

    void InitCirc(){
        // 初始化圆形的模型，贴图信息
        WorldMat.SetDiag(0.0);
        
        const double R = 0.6f;
        const Vector3 O(0.0f, 0.0f, 0.0f);
        const int Dpitch = 9; // 俯仰角
        const int Dyaw = 10;  // 偏航角
        
        int count = 0;
        vector<vector<int> > idx;
        for (int i = -90; i <= 90; i += Dpitch){
            vector<int> temp;
            double pitch = i / 180.0 * PI;
            for (int j = 0; j <= 360; j += Dyaw){
                double yaw = j / 180.0 * PI;
                Vector3 pos(cos(pitch) * cos(yaw), sin(pitch), cos(pitch) * sin(yaw));
                textureU.push_back((i + 90.0) / 180.0);
                textureV.push_back(j / 360.0);
                vertices.push_back(pos * R);
                Vector3 normal = pos;
                normal.normalize();
                vertexNormal.push_back(normal);

                temp.push_back(count);
                count++;
            }
            idx.push_back(temp);
        }

        auto Maketri = [](int x, int y, int z) -> vector<vector<int> >{
            vector<int> vx({x, x, x});
            vector<int> vy({y, y, y});
            vector<int> vz({z, z, z});
            return vector<vector<int> > ({vx, vy, vz});
        };

        for (int i = 0; i + 1 < idx.size(); i++)
            for (int j = 0; j + 1 < idx[0].size(); j++){
                if (i != 0)
                    faces.push_back(Maketri(idx[i][j], idx[i][j + 1], idx[i + 1][j]));
                if (i + 2 != idx.size())
                    faces.push_back(Maketri(idx[i][j + 1], idx[i + 1][j + 1], idx[i + 1][j]));
            }

        texture = NULL;
    }
};

class LightSource{
    public:
        // 默认是白光
        std::vector<Vector4> pLightdirect; // 平行光源
        std::vector<Vector4> dLightpos;    // 点光源
        double ka = 0.2; // 环境光强度系数
        double kd = 0.45; // 漫反射强度系数
        double ks = 0.8; // 高光强度系数
        int ns = 10; // 高光强度计算时候的幂次系数


        LightSource() = default;
        void ParallelLight(Vector4 direct){
            direct.normalize();
            pLightdirect.push_back(direct);
        }

        void DotLight(Vector4 pos){
            dLightpos.push_back(pos);
        }
        
        void Clear(){
            pLightdirect.resize(0);
            dLightpos.resize(0);
        }
};


class Camera{
    public:
        Vector4 eyePos;      // 相机位置
        Vector4 directFront; // 相机视角下，向前的方向
        Vector4 directUp;    // 相机视角下，向上的方向
        Vector4 directX;     // 相机视角下，向右的方向

        double zNear = 0.1, zFar = 500; // 相机近点，远点
        double FofV = 90;               // 相机视角大小
        double ratio = 4.0 / 3.0;       // 相机画布长宽比

    Camera():
        eyePos(Vector4(0.0, 0.0, 0.0, 1.0)),
        directFront(Vector4(0.0, 0.0, 1.0, 0.0)),
        directUp(Vector4(0.0, 1.0, 0.0, 0.0)),
        directX(Vector4(1.0, 0.0, 0.0, 0.0)){}
    Camera(const Vector4& _eyePos, const Vector4& _directFront, const Vector4& _directUp):
        eyePos(_eyePos), directFront(_directFront), directUp(_directUp){
            directFront.normalize();
            directUp.normalize();
            directX = Cross(directFront, directUp);
            directX.normalize();
        }
    public:
        void setCamera(const Vector4& _eyePos,
                        const Vector4& _directFront = Vector4(0.0, 0.0, 1.0, 0.0),
                        const Vector4& _directUp = Vector4(0.0, 1.0, 0.0, 0.0)){
                            eyePos = _eyePos;
                            directUp = _directUp;
                            directFront = _directFront;
                            directFront.normalize();
                            directUp.normalize();
                            directX = Cross(directUp, directFront);
                            directX.normalize();
                        }

        void KeyboardUpd(const string &opts){

            /* 理论上可以使用键盘控制相机视角 */
            /* 可能仅限于 Windows */
            int len = opts.length();
            for (int i = 0; i < len; i++){
                char ch = opts[i];
                double deltaT = 0.02;
                if (ch >= 'a' && ch <= 'z'){
                    deltaT = 0.1;
                    ch = ch - 'a' + 'A';
                }
                if (ch =='W')       eyePos = eyePos + deltaT * directUp;
                else if (ch == 'S') eyePos = eyePos - deltaT * directUp;
                else if (ch == 'A') eyePos = eyePos - deltaT * directX;
                else if (ch == 'D') eyePos = eyePos + deltaT * directX;
                else if (ch == 'Q') eyePos = eyePos - deltaT * directFront;
                else if (ch == 'E') eyePos = eyePos + deltaT * directFront;
                else if (ch >= 'I' && ch <= 'L'){
                    Matrix4 tr;
                    if (ch == 'I') tr.SetRotate(directX, deltaT * 0.3);
                    if (ch == 'K') tr.SetRotate(directX, -deltaT * 0.3);
                    if (ch == 'J') tr.SetRotate(directUp, deltaT * 0.3);
                    if (ch == 'L') tr.SetRotate(directUp, -deltaT * 0.3);
                    directUp = tr * directUp;
                    directX = tr * directX;
                    directFront = tr * directFront;
                }
            }
        }

        Matrix4 cameraViewTrans();

        Matrix4 cameraProject();

        Vector4 transProject(Vector4 p);

        Vector4 InvtransProject(Vector4 p);

};


inline ostream& operator << (ostream& os, const Camera& cam) {
	cout << "eye position: " << cam.eyePos << endl
		<< "gaze at: " << cam.directFront << endl
		<< "view up: " << cam.directUp << endl
		<< "cameraX: " << cam.directX;
	return os;
}

class Rendering{
    public:
        uint8_t** framebuf; // 像素信息，存储方式同 Texture 中 _bit 的存储方式。最终输出存储的位置
        double** depthbuf;  // 深度信息，存储方式同 Texture 中 _bit 的存储方式。
        double** colorbuf;  // 颜色信息，存储方式同 Texture 中 _bit 的存储方式。使用 double 存储中间结果。
        int _w,_h;          // 高度，宽度，存储方式同 Texture 中 _bit 的存储方式。
        int renderMode = 2; // 渲染模式，存储方式同 Texture 中 _bit 的存储方式。

        /*
        0: 只绘制三角面片框架
        1: 不使用 mipmap 进行渲染
        2: 使用 双线性插值进行渲染
        3: 使用 三线性插值进行渲染
        */

        Camera camera;              // 相机
        LightSource light;          // 光源
        vector<Triangle> rendered;  // 需要渲染的三角面片
    
        Rendering(int Width, int Height, uint8_t* screen_fb){
            _w = Width;
            _h = Height;

            framebuf = new uint8_t*[_h];
            for (int i = 0; i < _h; i++)
                framebuf[i] = screen_fb + 4 * _w * i;

            depthbuf = new double*[2 * _h];
            double* ptr = new double[4 * _w * _h];
            for (int i = 0; i < 2 * _h; i++)
                depthbuf[i] = ptr + 2 * i * _w;
            
            colorbuf = new double*[2 * _h];
            ptr = new double[12 * _w * _h];
            for (int i = 0; i < 2 * _h; i++)
                colorbuf[i] = ptr + 6 * i * _w;

            clearDepth();

            // 初始化相机信息
            camera.setCamera(Vector4(3.8, -0.4, 2.0, 1.0), Vector4(-0.6, 0.8, 0.0, 0.0), Vector4(0.8, 0.6, 0.0, 0.0));
            
            // 初始化光源信息
		    light.Clear();
            light.ParallelLight(Vector4(-1.0, 1.0, 1.0, 0.0));
            light.DotLight(Vector4(6.0, 10.0, 2.5, 0.0));
        }

        void refresh(const Color &bgColor){
            // 刷新幕布
            clearDepth();
            clearFrame(bgColor);
        }

        void drawScreen(){
            // 将幕布信息转换到最终输出屏幕上
            for (int i = 0; i < _h; i++)
                for (int j = 0; j < _w; j++){
                    uint8_t* ptr = framebuf[i] + j * 4;
                    double* ptr1 = colorbuf[2 * i + 0] + (2 * j + 0) * 3;
                    double* ptr2 = colorbuf[2 * i + 1] + (2 * j + 0) * 3;
                    *(ptr + 0) = (uint8_t)((*(ptr1 + 0) + *(ptr1 + 3) + *(ptr2 + 0) + *(ptr2 + 3)) * 255.0 / 4.0);
                    *(ptr + 1) = (uint8_t)((*(ptr1 + 1) + *(ptr1 + 4) + *(ptr2 + 1) + *(ptr2 + 4)) * 255.0 / 4.0);
                    *(ptr + 2) = (uint8_t)((*(ptr1 + 2) + *(ptr1 + 5) + *(ptr2 + 2) + *(ptr2 + 5)) * 255.0 / 4.0);
                }
        }

        int& switchMode(){
            return renderMode;
        }

        void drawScene(const vector<Model> &scene){
            for (const auto& mod: scene)
                drawModel(mod);
        }

        void drawModel(const Model& model){
            /*绘制 Model, 通过逐个绘制三角面片实现*/
            int faceSize = model.faces.size();

            rendered.resize(0);

            Matrix4 worldMat = model.WorldMat;
            Matrix4 invTransWorldMat = worldMat.Inverted().Transposed();
            for (int i = 0; i < faceSize; i++){

                // 进行模型内部的变换 worldMat，计算模型的三维平面下的坐标。
                Triangle tri;
                tri = findTriangle(model, i);
                for (int i = 0; i < 3; i++)
                {
			        tri.vertices[i].pos = worldMat * tri.vertices[i].pos; 
                    tri.vertices[i].normal = invTransWorldMat * tri.vertices[i].normal;
                }
                // 计算投影坐标
                // tri.UpdateNorm();
                for (int i = 0; i < 3; i++)
			        tri.vertices[i].pos = camera.transProject(tri.vertices[i].pos);
                
                rendered.push_back(tri);
            }

            int renderSize = rendered.size();
            int count = 0;

            for (auto &tri : rendered){
                /* 逐个渲染面片 */
                for (int i = 0; i < 3; i++){
                    tri.vertices[i].pos.ViewportTransform(2 * _w, 2 * _h);
                    tri.vertices[i].initRhw();
                }
                
                if (renderMode == 0){
                    /* 只绘制面片 */
                    drawWireFrame(tri);
                }
                else{
                    double mipLevel = 0.0;
                    /* 计算插值使用的 mipmap 层数*/
                    Triangle triTexture;
                    for (int j = 0; j < 3; j++){
                        double w = 1.0 / tri.vertices[j].rhw;
                        triTexture.vertices[j].pos =
                            Vector4(tri.vertices[j].u * w, tri.vertices[j].v * w, 0.0, 0.0);
                    }
                    mipLevel = calMipLevel(tri.getArea(), triTexture.getArea() * (model.texture)->_h * (model.texture)->_w, model);
                    /* 将三角形分割成若干个可退化的梯形*/
                    vector<Trapzoid> traps = splitTriangle(tri);
                    for (auto trap: traps)
                        drawTrap(trap, tri, model, mipLevel);
                }
                
            }
        }

        int calMipLevel(const double &arTri, const double &arTex, const Model& model){
            // 计算 Mipmap 层数
            double ratio = sqrt(arTex / arTri);
            double mipLevel = Clamp(log(ratio) / log(2), 0.0, (double)((model.texture)->Mipmap.size() - 1));
            return mipLevel;
        }

        Triangle findTriangle(const Model &mod, int idx){
            // 寻找模型中的三角面片，以及其对应的纹路信息。
            Triangle tri;
            for (int i = 0; i < 3; i++){
                tri.vertices[i].pos = Vector4(
                    mod.vertices[mod.faces[idx][i][0]].x,
                    mod.vertices[mod.faces[idx][i][0]].y,
                    mod.vertices[mod.faces[idx][i][0]].z,
                    1.0
                );
                tri.vertices[i].u = mod.textureU[mod.faces[idx][i][1]];
                tri.vertices[i].v = mod.textureV[mod.faces[idx][i][1]];

                if (!mod.vertexNormal.empty())
                {
                    tri.vertices[i].normal = mod.vertexNormal[mod.faces[idx][i][2]];
                }
            }
            
            if (mod.vertexNormal.empty())
                tri.UpdateNorm();
            return tri;
        }

        void drawWireFrame(Triangle tri, const Color color = Color::black()){
            /*绘制三角面片线框*/
            Vector4 a = tri.vertices[0].pos; 
            Vector4 b = tri.vertices[1].pos;
            Vector4 c = tri.vertices[2].pos;
            drawLine(a.x + 0.5, a.y + 0.5, b.x + 0.5, b.y + 0.5, color);
            drawLine(b.x + 0.5, b.y + 0.5, c.x + 0.5, c.y + 0.5, color);
            drawLine(c.x + 0.5, c.y + 0.5, a.x + 0.5, a.y + 0.5, color);
        }
        void drawLine(const int sx, const int sy, const int tx,const int ty,const Color color){
            /*绘制直线*/
            if (sx == tx && sy == ty)
                drawPixel(sx, sy, color);
            else if (sx == tx){
                int dy = (sy < ty ? 1 : -1);
                for (int y = sy; y != ty; y += dy)
                    drawPixel(sx, y, color);
            }
            else if (sy == ty){
                int dx = (sx < tx ? 1 : -1);
                for (int x = sx; x != tx; x += dx)
                    drawPixel(x, sy, color);
            }
            else{
                int Sx = sx, Sy = sy, Tx = tx, Ty = ty;
                bool reversed = (abs(Sx - Tx) < abs(Sy - Ty));
                if (reversed){
                    std::swap(Sx, Sy);
                    std::swap(Tx, Ty);
                }
                if (Sx > Tx){
                    std::swap(Sx, Tx);
                    std::swap(Sy, Ty);
                }
                int x = Sx, y = Sy;
                int dx = Tx - Sx, dy = abs(Ty - Sy);
                int incY = (Ty > Sy ? 1 : -1);
                int p = 2 * dy - dx;
                for (;x < Tx; ++x){
                    if (reversed)
                        drawPixel(y, x, color);
                    else
                        drawPixel(x, y, color);
                    if (p > 0){
                        y += incY;
                        p -= 2 * dx;
                    }
                    p += 2 * dy;
                }
            }
        }

        bool Compare(const Vector4 &a, const Vector4 &b){
            if (a.y == b.y) return a.x < b.x;
            return a.y < b.y;
        }
        vector<Trapzoid> splitTriangle(Triangle tri){
            /*将三角面片拆分成两个可以退化的梯形面片*/
            Vertex& a = tri.vertices[0];
            Vertex& b = tri.vertices[1];
            Vertex& c = tri.vertices[2];
            if (!Compare(a.pos, b.pos)) std::swap(a, b);
            if (!Compare(a.pos, c.pos)) std::swap(a, c);
            if (!Compare(b.pos, c.pos)) std::swap(b, c);
            if (a.pos.x == b.pos.x && a.pos.x == c.pos.x) return vector<Trapzoid>();
            if (a.pos.y == b.pos.y && a.pos.y == c.pos.y) return vector<Trapzoid>();
            if (a.pos.y == b.pos.y)
                return vector<Trapzoid>{Trapzoid{a.pos.y, c.pos.y, a, b, c, c}};
            if (b.pos.y == c.pos.y)
                return vector<Trapzoid>{Trapzoid{a.pos.y, c.pos.y, a, a, b, c}};
            
            Trapzoid trap1, trap2;
            trap1.t = a.pos.y; trap1.b = b.pos.y;
            trap2.t = b.pos.y; trap2.b = c.pos.y;
            trap1.topL = trap1.topR = a;
            trap2.botL = trap2.botR = c;

            double k = (b.pos.y - a.pos.y) / (c.pos.y - a.pos.y);
            Vertex d = a * (1.0 - k) + c * k;
            if (b.pos.x > d.pos.x) std::swap(b, d);
            trap1.botL = trap2.topL = b;
            trap1.botR = trap2.topR = d;
            return vector<Trapzoid>{trap1, trap2};
        }

        void drawTrap(Trapzoid trap, const Triangle& tri, const Model& model, const double mipLevel = 0.0){
            /*绘制梯形，采用扫描线的方式*/
            int t = (int)(trap.t + 0.5);
            int b = (int)(trap.b + 0.5);
            for (int y = std::max(0, t); y < b && y < 2 * _h; ++y){
                double rate = Clamp((y - trap.t) / (trap.b - trap.t), 0.0, 1.0);
                Vertex verL = trap.topL * (1 - rate) + trap.botL * rate;
                Vertex verR = trap.topR * (1 - rate) + trap.botR * rate;
                drawScanline(y, verL, verR, tri, model, mipLevel);
            }
        }

        int tempcount = 0;
        void drawScanline(int y, Vertex verL, Vertex verR, const Triangle& tri, const Model& model, const double Miplevel = 0.0){

            /*绘制梯形上的某个扫描线信息(固定 y 轴)*/
            int xl = (int)(verL.pos.x + 0.5);
            int xr = (int)(verR.pos.x + 0.5);
            for (int x = std::max(0, xl); x < xr && x < 2 * _w; ++x){
                double rate = Clamp((x - verL.pos.x) / (verR.pos.x - verL.pos.x), 0.0, 1.0);
                Vertex vertex = verL * (1 - rate) + verR * rate;
                double w = 1.0 / vertex.rhw;
                Color col;
                if (renderMode == 1){
                    /*从 texture 直接采样*/
                    int u = (int)((double)((model.texture)->_w - 1) * vertex.u * w + 0.5);
                    int v = (int)((double)((model.texture)->_h - 1) * vertex.v * w + 0.5);
                    u = Clamp(u, 0, (model.texture)->_w - 1);
                    v = Clamp(v, 0, (model.texture)->_h - 1);
                    col.x = (model.texture)->_bit[u][v * 3 + 0] / 255.0;
                    col.y = (model.texture)->_bit[u][v * 3 + 1] / 255.0;
                    col.z = (model.texture)->_bit[u][v * 3 + 2] / 255.0;
                }
                if (renderMode == 2){
                    /*从 texture 的 mipmap 进行双线性采样*/
                    col = (model.texture)->BilinearFliterColor(vertex.u * w, vertex.v * w, int(Miplevel));
                }
                if (renderMode == 3){
                    /*从 texture 的 mipmap 上进行三线性采样*/
                    col = (model.texture)->TrilinearFliterColor(vertex.u * w, vertex.v * w, Miplevel);
                }
                // cout << col << endl;
                Vector4 loc(x, y, vertex.pos.z, w);
                loc.InvViewportTransform(2 * _w, 2 * _h);

                Triangle tempTri = tri;
                tempTri.vertices[0].pos.InvViewportTransform(2 * _w, 2 * _h);
                tempTri.vertices[1].pos.InvViewportTransform(2 * _w, 2 * _h);
                tempTri.vertices[2].pos.InvViewportTransform(2 * _w, 2 * _h);

                Vector3 normal = InterpolateNormal(loc, tempTri);
                normal.normalize();

                loc = camera.InvtransProject(loc);
                col = BlinnPhong(col, loc, normal); 
                // 根据 blinn-phong 模型进行着色
                col.x = Clamp(col.x, 0.0, 1.0);
                col.y = Clamp(col.y, 0.0, 1.0);
                col.z = Clamp(col.z, 0.0, 1.0);
                drawPixel(x, y, col, vertex.pos.z);
            }
        }

        Vector3 InterpolateNormal(const Vector4 &loc, const Triangle &tri);

        Color BlinnPhong(Color col, const Vector4 &loc, const Vector4 &Norm);
        
        void clearDepth(){
            // 清除深度信息
            for (int i = 0; i < 2 * _h; i++)
                for (int j = 0; j < 2 * _w; j++)
                    depthbuf[i][j] = 1e100;
        }
        void clearFrame(const Color& bgColor){
            // 清除幕布信息
            for (int i = 0; i < 2 * _h; i++)
                for (int j = 0; j < 2 * _w; j++)
                    drawPixel(j, i, bgColor);
        }
        void drawPixel(const int y, const int x, const Color &color, const double depth){
            /*绘制像素*/
            if (x < 0 || y < 0 || x >= 2 * _h || y >= 2 * _w)
                return;
            if (depth < depthbuf[x][y]){
                depthbuf[x][y] = depth;
                double* ptr = colorbuf[x] + 3 * y;
                *(ptr + 0) = color.z;
                *(ptr + 1) = color.y;
                *(ptr + 2) = color.x;
            }
        }
        void drawPixel(const int y, const int x, const Color &color){
            /*绘制像素*/
            if (x < 0 || y < 0 || x >= 2 * _h || y >= 2 * _w)
                return;
            double* ptr = colorbuf[x] + 3 * y;
            *(ptr + 0) = color.z;
            *(ptr + 1) = color.y;
            *(ptr + 2) = color.x;
        }
};

#endif