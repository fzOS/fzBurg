#ifndef PUBLIC
#define PUBLIC
#if( !defined _M_AMD64 && !defined __amd64)
#error "Not targeting AMD64. Abort."
#endif
#include <X64/ProcessorBind.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Guid/FileInfo.h>
#include <Library/BaseMemoryLib.h>
extern EFI_GUID gFhhfsPartitionGuid;
#define KERNEL_STACK_PAGES 4
#endif

