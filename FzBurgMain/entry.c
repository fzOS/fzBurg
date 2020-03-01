#include "public.h"
#include "elf.h"
#include "bmp.h"
extern EFI_BOOT_SERVICES* gBS;
extern EFI_RUNTIME_SERVICES* gRT;
EFI_STATUS
EFIAPI
UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsProtocol;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystemProtocol;
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL* Root;
    EFI_EVENT Events[2];
    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,NULL,(VOID**) &GraphicsProtocol);
    GraphicsProtocol->SetMode(GraphicsProtocol,GraphicsProtocol->Mode->MaxMode);
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
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
    EFI_FILE_PROTOCOL* BgFile;
    Status = Root->Open(Root,&BgFile,L"bg.bmp",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot found BG file.Stop.\r\n");
        return Status;
    }
    UINTN BgFileSize = 0;
    EFI_FILE_INFO* BgFileInfo;
    Status = BgFile->GetInfo(BgFile,&gEfiFileInfoGuid,&BgFileSize,(VOID*)BgFileInfo);
    if(Status == EFI_BUFFER_TOO_SMALL) {
        Status = gBS->AllocatePool(EfiLoaderData, BgFileSize, (VOID**)&BgFileInfo);
        BgFile->GetInfo(BgFile,&gEfiFileInfoGuid,&BgFileSize,(VOID*)BgFileInfo);
    }
    BgFileSize = BgFileInfo->FileSize;
    VOID* BgFileBuffer;
    Status = gBS->AllocatePool(EfiLoaderData, BgFileInfo->FileSize, &BgFileBuffer);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot allocate pool.Stop.\r\n");
        return Status;
    }
    UINTN PrevFileSize = BgFileSize;
    Status = BgFile->Read(BgFile,&BgFileSize,BgFileBuffer);
    if(EFI_ERROR(Status)||PrevFileSize!=BgFileSize)
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot read BG file.Stop.\r\n");
        return Status;
    }
    DisplayImage(GraphicsProtocol,BgFileBuffer);
    //在此处加入监听用户的键盘(此处设置回车键为触发按键。)
    SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Press Return to continue boot.\r\n");
    while(1)
    {
        EFI_INPUT_KEY key={0,0};
        UINTN index=0;
        SystemTable->ConIn->Reset(SystemTable->ConIn,FALSE);
        Events[0] = SystemTable->ConIn->WaitForKey;
        Status = gBS->WaitForEvent(1,Events,&index);
        if(EFI_ERROR(Status))
        {
            SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot bind key event.Stop.\r\n");
            return EFI_UNSUPPORTED;
        }
        Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn,&key);
        if(EFI_ERROR(Status))
        {
            Print(L"Read Key returned with value %d\n",Status);
        }
        if(key.UnicodeChar==CHAR_CARRIAGE_RETURN)
        {
            SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Continuing boot.\r\n");
            break;
        }
    }
    EFI_FILE_PROTOCOL* KernelFile;
    Status = Root->Open(Root,&KernelFile,L"kernel",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
    if(EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot found kernel.Stop.\r\n");
        return Status;
    }
    SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Found kernel.\r\n");
    UINTN KernelFileSize = 0;
    EFI_FILE_INFO* KernelFileInfo;
    Status = KernelFile->GetInfo(KernelFile,&gEfiFileInfoGuid,&KernelFileSize,(VOID*)KernelFileInfo);
    if(Status == EFI_BUFFER_TOO_SMALL) {
        Print(L"Buffer Too Small!The buffer is going to be %d bytes.\r\n",KernelFileSize);
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
    PrevFileSize = KernelFileSize;
    Status = KernelFile->Read(KernelFile,&KernelFileSize,KernelFileBuffer);
    if(EFI_ERROR(Status)||PrevFileSize!=KernelFileSize)
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot read kernel file.Stop.\r\n");
        return Status;
    }
    Print(L"Parsing ELF format......\r\n");
    VOID* KernelEntry = NULL;
    UINTN KenrelLoadAddress,KenrelPageCount;
    Status = LoadKernel(KernelFileBuffer,&KernelEntry,&KenrelLoadAddress,&KenrelPageCount);
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
    //找到ACPI RSDP.
    VOID* RsdpAddress=NULL;
    EFI_CONFIGURATION_TABLE* C=SystemTable->ConfigurationTable;
    for (int Index = 0; Index < SystemTable->NumberOfTableEntries; Index++) {
        EFI_GUID Table_Guid = C->VendorGuid;
        if(!(CompareMem(&Table_Guid,&gEfiAcpiTableGuid,sizeof(EFI_GUID))))
         {
            RsdpAddress = C->VendorTable;
         }
         C++;
    }
    if(RsdpAddress==NULL)
    {
        Print(L"Cannot found ACPI Table.Stop.\r\n");
        return EFI_UNSUPPORTED;
    }
    //获取mmap.
    UINTN MemSize = 0, MemMapKey = 0, MemMapDescSize = 0;
    UINT32 MemMapDescVer =0;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;
    //首先，我们尝试一次以获得真实大小。
    Status  = gBS->GetMemoryMap(&MemSize,MemMap,NULL,&MemMapDescSize,NULL);
    if(Status!=EFI_BUFFER_TOO_SMALL)
    {
        Print(L"Unknown error!\r\n");
        return Status;
    }
    //第一次必然会太小，返回真实的大小。
    Print(L"The real mem size is:%llu\r\n",MemSize);
    MemSize*=2;
    Status = gBS->AllocatePool(EfiLoaderData, MemSize, (void**)&MemMap);
    if(EFI_ERROR(Status))
    {
        Print(L"Cannot allocate mem for Memory map!\r\n");
        return Status;
    }
    Print(L"Reading mmap....\r\n");
    //然后，我们可以去读取真实的mmap了。
    Status  = gBS->GetMemoryMap(&MemSize,MemMap,&MemMapKey,&MemMapDescSize,&MemMapDescVer);
    if (EFI_ERROR(Status))
    {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot get memmap.stop.\r\n");        
    }
    typedef struct {
        void *rsdp_address;
        UINT8 *memory_map;
        uint64_t mem_map_size;
        uint64_t mem_map_descriptor_size;
        uint64_t kernel_lowest_address;
        uint64_t kernel_page_count;
        EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    } KernelInfo;
    typedef void (*KernelMainFunc)(KernelInfo);
    KernelInfo info = 
    {
        .rsdp_address = RsdpAddress,
        .memory_map = (UINT8*)MemMap,
        .mem_map_size = MemSize,
        .mem_map_descriptor_size = MemMapDescSize,
        .kernel_lowest_address = KenrelLoadAddress,
        .kernel_page_count = KenrelPageCount,
        .gop = GraphicsProtocol
    };
    Status = gBS->ExitBootServices(ImageHandle,MemMapKey);
    if(EFI_ERROR(Status))
    {
        Print(L"Unable to exit boot service! the result is:%d\r\n",Status);
        return Status;
    }
    //gRT->ResetSystem(EfiResetWarm,EFI_TIMEOUT,0,NULL);
    //Ugly!

    ((KernelMainFunc)KernelEntry)(info);
    Print(L"Unable to successfully exit boot services. Last status: %d\n",
        Status);
    gBS->FreePool(MemMap);
    return EFI_SUCCESS;
}