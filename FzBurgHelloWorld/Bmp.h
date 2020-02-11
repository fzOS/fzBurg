/** @file
  This file defines BMP file header data structures.

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _BMP_H_
#define _BMP_H_
#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include <Pi/PiFirmwareFile.h>

#pragma pack(1)

typedef struct {
  UINT8   Blue;
  UINT8   Green;
  UINT8   Red;
  UINT8   Reserved;
} BMP_COLOR_MAP;

typedef struct {
  CHAR8         CharB;
  CHAR8         CharM;  //'BM':表示为bmp文件
  UINT32        Size;   //位图文件的大小
  UINT16        Reserved[2];    //保留字，必须为0
  UINT32        ImageOffset;    //位图数据距离文件头的偏移
  UINT32        HeaderSize;     //文件头长度（从此字段到结构尾的长度，40）
  UINT32        PixelWidth;     //位图宽
  UINT32        PixelHeight;    //位图高
  UINT16        Planes;          ///< Must be 1
  UINT16        BitPerPixel;     ///< 1, 4, 8, or 24(程序中只支持这些)
  UINT32        CompressionType; //压缩方式，可以是0,1,2。0表示不压缩
                                 //1 BI_RLE8 8比特游程编码(RLE),只用于8位位图
                                 //2 BI_RLE4 4比特游程编码(RLE),只用于4位位图
                                 //3,4,5不解释   
  UINT32        ImageSize;       //实际位图数据占用的字节数
  UINT32        XPixelsPerMeter; //X方向分辨率
  UINT32        YPixelsPerMeter; //Y方向分辨率
  UINT32        NumberOfColors;  //使用的颜色数，如果为0，则表示默认值(2^BitPerPixel)
  UINT32        ImportantColors; //重要影响的颜色索引数目，为0则表示所有都重要
} BMP_IMAGE_HEADER;

#pragma pack()

#endif
