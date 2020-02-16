#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/SafeIntLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Guid/FileInfo.h>
#include "../libeg/libegint.h"
EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsProtocol;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystemProtocol;
extern EFI_BOOT_SERVICES *gBS;

EFI_STATUS
EFIAPI
UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL* Root;
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,NULL,(VOID**) &GraphicsProtocol);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Failed to get GOP.Stop.\r\n");
        return Status;
    }
    UINT32 ScreenWidth=0,ScreenHeight=0;
    ScreenWidth = GraphicsProtocol->Mode->Info->HorizontalResolution;
    ScreenHeight = GraphicsProtocol->Mode->Info->VerticalResolution;
    SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Hello World!\r\n");
    ScreenWidth += 0;
    ScreenHeight += 0;
    Print(L"According to the GOP,the current Graphics resolution is %d * %d\n",ScreenWidth,ScreenHeight);
    Status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid,NULL,(VOID**)&FileSystemProtocol);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot open EFI partition.Stop.\r\n");
        return Status;
    }
    Status = FileSystemProtocol->OpenVolume(FileSystemProtocol,&Root);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot open root.Stop.\r\n");
        return Status;
    }
    EFI_FILE_PROTOCOL* Bg;
    Status = Root->Open(Root,&Bg,L"theme\\bg.bmp",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot found bg.bmp.Stop.\r\n");
        return Status;
    }
    UINTN BgFileSize = 0;
    EFI_FILE_INFO* BgFileInfo;
    Status = Bg->GetInfo(Bg,&gEfiFileInfoGuid,&BgFileSize,(VOID**)BgFileInfo);
    if(Status == EFI_BUFFER_TOO_SMALL) {
        Status = gBS->AllocatePool(EfiBootServicesData, BgFileSize, (VOID**)&BgFileInfo);
        Bg->GetInfo(Bg,&gEfiFileInfoGuid,&BgFileSize,(VOID**)BgFileInfo);
    }
    Print(L"The background image file size is going to be %d bytes.\r\n",BgFileInfo->FileSize);
    VOID* BgFile;
    Status = gBS->AllocatePool(EfiBootServicesData, BgFileInfo->FileSize, &BgFile);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot allocate pool.Stop.\r\n");
        return Status;
    }
    BgFileSize = BgFileInfo->FileSize;
    Print(L"Reading BMP file......\r\n");
    Status = Bg->Read(Bg,&BgFileSize,BgFile);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot read BMP file.Stop.\r\n");
        return Status;
    }
    Print(L"Converting BMP to Blt format......\r\n");
    EG_IMAGE* image = egDecodeBMP((UINT8*)BgFile,BgFileSize,0,FALSE);
    Print(L"Calling Blt......\r\n");
    Status = GraphicsProtocol->Blt(GraphicsProtocol,(EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)(image->PixelData),EfiBltBufferToVideo,0,0,0,0,image->Width,image->Height,0);
    //EFI_GRAPHICS_OUTPUT_BLT_PIXEL* GopBlt;
    //UINTN GopBltSize, PixelHeight, PixelWidth;
    // //Status = ConvertBmpToGopBlt(BgFile,BgFileSize,&GopBlt,&GopBltSize,&PixelHeight,&PixelWidth);
    // if(EFI_ERROR(Status))
    // {
    //     SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot convert BMP file into Blt.Stop.\r\n");
    //     return Status;
    // }
    // Print(L"The converted resolution is %d * %d,with %d bytes per pixel.\r\n",PixelWidth,PixelHeight,GopBltSize/PixelHeight/PixelWidth);
    // Print(L"Calling Blt......\r\n");
    // UINTN CoordinateX = (UINTN) (GraphicsProtocol->Mode->Info->HorizontalResolution / 2) - (PixelWidth / 2);
	// 	UINTN CoordinateY = (UINTN) (GraphicsProtocol->Mode->Info->VerticalResolution / 2) - (PixelHeight / 2);
    // Status = GraphicsProtocol->Blt(GraphicsProtocol,GopBlt,EfiBltBufferToVideo,0,0,CoordinateX,CoordinateY,PixelWidth,PixelHeight,0);
    gBS->FreePool(BgFile);
    gBS->FreePool(BgFileInfo);
    return EFI_SUCCESS;
}