#include "entry.h"
static CHAR16* DriverName = L"Fhhfs File System Driver";
static CHAR16* ControllerName = L"Fhhfs File System";
EFI_DRIVER_BINDING_PROTOCOL DriverBindingProtocol = {
    .ImageHandle         = NULL,
    .Version             = 0x10,
    .DriverBindingHandle = NULL,
    .Supported           = FhhfsDriverBindingSupported,
    .Start               = FhhfsDriverBindingStart,
    .Stop                = FhhfsDriverBindingStop
};
EFI_COMPONENT_NAME_PROTOCOL gAbcComponentName = {
  (EFI_COMPONENT_NAME_GET_DRIVER_NAME) FhhFsGetDriverName,
  (EFI_COMPONENT_NAME_GET_CONTROLLER_NAME) FhhFsGetControllerName,
  "eng"
};
EFI_COMPONENT_NAME2_PROTOCOL gAbcComponentName2 = {
  FhhFsGetDriverName,
  FhhFsGetControllerName,
  "en"
};
EFI_STATUS EFIAPI FhhFsGetDriverName(IN EFI_COMPONENT_NAME2_PROTOCOL *This,IN CHAR8 *Language,OUT CHAR16 **DrvName)
{
    *DrvName = DriverName;
    return EFI_SUCCESS;
}
EFI_STATUS EFIAPI FhhFsGetControllerName(IN EFI_COMPONENT_NAME2_PROTOCOL *This,IN EFI_HANDLE ControllerHandle,IN EFI_HANDLE ChildHandle OPTIONAL,IN CHAR8 *Language,OUT CHAR16 **ConName)
{
    *ConName = ControllerName;
    return EFI_SUCCESS;
}
EFI_STATUS EFIAPI FhhfsEntryPoint(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    SystemTable->ConOut->OutputString(SystemTable->ConOut,L"Loading fhhfs FileSystem Driver...\r\n");

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
EFI_STATUS EFIAPI FhhfsDriverBindingSupported(IN EFI_DRIVER_BINDING_PROTOCOL *This,IN EFI_HANDLE ControllerHandle,IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath OPTIONAL)
{
    return EFI_SUCCESS;
}
EFI_STATUS EFIAPI FhhfsDriverBindingStart(IN EFI_DRIVER_BINDING_PROTOCOL *This,IN EFI_HANDLE ControllerHandle,IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath OPTIONAL)
{
    return EFI_SUCCESS;
}
EFI_STATUS EFIAPI FhhfsDriverBindingStop(IN EFI_DRIVER_BINDING_PROTOCOL *This,IN EFI_HANDLE ControllerHandle,IN  UINTN NumberOfChildren,IN  EFI_HANDLE *ChildHandleBuffer OPTIONAL)
{
    return EFI_SUCCESS;
}
