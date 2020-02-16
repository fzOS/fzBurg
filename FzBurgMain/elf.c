#include "elf.h"
#include "public.h"
extern EFI_BOOT_SERVICES* gBS;
EFI_STATUS LoadKernel(VOID* File,void **entry_address)
{
    //首先，检查它是否是一个ELF文件。
    Elf64_Ehdr* Header = (Elf64_Ehdr*) File;
    if (Header->e_ident[0] != 0x7F ||
        Header->e_ident[1] != 'E' ||
        Header->e_ident[2] != 'L' ||
        Header->e_ident[3] != 'F' )
         return EFI_UNSUPPORTED;
    //然后，检查它是否是一个64位的ELF。
    if (Header->e_ident[4] != 2)
        return EFI_UNSUPPORTED;
    if (Header->e_machine != 62) 
        return EFI_UNSUPPORTED;
    Elf64_Phdr *program_headers = (Elf64_Phdr *)((char*)File + Header->e_phoff);
    //以下是抄来的代码。
    Elf64_Addr highest_addr_found = 0;
    Elf64_Addr lowest_addr_found = -1;
    for (int i = 0; i < Header->e_phnum; ++i) {
        if (program_headers[i].p_type == 1) {
            Elf64_Addr chunk_end = program_headers[i].p_vaddr + program_headers[i].p_memsz;
            if (chunk_end > highest_addr_found) highest_addr_found = chunk_end;
            if (program_headers[i].p_vaddr < lowest_addr_found) lowest_addr_found = program_headers[i].p_vaddr;
        }
    }
    int bytes_needed = (int)(highest_addr_found - lowest_addr_found);
    int num_pages_needed = (bytes_needed / EFI_PAGE_SIZE) + 1;
    EFI_PHYSICAL_ADDRESS region = lowest_addr_found;
    EFI_STATUS Status;
    //以上。
    Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, num_pages_needed, &region);
    if(EFI_ERROR(Status)|| region != lowest_addr_found)
    {
        Print(L"Error allocating pages for kernel.\n");
        Print(L"status: %d, region: 0x%x, e_entry: 0x%x\n", Status, region, Header->e_entry);
        return EFI_ABORTED;
    }
    for (int i = 0; i < Header->e_phnum; ++i) {
        Elf64_Phdr ph = program_headers[i];
        if (ph.p_type == 1) {
            // Copy as much data as we have to beginning
            CopyMem((void *)ph.p_vaddr, File + ph.p_offset, ph.p_filesz);
            // Zero out the rest of the section
            ZeroMem((void *)(ph.p_vaddr + ph.p_filesz),ph.p_memsz - ph.p_filesz);
        }
    }
    *entry_address = (void *)Header->e_entry;
    return EFI_SUCCESS;
}