#include "public.h"

EFI_STATUS EFIAPI FhhfsEntryPoint(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Driver Loaded!\r\n");
    return EFI_SUCCESS;
}
EFI_STATUS EFIAPI FhhfsUnload (IN EFI_HANDLE ImageHandle) {
    Print(L"Driver Unloaded!\r\n");
    return EFI_SUCCESS;
}
EFI_GUID gFhhfsPartitionGuid = {
    .Data1 = 0x12345678,
    .Data2 = 0x8765,
    .Data3 = 0x4231,
    .Data4 = {0x23,0x33,0x66,0x66,0x66,0x66,0x66,0x66}
};
