#ifndef BMP_H_INCLUDED
#define BMP_H_INCLUDED

#include "public.h"
EFI_STATUS DisplayImage( EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop, VOID* BmpBuffer);
EFI_STATUS PrintBMP( VOID* BmpBuffer);
typedef struct
{
    UINT8 Blue;
    UINT8 Green;
    UINT8 Red;
} __attribute__((packed)) BMP_PIXEL;

#endif // BMP_H_INCLUDED
