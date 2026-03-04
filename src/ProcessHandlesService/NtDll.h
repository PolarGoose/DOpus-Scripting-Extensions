#pragma once

struct SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX {
  // Pointer to the handle in the kernel virtual address space.
  void* Object;

  // PID that owns the handle
  ULONG_PTR UniqueProcessId;

  // Handle value in the process that owns the handle.
  HANDLE HandleValue;

  // Access rights associated with the handle.
  // Bit mask consisting of the fields defined in the winnt.h
  // For example: READ_CONTROL|DELETE|SYNCHRONIZE|WRITE_DAC|WRITE_OWNER|EVENT_ALL_ACCESS
  // The exact information that this field contain depends on the type of the handle.
  ULONG GrantedAccess;

  // This filed is reserved for debugging purposes.
  // For instance, it can store an index to a stack trace that was captured when the handle was created.
  USHORT CreatorBackTraceIndex;

  // Type of object a handle refers to.
  // For instance: file, thread, or process
  USHORT ObjectTypeIndex;

  // Bit mask that provides additional information about the handle.
  // For example: OBJ_INHERIT, OBJ_EXCLUSIVE
  // The attributes are defined in the winternl.h
  ULONG HandleAttributes;

  ULONG Reserved;
};

struct SYSTEM_HANDLE_INFORMATION_EX {
  ULONG_PTR NumberOfHandles;
  ULONG_PTR Reserved;
  SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
};

class NtDll final : boost::noncopyable {
private:
  decltype(NtQuerySystemInformation)* ntQuerySystemInformation_func;

public:
  NtDll() {
    const auto& ntDll = LoadLibrary(L"ntdll.dll");
    if (!ntDll) {
      THROW_WINAPI_EX(LoadLibrary);
    }

#define GET_FUNCTION(dll, function) GetFunction<decltype(function)>(dll, L#function, #function)

    ntQuerySystemInformation_func = GET_FUNCTION(ntDll, NtQuerySystemInformation);

#undef GET_FUNCTION
  }

  auto QuerySystemHandleInformation() const {
    const NTSTATUS STATUS_INFO_LENGTH_MISMATCH = 0xC0000004L; // Copied from ntstatus.h because "um/winnt.h" conflicts with general inclusion of "ntstatus.h"
    const auto SystemExtendedHandleInformation = static_cast<SYSTEM_INFORMATION_CLASS>(64);
    const size_t mb = 1024 * 1024;
    const size_t gb = mb * 1024;

    for (size_t bufSize = 32 * mb; bufSize <= gb; bufSize *= 2) {
      auto buf = std::make_unique<std::byte[]>(bufSize);

      ULONG returnedLength;
      const auto& status = ntQuerySystemInformation_func(/* SystemInformationClass  */ SystemExtendedHandleInformation,
                                                         /* SystemInformation       */ buf.get(),
                                                         /* SystemInformationLength */ static_cast<ULONG>(bufSize),
                                                         /* ReturnLength            */ &returnedLength);

      if (NT_SUCCESS(status)) {
        return std::unique_ptr<SYSTEM_HANDLE_INFORMATION_EX>(reinterpret_cast<SYSTEM_HANDLE_INFORMATION_EX*>(buf.release()));
      }

      if (status != STATUS_INFO_LENGTH_MISMATCH) {
        THROW_WEXCEPTION(L"NtQuerySystemInformation failed with NTSTATUS 0x{:08X}", status);
      }
    }

    THROW_WINAPI_EX_MSG(NtQuerySystemInformation, L"buffer size is not enough");
  }

private:
  template <typename T>
  T* GetFunction(const HMODULE dll, const wchar_t* functionNameWide, const char* functionName) {
    const auto function = reinterpret_cast<T*>(GetProcAddress(dll, functionName));
    if (!function) {
      THROW_WINAPI_EX_MSG(GetProcAddress, L"{}", functionNameWide);
    }
    return function;
  }
};
