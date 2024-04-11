//
#include <iostream>
#include <Windows.h>
#include <malloc.h>
#include <vector>

using namespace std;

typedef struct{
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    vector<vector<uint8_t>> imgData;
}ImgInfo;

//根据图片路径读取Bmp图像，生成ImgInfo对象
ImgInfo readBitmap(string imgPath) {
    ImgInfo imgInfo;
    char* buf;                                              //定义文件读取缓冲区
    char* p;

    FILE* fp;
    fopen_s(&fp, imgPath.c_str(), "rb");
    if (fp == NULL) {
        cout << "Open Fail." << endl;
        exit(0);
    }

    fread(&imgInfo.bf, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&imgInfo.bi, sizeof(BITMAPINFOHEADER), 1, fp);
    printf("Image Width : %d  Image Height : %d\n", imgInfo.bi.biWidth, imgInfo.bi.biHeight);
    if (imgInfo.bi.biBitCount != 24){
        printf("Don't Support %d Bit BitMap",imgInfo.bi.biBitCount);
        exit(0);
    }

    fseek(fp, imgInfo.bf.bfOffBits, 0);

    buf = (char*)malloc(imgInfo.bi.biWidth * imgInfo.bi.biHeight * 3);
    fread(buf, 1, imgInfo.bi.biWidth * imgInfo.bi.biHeight * 3, fp);

    p = buf;
    // printf("ckpt2\n");
    vector<vector<uint8_t>> imgData;
    for (int y = 0; y < imgInfo.bi.biHeight; y++){
        vector<uint8_t> vRGB;
        for (int x = 0; x < imgInfo.bi.biWidth; x++) {
            uint8_t r, g, b;
            r = *(p++);
            g = *(p++);
            b = *(p++);

            vRGB.push_back(b);     //blue
            vRGB.push_back(g);     //green
            vRGB.push_back(r);     //red

            if (x == imgInfo.bi.biWidth - 1)
            {
                for (int k = 0; k < imgInfo.bi.biWidth % 4; k++) p++;
            }
            
        }
        imgData.push_back(vRGB);
    }
    fclose(fp);
    imgInfo.imgData = imgData;
    return imgInfo;
}


void saveBitmap(ImgInfo imgInfo, string & saveImgPath) {
    FILE* fpw;
    fopen_s(&fpw, saveImgPath.c_str(), "wb");
    fwrite(&imgInfo.bf, sizeof(BITMAPFILEHEADER), 1, fpw);  //写入文件头
    fwrite(&imgInfo.bi, sizeof(BITMAPINFOHEADER), 1, fpw);  //写入文件头信息

    int size = imgInfo.bi.biWidth * imgInfo.bi.biHeight;
    for (int i = 0; i < size; i++) {
        fwrite(&imgInfo.imgData.at(i).at(0), 1, 1, fpw);
        fwrite(&imgInfo.imgData.at(i).at(1), 1, 1, fpw);
        fwrite(&imgInfo.imgData.at(i).at(2), 1, 1, fpw);

        if (i % imgInfo.bi.biWidth == imgInfo.bi.biWidth - 1) {
            char ch = '0';
            for (int j = 0; j < imgInfo.bi.biWidth % 4; j++) {
                fwrite(&ch, 1, 1, fpw);
            }
        }
    }
    fclose(fpw);
    cout << "Image Has : " + saveImgPath << endl;
}
