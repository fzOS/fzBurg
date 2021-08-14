//此处加载了UEFI通用的头。
#ifndef PUBLIC
#define PUBLIC
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
