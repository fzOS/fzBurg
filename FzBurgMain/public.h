//此处加载了UEFI通用的头。
#ifndef PUBLIC
#define PUBLIC
#if( !defined _M_AMD64 && !defined __amd64)
#error "Not targeting AMD64. Abort."
#endif
#include <X64/ProcessorBind.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Guid/FileInfo.h>
#include <Guid/Acpi.h>
#include <Library/BaseMemoryLib.h>
#include <IndustryStandard/Bmp.h>

#define KERNEL_STACK_PAGES 4
#endif
