#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Protocol/GraphicsOutput.h>
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

    // EFI_GRAPHICS_OUTPUT_BLT_PIXEL BltBuffer[1] = {{0xCC,0xCC,0x75,0}};
    // GraphicsProtocol->Blt(GraphicsProtocol,BltBuffer,EfiBltVideoFill,0,0,0,0,ScreenWidth,ScreenHeight,0);
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
    Print(L"File is ready.\r\n");
    return EFI_SUCCESS;
}