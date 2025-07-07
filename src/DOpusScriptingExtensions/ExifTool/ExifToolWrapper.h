#pragma once

// When instance of this class is created it starts ExifTool.exe process in the background mode.
// In this mode ExifTool accepts commands via stdin and prints results to stdout or stderr in case of an error.
class ExifToolWrapper : boost::noncopyable {
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
  std::wstring GetTagInfosJson(const std::wstring_view filePath, const std::vector<std::string>& tagNames) {
    ExifToolCommandArgsGenerator argsGenerator(filePath);
    argsGenerator.AddCommandLineArgs({
      "-decimal",
      "-json",
      "-long",
      "-unknown",
      "-groupNames:0:1"
      });
    argsGenerator.AddTagNames(tagNames);
    return Execute(argsGenerator.GenerateExifToolInput());
  }

  std::wstring Run(const std::wstring_view filePath, const std::vector<std::string>& commandLineArgs) {
    ExifToolCommandArgsGenerator argsGenerator(filePath);
    argsGenerator.AddCommandLineArgs(commandLineArgs);
    return Execute(argsGenerator.GenerateExifToolInput());
  }

private:
  std::wstring Execute(const std::string_view exifToolCommands) {
    ioCtx.restart();

    boost::asio::write(stdInPipe, boost::asio::buffer(exifToolCommands));

    // After the write is completed, we need to read from stdOut and stdErr in parallel, to avoid ExifTool from deadlocking in case one of the pipes gets full.
    // When ExifTool is ready, it prints "{ready}\r\n" to stdOut and "{readyErr}\r\n" to stdErr (even if there are no errors).
    static const std::string readyStr = "{ready}\r\n";
    static const std::string readyErrStr = "{readyErr}\r\n";
    boost::asio::streambuf stdOutBuf;
    boost::asio::streambuf stdErrBuf;
    boost::asio::async_read_until(stdOutPipe, stdOutBuf, readyStr, [&](auto, std::size_t) {});
    boost::asio::async_read_until(stdErrPipe, stdErrBuf, readyErrStr, [&](auto, std::size_t) {});
    ioCtx.run();

    // If there is anything written to stdErr, it means that ExifTool failed to process the file.
    const auto& lengthOfErrorMessage = stdErrBuf.size() - readyErrStr.size(); // we ignore the "{readyErr}\r\n" part
    if(lengthOfErrorMessage != 0) {
      const auto& errorMessage = ToUtf16({ GetDataPointer(stdErrBuf), lengthOfErrorMessage });
      THROW_WEXCEPTION(L"ExifTool.exe failed to get information using the following parameters:\n{}\n\nError message:\n{}", ToUtf16(exifToolCommands), errorMessage);
    }
    
    const auto& lengthOfStdOutput = stdOutBuf.size() - readyStr.size(); // we ignore the "{ready}\r\n" part
    return ToUtf16({ GetDataPointer(stdOutBuf), lengthOfStdOutput });
  }

  boost::asio::io_context ioCtx;
  boost::asio::readable_pipe stdOutPipe{ ioCtx };
  boost::asio::readable_pipe stdErrPipe{ ioCtx };
  boost::asio::writable_pipe stdInPipe{ ioCtx };
  boost::process::v2::process proc{
    ioCtx,
    boost::dll::this_line_location().parent_path() / L"exiftool" / L"exiftool.exe",
    { "-stay_open", "true", "-@", "-" },
    boost::process::v2::process_stdio{ stdInPipe, stdOutPipe, stdErrPipe },
    boost::process::v2::windows::show_window_hide };
};
