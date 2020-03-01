#include "bmp.h"
#include "public.h"
EFI_STATUS
DisplayImage( EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop, 
              VOID *BmpBuffer)
{
    BMP_IMAGE_HEADER *BmpHeader = (BMP_IMAGE_HEADER *) BmpBuffer;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer;
    EFI_STATUS Status = EFI_SUCCESS;
    BMP_PIXEL  *BitmapData;
    UINTN   Pixels;
    UINTN   XIndex;
    UINTN   YIndex;
    UINTN   Pos;
    UINTN   BltPos;

    BitmapData = (BMP_PIXEL*)((UINT8*)BmpBuffer + BmpHeader->ImageOffset);

    Pixels = BmpHeader->PixelWidth * BmpHeader->PixelHeight;
    BltBuffer = AllocateZeroPool( sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * Pixels);
    if (BltBuffer == NULL) {
        Print(L"ERROR: BltBuffer. No memory resources\n");
        return EFI_OUT_OF_RESOURCES;
    }

    for (YIndex = BmpHeader->PixelHeight; YIndex > 0; YIndex--) {
        for (XIndex = 0; XIndex < BmpHeader->PixelWidth; XIndex++) {
            Pos    = (YIndex - 1) * ((BmpHeader->PixelWidth + 3) / 4) * 4 + XIndex;
            BltPos = (BmpHeader->PixelHeight - YIndex) * BmpHeader->PixelWidth + XIndex;
            BltBuffer[BltPos].Blue = BitmapData[Pos].Blue;
            BltBuffer[BltPos].Green = BitmapData[Pos].Green;
            BltBuffer[BltPos].Red = BitmapData[Pos].Red;
            BltBuffer[BltPos].Reserved = 0;
            // BltBuffer[BltPos].Blue     = (UINT8) BitFieldRead32(Palette[BitmapData[Pos]], 0 , 7 );
            // BltBuffer[BltPos].Green    = (UINT8) BitFieldRead32(Palette[BitmapData[Pos]], 8 , 15);
            // BltBuffer[BltPos].Red      = (UINT8) BitFieldRead32(Palette[BitmapData[Pos]], 16, 23);
            // BltBuffer[BltPos].Reserved = (UINT8) BitFieldRead32(Palette[BitmapData[Pos]], 24, 31);
        }
    }


    Status = Gop->Blt( Gop,
                       BltBuffer,
                       EfiBltBufferToVideo,
                       0, 0,            /* Source X, Y */
                       0, 0,          /* Dest X, Y */
                       BmpHeader->PixelWidth, BmpHeader->PixelHeight, 
                       0);

    FreePool(BltBuffer);

    return Status;
}


//
// Print the BMP header details
//
EFI_STATUS
PrintBMP( VOID* BmpBuffer)
{
    BMP_IMAGE_HEADER* BmpHeader = (BMP_IMAGE_HEADER *)BmpBuffer;
    EFI_STATUS Status = EFI_SUCCESS;

    // not BMP format
    if (BmpHeader->CharB != 'B' || BmpHeader->CharM != 'M') {
        Print(L"ERROR: Unsupported image format\n"); 
        return EFI_UNSUPPORTED;
    }

    // BITMAPINFOHEADER format unsupported
    if (BmpHeader->HeaderSize != sizeof (BMP_IMAGE_HEADER) \
        - ((UINTN) &(((BMP_IMAGE_HEADER *)0)->HeaderSize))) {
        Print(L"ERROR: Unsupported BITMAPFILEHEADER\n");
        return EFI_UNSUPPORTED;
    }

    // compression type not 0
    if (BmpHeader->CompressionType != 0) {
        Print(L"ERROR: Compression type not 0\n");
        return EFI_UNSUPPORTED;
    }

    // unsupported bits per pixel
    if (BmpHeader->BitPerPixel != 4 &&
        BmpHeader->BitPerPixel != 8 &&
        BmpHeader->BitPerPixel != 12 &&
        BmpHeader->BitPerPixel != 24) {
        Print(L"ERROR: Bits per pixel is not one of 4, 8, 12 or 24\n");
        return EFI_UNSUPPORTED;
    }

    Print(L"\n");
    Print(L"Size              : %d\n", BmpHeader->Size);
    Print(L"Image Offset      : %d\n", BmpHeader->ImageOffset);
    Print(L"Header Size       : %d\n", BmpHeader->HeaderSize);
    Print(L"Image Width       : %d\n", BmpHeader->PixelWidth);
    Print(L"Image Height      : %d\n", BmpHeader->PixelHeight);
    Print(L"Planes            : %d\n", BmpHeader->Planes);
    Print(L"Bit Per Pixel     : %d\n", BmpHeader->BitPerPixel);
    Print(L"Compression Type  : %d\n", BmpHeader->CompressionType);
    Print(L"Image Size        : %d\n", BmpHeader->ImageSize);
    Print(L"X Pixels Per Meter: %d\n", BmpHeader->XPixelsPerMeter);
    Print(L"Y Pixels Per Meter: %d\n", BmpHeader->YPixelsPerMeter);
    Print(L"Number of Colors  : %d\n", BmpHeader->NumberOfColors);
    Print(L"Important Colors  : %d\n", BmpHeader->ImportantColors);

    return Status;
}

