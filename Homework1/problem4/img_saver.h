#include<cstdio>
#include<iostream>
#include<fstream>
#include<cstring>
#include<Windows.h>
#include<stdio.h>
#include<fstream>
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

void Cov_uint8(char* ptr, unsigned char val){
    *((unsigned char*)ptr) = val;
}
void Cov_uint16(char* ptr, unsigned short val){
    *((unsigned short*)ptr) = val;
}
void Cov_uint32(char* ptr, unsigned int val){
    *((unsigned int*)ptr) = val;
}




/*
按照 BMP 格式存储图片
*/
void saveImg(int Width, int Height, unsigned char* frame_buf, const char* out_file){
    
    std::ofstream fout(out_file, std::ios::binary|std::ios::ate);
    if (!fout.is_open()){
        std::cerr << "failed to save an Image" << std::endl;
    }
    else{
        std::cerr << "successfully save an Image" << std::endl;
    }
    char* buf;
    int BytePerRow = Width * 3;
    BytePerRow += (4 - BytePerRow % 4) % 4;
    int len = 0x36 + BytePerRow * Height;// Head + Info;
    

    buf = new char[(len + 5)];
    memset(buf, 0x00, sizeof(char) * (len + 5));

    Cov_uint8(buf + 0x00, 0x42); 
    Cov_uint8(buf + 0x01, 0x4D);    // BM
    Cov_uint32(buf + 0x02, len);    // 文件大小
    Cov_uint32(buf + 0x06, 0);      // Reversed, set 0
    Cov_uint32(buf + 0x0A, 0x36);   // The offset, i.e. starting address

    Cov_uint32(buf + 0x0E, 0x28);   // Size of header
    Cov_uint32(buf + 0x12, Width);  // Width
    Cov_uint32(buf + 0x16, Height); // Height
    Cov_uint16(buf + 0x1A, 0x01);   // The number of color planes (must be 1)
    Cov_uint16(buf + 0x1C, 0x18);   // Bits per Pixel
    Cov_uint16(buf + 0x1E, 0x00);   // BI_RGB
    Cov_uint32(buf + 0x22, 0x00);   // Image Size, 0 in BI_RGB
    Cov_uint32(buf + 0x26, 0x1EC2); // Horizontal resolution
    Cov_uint32(buf + 0x2A, 0x1EC2); // Vertical resolution
    Cov_uint32(buf + 0x2E, 0x00);   // Number of colors, 0 to default to 2^n
    Cov_uint32(buf + 0x32, 0x00);   // Number of important colors, 0 to every color is important
    cout << Width<<endl;
    printf("%x ",*((unsigned int*)(buf+0x12)));
    for (int i = 0; i < Height; i++)
        for (int j = 0; j < Width; j++){
            int offset_img = (Height - 1 - i) * BytePerRow + j * 3 + 0x36;
            int offset_frame = i * Width * 4 + j * 4;
            Cov_uint8(buf + offset_img + 0x00, frame_buf[offset_frame + 0]);
            Cov_uint8(buf + offset_img + 0x01, frame_buf[offset_frame + 1]);
            Cov_uint8(buf + offset_img + 0x02, frame_buf[offset_frame + 2]);
        }
    for (int i = 0; i < len; i++)
        fout << buf[i];
    
    fout.close();
    delete[] buf;
}

