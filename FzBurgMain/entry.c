#include "public.h"
#include "elf.h"
extern EFI_BOOT_SERVICES* gBS;
EFI_STATUS
EFIAPI
UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsProtocol;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystemProtocol;
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL* Root;
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,NULL,(VOID**) &GraphicsProtocol);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Failed to get GOP.Stop.\r\n");
        return Status;
    }
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
    EFI_FILE_PROTOCOL* KernelFile;
    Status = Root->Open(Root,&KernelFile,L"kernel",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot found kernel.Stop.\r\n");
        return Status;
    }
    UINTN KernelFileSize = 0;
    EFI_FILE_INFO* KernelFileInfo;
    Status = KernelFile->GetInfo(KernelFile,&gEfiFileInfoGuid,&KernelFileSize,(VOID*)KernelFileInfo);
    if(Status == EFI_BUFFER_TOO_SMALL) {
        Status = gBS->AllocatePool(EfiLoaderData, KernelFileSize, (VOID**)&KernelFileInfo);
        KernelFile->GetInfo(KernelFile,&gEfiFileInfoGuid,&KernelFileSize,(VOID*)KernelFileInfo);
    }
    Print(L"The kernel size is going to be %d bytes.\r\n",KernelFileInfo->FileSize);
    KernelFileSize = KernelFileInfo->FileSize;
    VOID* KernelFileBuffer;
    Status = gBS->AllocatePool(EfiLoaderData, KernelFileInfo->FileSize, &KernelFileBuffer);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot allocate pool.Stop.\r\n");
        return Status;
    }
    Print(L"Loading kernel file......\r\n");
    UINTN PrevFileSize = KernelFileSize;
    Status = KernelFile->Read(KernelFile,&KernelFileSize,KernelFileBuffer);
    if(EFI_ERROR(Status)||PrevFileSize!=KernelFileSize)
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot read kernel file.Stop.\r\n");
        return Status;
    }
    Print(L"Parsing ELF format......\r\n");
    VOID* KernelEntry = NULL;
    Status = LoadKernel(KernelFileBuffer,&KernelEntry);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot parse kernel.\r\n");
        return Status;
    }
    if(KernelEntry != NULL)
    {
        Print(L"Successfully loaded kernel at 0x%llx\r\n",KernelEntry);
    }
    gBS->FreePool(KernelFileInfo);
    KernelFileInfo = NULL;
    gBS->FreePool(KernelFileBuffer);
    KernelFileBuffer = NULL;
    //获取mmap.
    UINTN MemSize = 0, MemMapKey = 0, MemMapDescSize = 0;
    UINT32 MemMapDescVer =0;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;
    //首先，我们尝试一次以获得真实大小。
    Status  = gBS->GetMemoryMap(&MemSize,MemMap,NULL,&MemMapDescSize,NULL);
    //第一次必然会太小，返回真实的大小。
    Print(L"The real mem size is:%llu\r\n",MemSize);
    MemSize*=2;
    Status = gBS->AllocatePool(EfiLoaderData, MemSize, (void**)&MemMap);
    if(EFI_ERROR(Status))
    {
        Print(L"Cannot allocate mem for Memory map!\r\n");
        return Status;
    }
    //然后，我们可以去读取真实的mmap了。
    Status  = gBS->GetMemoryMap(&MemSize,MemMap,&MemMapKey,&MemMapDescSize,&MemMapDescVer);
    if (EFI_ERROR(Status))
    {
        Print(L"Cannot read Memory map!\r\n");
        return Status;
    }
    Print(L"Got memory map of %d bytes.\r\n",MemSize);
    //退出启动环境。
    gBS->ExitBootServices(ImageHandle,MemMapKey);
    //Ugly!
    typedef struct {
    /*void *xdsp_address;
    uint8_t *memory_map;
    uint64_t mem_map_size;
    uint64_t mem_map_descriptor_size;
    uint64_t kernel_lowest_address;
    uint64_t kernel_page_count;*/
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    } KernelInfo;
    typedef void (*KernelMainFunc)(KernelInfo);
    Print(L"Loading Kernel...\r\n");
    KernelInfo info = {.gop = GraphicsProtocol};
    ((KernelMainFunc)KernelEntry)(info);
    return EFI_SUCCESS;
}