# DOpus-Scripting-Extensions
A COM DLL that extends JScript functionality for [Directory Opus](https://www.gpsoft.com.au/) scripts.

# Limitations
* Supports Windows 10 x64 or higher.
* Supports only JScript scripting language. VBScript is not supported.
* Portable DOpus is not supported because the extensions DLL require installation using a `msi` installer.

# Description of classes
## ProcessRunner
This is an alternative to [WScript.shell Run](https://ss64.com/vb/run.html) method. It allows to get `StdOut` and `StdErr` of a process without creating a temporary file.<br>
`ProcessRunner` is 2-3 times faster than using the `WScript.shell`.

### Examples
#### Simple example
```javascript
// Acquire the COM object. It is recommended to acquire it only once and reuse it for performance reasons.
var processRunner = new ActiveXObject("DOpusScriptingExtensions.ProcessRunner")

// Run the executable with the specified arguments.
// The executable should be a full path.
// The arguments should be an array of strings.
var res = processRunner.Run("C:/Program Files/Git/usr/bin/cat.exe", ["C:/some_file"])

// The res variable is a `ProcessRunnerResult` object and can be used in the following way
WScript.Echo("Exit code: " + res.ExitCode)
WScript.Echo("Standart output: " + res.StdOut)
WScript.Echo("Standart error output: " + res.StdErr)
```
#### More examples of using `Run` method
```javascript
// Run an executable without arguments
res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe")
res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", [])

// Run an executable with many arguments
var font = "Consolas"
var caption = "test"
var destFile = "some name with spaces.png"
var paramsArray =
  [ "-background", "#ff5722", "-fill", "#000000", "-size", "227x47",
    "-pointsize", "22", "-gravity", "West", "-font", font,
    "caption:"+ caption, destFile ];
res = processRunner.Run("C:/Program Files/ImageMagick/magick.exe", paramsArray)

// Run an executable with working directory
res = processRunner.Run("C:/Program Files/Git/usr/bin/file.exe", ["twain_32.dll"], "C:/Windows")

// Use environment variables in the executable path
res = processRunner.Run("%comspec%", ["/c", "echo test"])
res = processRunner.Run("%ProgramW6432%/Git/usr/bin/echo.exe", ["test string"])
```
### Notes
* Arguments must be an array of strings.

## StringFormatter
This class allows you to format strings in a similar way to how it works in Python.<br>
It is an alternative to a traditional string concatenation approach used in JScript.<br>
It also allows to use format specifiers described in [format specification](https://en.cppreference.com/w/cpp/utility/format/spec).<br>
The implementation uses `std::format` from C++20.

### Examples

#### Simple example
```javascript
var fmt = new ActiveXObject("DOpusScriptingExtensions.StringFormatter")

// You can pass any object as an argument. JScript will convert it to a string
var resStr = fmt.Format("Some {} with {}", "message", "arguments")
WScript.Echo(resStr) // Some message with arguments
```

#### Using format specifiers
You can use any format specifiers that only apply to strings<br>
Because all parameters are converted to strings and then passed to the `Format` method.<br>
For example, the `{:X}` format specifier doesn't work
```javascript
var resStr = fmt.Format("Value with offset {:6}", 1234)
WScript.Echo(resStr) // Value with offset   1234
```

### Notes
The `Format` method accepts up to 12 arguments.

## FileMimeTypeDetector
This class allows you to detect the MIME type and encoding based on the file's content, not its extension. This class uses the [libmagic](https://man7.org/linux/man-pages/man3/libmagic.3.html), it is the same mechanism that the UNIX [file utility](https://man7.org/linux/man-pages/man1/file.1.html) uses.

### Examples
```javascript
// Acquire the COM object. It is recommended to acquire it only once and reuse it for performance reasons.
fileMimeTypeDetector = new ActiveXObject("DOpusScriptingExtensions.FileMimeTypeDetector")

var res = fileMimeTypeDetector.DetectMimeType("C:/some text file.txt")
WScript.Echo("MimeType: " + res.MimeType) // "text/plain"
WScript.Echo("Encoding: " + res.Encoding) // "utf-8"
```

### Notes
* The encoding doesn't show if a file has BOM or not.

# How to use
* Download the installer from the [latest release](https://github.com/PolarGoose/DOpus-Scripting-Extensions/releases) and install it on your system
* After that, you can access the functionality from any JScript or DOpus script.

# How to build
## Build requirements
* Visual Studio 2022 or higher
* [HeatWave for VS2022](https://marketplace.visualstudio.com/items?itemName=FireGiant.FireGiantHeatWaveDev17) plugin

## Build instructions
* Run `build.ps1` script as an admin.

# References
* Discussion of this project on DOpus forum: [DOpus-Scripting-Extensions project](https://resource.dopus.com/t/dopus-scripting-extensions-project-wild-idea/55000)
