#include "rendering.h"

/*
Texture 类内的变量集合

        int32_t _h,_w; // 原始贴图的高度和宽度
        vector<vector<uint8_t>> _bit; // 原始贴图的颜色信息，存储方式为 第 i 行第 j 列的第 k 个颜色 channel 信息存储在 _bit[i][3j+k] 中
        vector<vector<vector<uint8_t>>> Mipmap; // Mipmap 分层贴图信息，第 0 层表示原始贴图信息。
                                                // 第 i 层的贴图高度，宽度恰好为第 i - 1 层的 1/2
                                                // 每一层贴图存储方式与 _bit 中贴图的存储方式相同
        vector<int32_t> _MipmapH; //Mipmap 分层贴图中每一层的高度
        vector<int32_t> _MipmapW; //Mipmap 分层贴图中每一层的宽度
        
        double alpha = 1.1; //模糊参数
        bool covered; //贴图是否初始化
*/

/*
problem 3. (20%)
在预处理好的 Mipmap 结构中，使用双线性插值法找到第 level 层的 (x, y) 对应的颜色信息
输入：
    level: 查询的 Mipmap 层数，处理后保证不超过预处理时候计算的总层数，也就是不超过 (int)Mipmap.size() - 1
    x, y: 二维坐标，范围在 [0, 1] 之间，表示查询的坐标。我们假设整个纹理的左下角为 (0, 0)，右上角为 (1, 1)。
输出：
    Color 类型，表示 (x, y) 点在第 level 层的 mipmap 上对应的颜色信息。颜色需要从 [0 ~ 255] 整数值映射到 [0 ~ 1] 的浮点数值。
处理方法：
    1. 计算 (x, y) 对应的在第 level 层的 Mipmap 上的坐标，记为 (doubleX, doubleY)
    2. 计算 (doubleX, doubleY) 对应的方格的四个格点。
    3. 对四个格点的颜色进行双线性插值，得到 (doubleX, doubleY) 对应的颜色信息，返回即可。
*/

Color Texture::BilinearFliterColor(double x, double y, int level = 0){
    if (level >= (int)Mipmap.size())
        level = (int)Mipmap.size() - 1;
    if (level < 0)
        level = 0;

    // problem 3: 20%
    // 在此处填写你的代码：
    double doubleX, doubleY;
    //printf("Mipmap Size %d %d\n", _MipmapH.size(), _MipmapW.size());
    doubleX = x * (_MipmapH[level] - 1);
    doubleY = y * (_MipmapW[level] - 1);

    int l = int(doubleX), r = Clamp(l+1, 0, _MipmapH[level]-1),
    t = int(doubleY), b = Clamp(t + 1, 0, _MipmapW[level]-1);
    double red, green, blue;
    //printf("ckpt9\n");
    red = ((Mipmap[level][l][3 * t+0] * (doubleX - double(l)) + 
        Mipmap[level][r][3 * t+0] * (double(r) - doubleX)) * (doubleY - double(t)) + 
        (Mipmap[level][l][3 * b+0] * (doubleX - double(l)) + 
        Mipmap[level][r][3 * b+0] * (double(r) - doubleX)) * (double(b) - doubleY)) / 255.;
    green = ((Mipmap[level][l][3 * t+1] * (doubleX - double(l)) + 
        Mipmap[level][r][3 * t+1] * (double(r) - doubleX)) * (doubleY - double(t)) + 
        (Mipmap[level][l][3 * b+1] * (doubleX - double(l)) + 
        Mipmap[level][r][3 * b+1] * (double(r) - doubleX)) * (double(b) - doubleY)) / 255.;
    blue = ((Mipmap[level][l][3 * t+2] * (doubleX - double(l)) + 
        Mipmap[level][r][3 * t+2] * (double(r) - doubleX)) * (doubleY - double(t)) + 
        (Mipmap[level][l][3 * b+2] * (doubleX - double(l)) + 
        Mipmap[level][r][3 * b+2] * (double(r) - doubleX)) * (double(b) - doubleY)) / 255.;
    // cout << Color(red, green, blue) <<endl;
    // cout << red << blue << green;
    return Color(red, green, blue);
}
