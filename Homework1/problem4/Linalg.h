#ifndef MY_LINALG_H
#define MY_LINALG_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <vector>
#include <ctime>

using std::ostream;
using std::vector;
using std::cout;
using std::endl;

const double PI = acos(-1);

class Vector3; // 三维向量
class Vector4; // 四维向量
class Matrix4x4; // 4*4 矩阵

typedef Vector3 Color; // R G B
typedef Vector4 Quaternion;
typedef Vector3 Normal3;// 定义法线为向量3D
typedef Vector3 Point3; // 点的位置

class Vector3{
    /*
    Note:
    x, y, z: 三维空间内的坐标点 / 颜色的 RGB 分量
    支持包括叉乘，点乘，数字乘在内的基础向量操作，
    */
    public:
        double x,y,z;
    
        Vector3(): x(0.0), y(0.0), z(0.0){}
        Vector3(double _x, double _y, double _z): x(_x), y(_y), z(_z){}
    
    public:
        double operator[](int i)const{
            assert(i >= 0 && i <= 2);
            if(i==0){
                return x;
            }
            else if(i == 1){
                return y;
            }
            else if(i == 2){
                return z;
            }
        }

        double & operator[](int i){
            assert(i >=0 && i <= 2);
            if(i==0){
                return x;
            }
            else if(i == 1){
                return y;
            }
            else if(i == 2){
                return z;
            }
        }

        Vector3 operator +(const Vector3 &a)const{
            return Vector3(
                x + a.x,
                y + a.y,
                z + a.z
            );
        }
        Vector3 operator -(const Vector3 &a)const{
            return Vector3(
                x - a.x,
                y - a.y,
                z - a.z
            );
        }

        Vector3 operator +=(const Vector3 &a){
            x += a.x;
            y += a.y;
            z += a.z;
            return *this;
        }
        Vector3 operator -=(const Vector3 &a){
            x -= a.x;
            y -= a.y;
            z -= a.z;
            return *this;
        }
        Vector3 operator *=(const double &a){
            x *= a;
            y *= a;
            z *= a;
            return *this;
        }
        Vector3 operator /=(const double &a){
            if (a){
                double ra = 1.0 / a;
                x *= ra;
                y *= ra;
                z *= ra;
            }
            return *this;
        }

        double sqrMagnitude() const{
            // 计算模长平方
            return x * x + y * y + z * z;
        }
        double Magnitude() const{
            // 计算模长
            return sqrt(x * x + y * y + z * z);
        }
        
        void normalize(){
            // 向量标准化
            double a = Magnitude();
            if (a){
                double ra = 1.0 / a;
                x *= ra;
                y *= ra;
                z *= ra;
            }
        }

        static Vector3 black(){
            // 预先定义的黑色 RGB 向量
            return Vector3(0.0, 0.0, 0.0);
        }
        static Vector3 white(){
            // 预先定义的白色 RGB 向量
            return Vector3(1.0, 1.0, 1.0);
        }
        static Vector3 blue(){
            // 预先定义的蓝色 RGB 向量
            return Vector3(0.0, 0.0, 1.0);
        }

};

inline Vector3 operator *(const Vector3 &a,const double &b){
    return Vector3(
        a.x * b,
        a.y * b,
        a.z * b
    );
}
inline Vector3 operator *(const double &b,const Vector3 &a){
    return Vector3(
        a.x * b,
        a.y * b,
        a.z * b
    );
}
inline Vector3 operator /(const Vector3 &a,const double &b){
    double rb = 1.0;
    if (b) rb = rb / b;
    return Vector3(
        a.x * rb,
        a.y * rb,
        a.z * rb
    );
}

inline ostream& operator <<(ostream &os, const Vector3 &a){
    os << "(" << a.x << ',' << a.y << ',' << a.z << ')';
    return os;
}

inline double Dot(const Vector3 &a,const Vector3 &b){
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// 线性插值
Vector3 Lerp(float t, const Vector3& p1, const Vector3& p2){
    return t * p1 + (1-t)*p2;
}

inline Vector3 Cross(const Vector3 &a,const Vector3 &b){
    return Vector3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

Vector3 Floor(const Vector3 &a){
    return Vector3(std::floor(a.x), std::floor(a.y), std::floor(a.z));
}

class Vector4{

    /*
    Note:
    x, y, z: 三维空间内的坐标点
    w: 三维的投影变换下，该点的深度。用于透视投影下的插值。
    */

    public:

        double x,y,z,w;
    
        Vector4(): x(0.0), y(0.0), z(0.0), w(0.0){}
        Vector4(double _x, double _y, double _z,double _w): x(_x), y(_y), z(_z), w(_w){}        
        Vector4(const Vector3 v): x(v.x), y(v.y), z(v.z), w(1.0){}
        /*explicit operator Vector3(){
            return Vector3(x, y, z);
        }*/
    
    public:

        Vector4 operator -()const{
            return Vector4(-x, -y, -z, -w);
        }
        Vector4 operator +(const Vector4 &a)const{
            return Vector4(
                x + a.x,
                y + a.y,
                z + a.z,
                w + a.w
            );
        }
        Vector4 operator -(const Vector4 &a)const{
            return Vector4(
                x - a.x,
                y - a.y,
                z - a.z,
                w - a.w
            );
        }
        Vector4 operator +=(const Vector4 &a){
            x += a.x;
            y += a.y;
            z += a.z;
            w += a.w;
            return *this;
        }
        Vector4 operator -=(const Vector4 &a){
            x -= a.x;
            y -= a.y;
            z -= a.z;
            w -= a.w;
            return *this;
        }
        Vector4 operator *=(const double &a){
            x *= a;
            y *= a;
            z *= a;
            w *= a;
            return *this;
        }
        Vector4 operator /=(const double &a){
            if (a){
                double ra = 1.0 / a;
                x *= ra;
                y *= ra;
                z *= ra;
                w *= ra;
            }
            return *this;
        }
        double operator *(const Vector4 &a){
            return a.x * x + a.y * y + a.z * z + a.w * w;
        } // Dot product

        double sqrMagnitude() const{
            // 向量模长平方
            return x * x + y * y + z * z;
        }
        double Magnitude() const{
            // 向量模长
            return sqrt(x * x + y * y + z * z);
        }

        Vector3 getVector3() const{
            return Vector3(x, y, z);
        }
        
        void normalize(){
            // 向量标准化
            double a = Magnitude();
            if (a){
                double ra = 1.0 / a;
                x *= ra;
                y *= ra;
                z *= ra;
            }
        }
        
        void divideW(){
            // 根据 w 分量处理坐标，便于进行投影的插值。
            if (w != 0 && w != 1){
                double rw = 1.0 / w;
                x *= rw;
                y *= rw;
                z *= rw;
                w = 1;
            }
        }

        void ViewportTransform(int width, int height){
            /*
            输入：画布的大小 width, height
            操作：根据当前点在世界坐标系中的坐标(x,y,z)，以及景深(w)，计算其在屏幕上的投影坐标(x', y') 以及投影深度 (z')。
            */
            double rhw = 1.0 / w;
            x = (x * rhw + 1.0) * width * 0.5;
            y = (-y * rhw + 1.0) * height * 0.5;
            z = z * rhw;
        }

        void InvViewportTransform(int width, int height){
            /*
            输入：画布的大小 width, height操作：
            操作：根据当前点屏幕上的投影坐标(x', y')，投影深度 (z'），以及景深(w)，计算当前点在世界坐标系中的坐标(x,y,z)。
            */
            x = (2 * x / width - 1.0) * w;
            y = -(2 * y / height - 1.0) * w;
            z = z * w;
        }
};

inline Vector4 operator *(const Vector4 &a,const double &b){
    return Vector4(
        a.x * b,
        a.y * b,
        a.z * b,
        a.w * b
    );
}
inline Vector4 operator *(const double &b,const Vector4 &a){
    return Vector4(
        a.x * b,
        a.y * b,
        a.z * b,
        a.w * b
    );
}
inline Vector4 operator /(const Vector4 &a,const double &b){
    double rb = 1.0;
    if (b) rb = rb / b;
    return Vector4(
        a.x * rb,
        a.y * rb,
        a.z * rb,
        a.w * rb
    );
}

Vector3 V4to3(const Vector4 &a){
    return Vector3(
                a.x,
                a.y,
                a.z
            );
}

inline ostream& operator <<(ostream &os, const Vector4 &a){
    os << "(" << a.x << ',' << a.y << ',' << a.z << ',' << a.w << ')';
    return os;
}

inline double Dot(const Vector4 &a,const Vector4 &b){
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline Vector4 Cross(const Vector4 &a,const Vector4 &b){
    return Vector4(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
        0.0
    );
}


class Matrix4{
    public:
        double v[4][4];
        Matrix4(){
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    v[i][j] = 0.0;
        }
        Matrix4(double diag){
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    if (i == j)
                        v[i][j] = diag;
                    else
                        v[i][j] = 0;
        }
        Matrix4(const std::initializer_list<double>& il) {
            if (il.size() != 16) {
                throw "Matrix need 16 numbers.";
            }
            auto it = il.begin();
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    v[i][j] = *it++;
        }
    
    public:
        Matrix4 operator *(const Matrix4 &a)const{
            Matrix4 res;
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    res.v[i][j] = v[i][0] * a.v[0][j] +
                                v[i][1] * a.v[1][j] +
                                v[i][2] * a.v[2][j] +
                                v[i][3] * a.v[3][j];
            return res;
        }
        Vector4 operator *(const Vector4 &a)const{
            Vector4 res;
            res = res + Vector4(v[0][0], v[1][0], v[2][0], v[3][0]) * a.x;
            res = res + Vector4(v[0][1], v[1][1], v[2][1], v[3][1]) * a.y;
            res = res + Vector4(v[0][2], v[1][2], v[2][2], v[3][2]) * a.z;
            res = res + Vector4(v[0][3], v[1][3], v[2][3], v[3][3]) * a.w;
            return res;
        }

        Vector3 operator *(const Vector3 &a)const{
            Vector4 res;
            res = res + Vector4(v[0][0], v[1][0], v[2][0], v[3][0]) * a.x;
            res = res + Vector4(v[0][1], v[1][1], v[2][1], v[3][1]) * a.y;
            res = res + Vector4(v[0][2], v[1][2], v[2][2], v[3][2]) * a.z;
            res = res + Vector4(v[0][3], v[1][3], v[2][3], v[3][3]) * 1;
            return Vector3(res.x, res.y, res.z);
        }

        void SetDiag(double diag){
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    if (i == j)
                        v[i][j] = diag;
                    else
                        v[i][j] = 0.0;
        }
        void SetTranslate(double x, double y, double z){
            SetDiag(1.0);
            v[0][3] = x;
            v[1][3] = y;
            v[2][3] = z;
            return;
        }
        void SetScale(double x, double y, double z){
            v[0][0] = x;
            v[1][1] = y;
            v[2][2] = z;
            v[3][3] = 1;
            return;
        }
        void SetRotate(double X, double Y, double Z, double theta){
            double qsin = sin(theta * 0.5);
            double qcos = cos(theta * 0.5);
            Vector3 di(X, Y, Z);
            di.normalize();
            double w = -1;
            double x = -1;
            double y = -1;
            double z = -1;

            w = qcos;
            x = qsin * di.x;
            y = qsin * di.y;
            z = qsin * di.z;
            
            SetDiag(0.0);
            v[0][0] = 1 - 2 * y * y - 2 * z * z;
            v[1][1] = 1 - 2 * z * z - 2 * x * x;
            v[2][2] = 1 - 2 * x * x - 2 * y * y;
            v[1][0] = 2 * x * y - 2 * w * z;
            v[2][0] = 2 * x * z + 2 * w * y;
            v[2][1] = 2 * y * z - 2 * w * x;
            v[0][1] = 2 * x * y + 2 * w * z;
            v[0][2] = 2 * x * z - 2 * w * y;
            v[1][2] = 2 * y * z + 2 * w * x;
            v[3][3] = 1;
        }
        void SetRotate(const Vector3 &v, double theta){
            SetRotate(v.x, v.y, v.z, theta);
        }
        void SetRotate(const Vector4 &v, double theta){
            SetRotate(v.x, v.y, v.z, theta);
        }

        Matrix4 Inverted() const
        {
            double temp[4][8];
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++){
                    temp[i][j] = v[i][j];
                    if (i == j) temp[i][j + 4] = 1.0;
                    else temp[i][j + 4] = 0.0;
                }
            for (int i = 0; i < 4; i++){
                int p = -1;
                for (int j = i; j < 4; j++)
                    if (std::fabs(temp[j][i]) > 1e-5)
                        p = j;
                if (p == -1){
                    throw "Matrix can not be inverted";
                    break;
                }
                if (p != i){
                    for (int j = 0; j < 8; j++)
                        std::swap(temp[i][j], temp[p][j]);
                }
                for (int j = 7; j >= i; j--)
                    temp[i][j] /= temp[i][i];
                for (int j = 0; j < 4; j++){
                    if (j == i) continue;
                    for (int k = 7; k >= i; k--)
                        temp[j][k] -= temp[j][i] * temp[i][k];
                }
            }

            Matrix4 re;
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    re.v[i][j] = temp[i][j + 4];
            return re;
        }

        Matrix4 Transposed() const
        {
            Matrix4 res;
            
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    res.v[i][j] = v[j][i];
                    
            return res;
        }
};

ostream& operator <<(ostream &os, Matrix4 a){
    os << "[" << endl;
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            os << a.v[i][j];
            if (j + 1 == 4)
                os << endl;
            else os << ',';
        }
    }
    os << ']';
    return os;
}

inline Matrix4 operator *(const Matrix4 &a, const double &b){
    Matrix4 re;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            re.v[i][j] = a.v[i][j] * b;
    return re;
}
inline Matrix4 operator *(const double &b,const Matrix4 &a){
    Matrix4 re;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            re.v[i][j] = a.v[i][j] * b;
    return re;
}


template<class T> T Clamp(T v, T llim, T rlim){
    if (v < llim) return llim;
    if (v > rlim) return rlim;
    return v;
}

double myPow(double a, int b){
    double c = a;
    for (--b; b;){
        if (b & 1) c *= a;
        if (b /= 2) a *= a;
    }
    return c;
}

#endif