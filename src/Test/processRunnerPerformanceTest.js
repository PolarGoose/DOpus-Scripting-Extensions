var fso = new ActiveXObject("Scripting.FileSystemObject");
var shell = new ActiveXObject("WScript.shell");
var processRunner = new ActiveXObject("DOpusScriptingExtensions.ProcessRunner")

function getFunctionName(fn) {
  var match = fn.toString().match(/^function\s+([\w\$]+)\s*\(/)
  return match ? match[1] : "anonymous"
}

function createTextFile(sizeInMb) {
  var fso = new ActiveXObject("Scripting.FileSystemObject");
  var tempFolder = fso.GetSpecialFolder(2);
  var filePath = tempFolder.Path + "/DOpusScriptingExtensions_testTextFile.txt";
  var file = fso.CreateTextFile(filePath, true);

  var oneKB = "";
  for (var i = 0; i < 1023; i++) {
    oneKB += "A";
  }
  oneKB += "\n";

  for (var i = 0; i < sizeInMb * 1024; i++) {
      file.Write(oneKB);
  }

  file.Close();
  return filePath;
}

function runTest(testFunction) {
  try {
    WScript.Echo("\nRunning test: " + getFunctionName(testFunction))
    testFunction()
    WScript.Echo("Pass")
  } catch (e) {
    WScript.Echo("FAIL:\n" + e.message)
    WScript.Quit(1)
  }
}

function runFunctionAndMeasureTime(func) {
  var startTime = new Date()
  func()
  return new Date() - startTime
}

function testThatPerformanceIsBetter(wshellFunc, processRunnerFunc) {
  WScript.Echo("Run using WScript.shell")
  var wshellTime = runFunctionAndMeasureTime(wshellFunc)
  WScript.Echo("Run using ProcessRunner")
  var processRunnerTime = runFunctionAndMeasureTime(processRunnerFunc)

  WScript.Echo("Result:\nWScript.shell: " + wshellTime + "ms\nProcessRunner: " + processRunnerTime + "ms")

  if(wshellTime < processRunnerTime) {
    throw new Error("ProcessRunner is slower")
  }
}

function runCommandUsingWShellAndReturnOutput(command, args) {
  var tempFileFullName = fso.GetSpecialFolder(2) + "\\DOpusScriptingExtensions_ProcessRunner_performance_test.txt"
  var cmdLine = 'cmd.exe /c ""' + command + '" ' + args + '  > "'+ tempFileFullName + '""'
  WScript.Echo("shell.run " + cmdLine)

  try {
    var exitCode = shell.run(cmdLine, 0, true)
    if (exitCode !== 0) {
      throw new Error("Failed to execute the command. ExitCode=" + exitCode)
    }

    var handle = fso.OpenTextFile(tempFileFullName, 1);
    var content = handle.ReadAll();
    handle.Close()
    return content
  }
  finally {
    fso.DeleteFile(tempFileFullName)
  }
}

function ProcessRunner_test_executable_with_small_output() {
  testThatPerformanceIsBetter(
    function() {
      runCommandUsingWShellAndReturnOutput("C:/Program Files/Git/usr/bin/echo.exe",  "123")
    },
    function() {
      processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", ["123"])
    }
  )
}

function ProcessRunner_test_executable_with_1mb_output() {
  var file = createTextFile(1)

  testThatPerformanceIsBetter(
    function() {
      runCommandUsingWShellAndReturnOutput("C:/Program Files/Git/usr/bin/cat.exe",  file)
    },
    function() {
      processRunner.Run("C:/Program Files/Git/usr/bin/cat.exe", [file])
    }
  )
}

function ProcessRunner_test_executable_with_30mb_output() {
  var file = createTextFile(30)

  testThatPerformanceIsBetter(
    function() {
      runCommandUsingWShellAndReturnOutput("C:/Program Files/Git/usr/bin/cat.exe",  file)
    },
    function() {
      processRunner.Run("C:/Program Files/Git/usr/bin/cat.exe", [file])
    }
  )
}

runTest(ProcessRunner_test_executable_with_small_output)
runTest(ProcessRunner_test_executable_with_1mb_output)
runTest(ProcessRunner_test_executable_with_30mb_output)
