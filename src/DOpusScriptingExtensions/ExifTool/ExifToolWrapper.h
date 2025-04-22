#pragma once

class ExifToolWrapper : NonCopyableAndNonMovable {
public:
  static ExifToolWrapper* GetInstance() {
    static ExifToolWrapper instance;
    return &instance;
  }

  // The returned JSON looks like this:
  //   [{ 
  //    "SourceFile": "C:/Windows/SystemResources/Windows.UI.SettingsAppThreshold/SystemSettings/Assets/HDRSample.mkv",
  //    "ExifTool:ExifTool:ExifToolVersion" : {
  //      "desc": "ExifTool Version Number",
  //      "id" : "ExifToolVersion",
  //      "val" : 13.27
  //    },
  //    "File:System:FileName" : {
  //      "desc": "File Name",
  //      "id" : "FileName",
  //      "val" : "HDRSample.mkv"
  //    },
  //    "File:System:FileSize": { 
  //      "desc": "File Size", 
  //      "id": "FileSize", 
  //      "num": 1764666, 
  //      "val": "1765 kB" 
  //    },   
  //    ...
  //   }]
  std::string GetTagInfosJson(std::wstring_view filePath, const std::vector<std::string>& tagNames) {
    if (!std::filesystem::exists(filePath))
    {
      THROW_WEXCEPTION(L"File not found '{}'", filePath);
    }

    ioCtx.restart();

    boost::asio::write(stdInPipe, boost::asio::buffer(
      ToUtf8(filePath) + "\n-decimal\n-json\n-long\n-groupNames:0:1\n-echo4\n{readyErr}\n" + toCommandLineArgs(tagNames) + "-execute\n"));

    static const std::string readyStr = "{ready}\r\n";
    static const std::string readyErrStr = "{readyErr}\r\n";
    boost::asio::streambuf outBuf, errBuf;
    boost::asio::async_read_until(stdOutPipe, outBuf, readyStr, [&](auto, std::size_t) { });
    boost::asio::async_read_until(stdErrPipe, errBuf, readyErrStr, [&](auto, std::size_t) { });

    ioCtx.run();

    const std::string stdErr = std::string{ static_cast<const char*>(errBuf.data().data()), errBuf.size() - readyErrStr.size() };
    if (!stdErr.empty()) {
      THROW_WEXCEPTION(L"ExifTool.exe failed to get information from the file: '{}'. Error message: {}", filePath, ToWide(stdErr));
    }

    return { static_cast<const char*>(outBuf.data().data()), outBuf.size() - readyStr.size() };
  }

private:
  ExifToolWrapper() = default;

  std::string toCommandLineArgs(const std::vector<std::string>& tagNames) {
    std::string result;
    for (const auto& tag : tagNames) {
      result += std::format("-{}\n", tag);
    }
    return result;
  }

  boost::asio::io_context ioCtx;
  boost::asio::readable_pipe stdOutPipe{ ioCtx }, stdErrPipe{ ioCtx };
  boost::asio::writable_pipe stdInPipe{ ioCtx };
  boost::process::v2::process proc{
    ioCtx,
    boost::dll::this_line_location().parent_path() / L"exiftool" / L"exiftool(-k).exe",
    { "-stay_open", "true", "-@", "-" },
    boost::process::v2::process_stdio{ stdInPipe, stdOutPipe, stdErrPipe },
    boost::process::v2::windows::show_window_hide };
};
