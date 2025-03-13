# DOpus-Scripting-Extensions
A COM DLL that extends JScript functionality for [Directory Opus](https://www.gpsoft.com.au/) scripts.

# Current status
Currently, this project is a proof of concept.<br>
There is only a `ProcessRunner` class.<br>
However, the provided functionality works properly and can be used.

# Limitations
* It was tested on Windows 11 but should also work on Windows 7 and higher.
* Supports only Windows x64 bit
* Supports only JScript scripting language. VBScript is not supported.
* Portable DOpus is not supported because the extensions DLL require installation using a `msi` installer.

# Description of classes
## ProcessRunner
This is an improved version of the [WScript.shell Run](https://ss64.com/vb/run.html) method. It allows to get StdOur and StdErr of the process without creating a temporary file.

### Example
```
// Acquire the COM object. It is recommended to acquire it only once and reuse it for performance reasons.
var processRunner = new ActiveXObject("DOpusScriptingExtensions.ProcessRunner")

// Run the executable with the specified arguments.
// The executable should be a full path.
// The arguments should be an array of strings.
var res = processRunner.Run("C:/Program Files/Git/usr/bin/cat.exe", ["C:/some_file"])
res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", []) // This will run the executable without any arguments
res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", ["--arg1", "--arg2"])
// res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", [1]) // This will throw an exception. The arguments must be strings

// The res variable is a `ProcessRunnerResult` object and can be used in the following way
WScript.Echo("Exit code: " + res.ExitCode)
WScript.Echo("Standart output: " + res.StdOut)
WScript.Echo("Standart error output: " + res.StdErr)
```

# How to use
* Download the installer from the [latest release](https://github.com/PolarGoose/DOpus-Scripting-Extensions/releases) and install it on your system
* After that you can access the functionality from any JScript and DOpus script.

# How to build
## Build requirements
* Visual Studio 2022 or higher
* [HeatWave for VS2022](https://marketplace.visualstudio.com/items?itemName=FireGiant.FireGiantHeatWaveDev17) plugin

## Build instructions
* Run `build.ps1` script as an admin.
