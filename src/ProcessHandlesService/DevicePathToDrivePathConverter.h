#pragma once

class DevicePathToDrivePathConverter final : boost::noncopyable {
  std::map<std::wstring /* DeviceName */, wchar_t /* DriveLetter */> _deviceNameToDriveLetterMap;

public:
  // Creates a conversion map {DeviceName, DriveLetter}, consisting of all available logical drives on the machine.
  // Example:
  //   [ {"\Device\HardDiskVolume2\"                                 , 'C'},
  //     {"\Device\HardDiskVolume15\"                                , 'D'},
  //     {"\Device\VBoxMiniRdr\;H:\VBoxSvr\My-H\"                    , 'H'},
  //     {"\Device\LanmanRedirector\;I:000215d7\10.22.3.84\i\"       , 'I'},
  //     {"\Device\LanmanRedirector\;S:000215d7\10.22.3.84\devshare\", 'S'},
  //     {"\Device\LanmanRedirector\;U:000215d7\10.22.3.190\d$\"     , 'U'},
  //     {"\Device\LanmanRedirector\;V:000215d7\10.22.3.153\d$\"     , 'V'},
  //     {"\Device\CdRom0\"                                          , 'X } ]
  DevicePathToDrivePathConverter() {
    for (auto driveLetter = L'A'; driveLetter <= L'Z'; driveLetter++) {
      std::array<wchar_t, 10000> deviceNameBuf;
      const wchar_t drive[]{ driveLetter, L':', 0 };

      const auto length = QueryDosDevice(/* lpDeviceName */ drive,
                                         /* lpTargetPath */ deviceNameBuf.data(),
                                         /* ucchMax      */ static_cast<DWORD>(deviceNameBuf.size()));
      if (length == 0) {
        // Failed to find a device name for the corresponding driveLetter
        continue;
      }

      // The returned from QueryDosDevice device name doesn't have an '\' at the end.
      // We add it to be able to distinguish "\Device\HardDiskVolume1\path" from "\Device\HardDiskVolume10\path"
      // when converting device names to drive letters.
      _deviceNameToDriveLetterMap.emplace(std::wstring{ deviceNameBuf.data() } + L'\\', driveLetter);
    }
  }

  // Converts path
  // from "\Device\HardDiskVolume3\Windows\System32\en-US\KernelBase.dll.mui"
  // to   "C:\Windows\System32\en-US\KernelBase.dll.mui".
  std::optional<std::wstring> GetDriveLetterBasedFullName(const std::wstring_view deviceBasedFileFullName) const {
    for (const auto& [deviceName, driveLetter] : _deviceNameToDriveLetterMap) {
      if (deviceBasedFileFullName.starts_with(deviceName)) {
        return std::format(L"{}:\\{}", driveLetter, deviceBasedFileFullName.substr(deviceName.length()));
      }
    }
    return {};
  }
};
