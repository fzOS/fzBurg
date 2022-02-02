#include "public.h"
#include "elf.h"
#include "bmp.h"
extern EFI_BOOT_SERVICES* gBS;
extern EFI_RUNTIME_SERVICES* gRT;

#ifdef FzOS_QUICK_BOOT
#pragma message("You chose Quick boot.")
#endif
EFI_STATUS
EFIAPI
UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsProtocol;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystemProtocol;
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL* Root;
    #ifndef FzOS_QUICK_BOOT
    EFI_EVENT Events[2];
    #endif
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
    //将地址重新映射到0xFFFF800000000000.（保留原始的地址）
    EFI_PHYSICAL_ADDRESS pml4e=0;
    __asm__ volatile ("movq %%cr3,%0":"=g"(pml4e)::);
    pml4e &=0x000FFFFFFFFFF000ULL;
    EFI_PHYSICAL_ADDRESS fake_pml4e;
    gBS->AllocatePages(AllocateAnyPages,EfiRuntimeServicesData,1,&fake_pml4e);
    CopyMem((void*)fake_pml4e,(void*)pml4e,4096);
    CopyMem((void*)fake_pml4e+256*sizeof(UINTN),(void*)fake_pml4e,sizeof(UINTN));
    pml4e &=~(0x000FFFFFFFFFF000ULL);
    pml4e |= fake_pml4e;
    __asm__ volatile ("movq %0,%%cr3"::"l"(pml4e):);    
    Print(L"Pulling kernel to upper space done.\r\n");
    UINT32 Attrs;
    UINTN  DataSize=0;
    CHAR16* Buffer;
    //输出变量。
    Status = gRT->GetVariable (
                  L"BootParameters",
                  &gFzosBootArgumentGuid,
                  &Attrs,
                  &DataSize,
                  Buffer);
    if(Status == EFI_BUFFER_TOO_SMALL) {
        Status = gBS->AllocatePool(EfiBootServicesData,DataSize+1,(VOID**)&Buffer);
        if(EFI_ERROR(Status)) {
            SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot read EFI Variable.Stop.\r\n");
            return Status;
        }
        ZeroMem(Buffer,DataSize+1);
        if(Buffer==NULL) {
            SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot read EFI Variable.Stop.\r\n");
            return Status;
        }
        Status = gRT->GetVariable (
                  L"BootParameters",
                  &gFzosBootArgumentGuid,
                  &Attrs,
                  &DataSize,
                  Buffer);
        if(EFI_ERROR(Status)) {
            SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot read EFI Variable.Stop.\r\n");
            return Status;
        }
        Print(L"Boot Variable:");
        AsciiPrint((CHAR8*)Buffer);
        Print(L"\r\n");
    }
    else {
        SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Cannot read EFI Variable.Stop.\r\n");
        return Status;
    }
    #ifndef FzOS_QUICK_BOOT
    //在此处加入监听用户的键盘(此处设置回车键为触发按键。)
    SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Press <Return> to continue boot, <Tab> to modify boot parameters.\r\n");
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
        if(key.UnicodeChar==CHAR_TAB)
        {
            SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Input new parameter,<Return> to proceed.\r\n");
            //FIXME:STUB!
            while(key.UnicodeChar!=CHAR_CARRIAGE_RETURN) {
                SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn,&key);
                if(key.UnicodeChar!=0x00) {
                    Print(L"%c",key.UnicodeChar);
                }
            }
            break;
        }
    }
    #endif
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
        (KernelEntry) = (VOID*)(((UINTN)KernelEntry)|0xFFFF800000000000ULL);
    }
        
    //分配2页的内核栈。
    uint64_t new_empty_stack = 0ULL;
    gBS->AllocatePages(AllocateAnyPages,EfiRuntimeServicesData,KERNEL_STACK_PAGES,&new_empty_stack);
    
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
        EFI_RUNTIME_SERVICES *rt;
        uint64_t new_empty_stack;
    } KernelInfo;
    typedef void (*KernelMainFunc)(KernelInfo);
    KernelInfo info = 
    {
        .rsdp_address = RsdpAddress+0xFFFF800000000000ULL,
        .memory_map = (UINT8*)((UINTN)MemMap+0xFFFF800000000000ULL),
        .mem_map_size = MemSize,
        .mem_map_descriptor_size = MemMapDescSize,
        .kernel_lowest_address = KenrelLoadAddress,
        .kernel_page_count = KenrelPageCount,
        .gop = (EFI_GRAPHICS_OUTPUT_PROTOCOL*)((UINTN)GraphicsProtocol+0xFFFF800000000000ULL),
        .rt = gRT,
        .new_empty_stack = new_empty_stack
    };
    Status = gBS->ExitBootServices(ImageHandle,MemMapKey);
    if(EFI_ERROR(Status))
    {
        Print(L"Unable to exit boot service! the result is:%d\r\n",Status);
        return Status;
    }
    typedef struct {
        UINTN Type; //和标准不符？？？
        UINTN PhysicalStart;
        UINTN VirtualStart;
        UINTN NumberOfPages;
        UINTN Attribute;
        UINTN StrangePadding;
    } memmap;
    for(int i=0;i<MemSize/MemMapDescSize;i++) {
        (((memmap*)MemMap)+i)->VirtualStart = (((memmap*)MemMap)+i)->PhysicalStart | 0xFFFF800000000000ULL;
    }
    Status = gRT->SetVirtualAddressMap(MemSize,MemMapDescSize,MemMapDescVer,MemMap);
    if(EFI_ERROR(Status)) {
        gRT->ResetSystem(EfiResetWarm,EFI_TIMEOUT,0,NULL);
    }

    //Ugly!
    ((KernelMainFunc)KernelEntry)(info);
    gBS->FreePool(MemMap);
    return EFI_SUCCESS;
}
EFI_GUID gFzosBootArgumentGuid = {
    .Data1 = 0x1234ABCD,
    .Data2 = 0xBEEF,
    .Data3 = 0xBABE,
    .Data4 = {0x23,0x33,0x88,0x88,0x88,0x88,0x88,0x88}
};
