function assertionFailed(message) {
  throw new Error("Assertion failed:\n" + message)
}

function assertStringEmpty(string) {
  if (string != "") {
    assertionFailed("Expected empty string, but got:\n" + string)
  }
}

function assertEqual(actual, expected) {
  if (actual != expected) {
    assertionFailed("actual=\n'" + actual + "'\nnot equal to expected\n'" + expected + "'")
  }
}

function assertGreater(a, b) {
  if (a > b) {
    assertionFailed("a='" + a + "' is not greater than b='" + b + "'")
  }
}

function assertLess(a, b) {
  if (a < b) {
    assertionFailed("a='" + a + "' is not less than b='" + b + "'")
  }
}

function assertStringContains(string, substring) {
  if (string.indexOf(substring) == -1) {
    assertionFailed("Expected string to contain substring\n'" + substring + "'\nbut it did not. Full string:\n'" + string + "'")
  }
}

function assertThrows(fn) {
  try {
    fn()
  } catch (e) {
    return e.message
  }
  assertionFailed("Expected exception, but no exception was thrown")
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

function getFunctionName(fn) {
  var match = fn.toString().match(/^function\s+([\w\$]+)\s*\(/)
  return match ? match[1] : "anonymous"
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

var processRunner
try {
  processRunner = new ActiveXObject("DOpusScriptingExtensions.ProcessRunner")
} catch (e) {
  WScript.Echo("Failed to get ProcessRunner object:\n" + e.message)
  WScript.Quit(1)
}

function ProcessRunner_can_run_an_executable_without_parameters() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", [])
  assertEqual(res.ExitCode, 0)
}

function ProcessRunner_can_run_an_executable_that_print_to_stdErr() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/cat.exe", ["C:/non_existent_file"])
  assertStringEmpty(res.StdOut)
  assertEqual(res.StdErr, "/usr/bin/cat: 'C:/non_existent_file': No such file or directory\n")
  assertEqual(res.ExitCode, 1)
}

function ProcessRunner_can_run_an_executable_with_parameters() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", ["arg1_трじα", "arg2_трじα"])
  assertEqual(res.StdOut, "arg1_трじα arg2_трじα\n")
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

function ProcessRunner_can_run_a_file_exe() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/file.exe", ["--mime-type", "--brief", "C:/Windows/regedit.exe"])
  assertEqual(res.StdOut, "application/vnd.microsoft.portable-executable\n")
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

function ProcessRunner_can_run_a_powershell_command() {
  var res = processRunner.Run("C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe", ["-ExecutionPolicy", "Bypass", "-Command", "[System.Reflection.AssemblyName]::GetAssemblyName('C:/Windows/regedit.exe')"])
  assertStringEmpty(res.StdOut)
  assertStringContains(res.StdErr, "Could not load file or assembly")
  assertEqual(res.ExitCode, 1)
}

function ProcessRunner_can_run_a_file_exe() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/file.exe", ["--mime-type", "--brief", "C:/Windows/regedit.exe"])
  assertEqual(res.StdOut, "application/vnd.microsoft.portable-executable\n")
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

function ProcessRunner_can_receive_very_big_std_input() {
  var filePath = createTextFile(10)
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/cat.exe", [filePath])
  assertEqual(res.StdOut.length, 10 * 1024 * 1024)
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

function ProcessRunner_fails_if_exe_does_not_exist() {
  var exeptionMessage = assertThrows(function() { processRunner.Run("C:/non_existent_executable", []) })
  assertStringContains(exeptionMessage, "CProcessRunner::RunProcess:")
  assertStringContains(exeptionMessage, "The executable not found 'C:/non_existent_executable'. HRESULT=0x80")
  assertStringContains(exeptionMessage, "could not be found.")
}

function ProcessRunner_fails_if_argument_has_wrong_format() {
  var exeptionMessage = assertThrows(function() { processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", [1]) })
  assertStringContains(exeptionMessage, "JsStringArrayToVector:")
  assertStringContains(exeptionMessage, "the element with the index 0 is not a string but a variant type #3. HRESULT=0x80020005: Type mismatch.")
}

function ProcessRunner_can_run_cmd_exe_with_pipe() {
  var res = processRunner.Run("C:/WINDOWS/system32/cmd.exe", ["/c", "echo test string| findstr test"])
  assertEqual(res.StdOut, "test string\r\n")
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

function ProcessRunner_can_run_with_env_variable() {
  var res = processRunner.Run("%comspec%", ["/c", "echo test string"])
  assertEqual(res.StdOut, "test string\r\n")
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)

  var res = processRunner.Run("%ProgramW6432%/Git/usr/bin/echo.exe", ["test string"])
  assertEqual(res.StdOut, "test string\n")
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

function ProcessRunner_fails_if_env_variable_does_not_exist() {
  var exeptionMessage = assertThrows(function () { processRunner.Run("%nonExistentEnvVar%", []) })
  assertStringContains(exeptionMessage, "The executable not found '%nonExistentEnvVar%'")
}

function ProcessRunner_if_wrong_type_is_passed_it_ignores_it() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", ProcessRunner_if_wrong_type_is_passed_it_ignores_it)
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

function ProcessRunner_fails_if_the_exe_is_not_executable() {
  var exeptionMessage = assertThrows(function () { processRunner.Run("C:/Program Files", []) })
  assertStringContains(exeptionMessage, "The file 'C:/Program Files' is not executable")

  var exeptionMessage = assertThrows(function () { processRunner.Run("C:/Windows/System32/mssecuser.dll", []) })
  assertStringContains(exeptionMessage, "The file 'C:/Windows/System32/mssecuser.dll' is not executable.")
}

function ProcessRunner_can_specify_working_directory() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/file.exe", ["--mime-type", "--brief", "regedit.exe"], "C:/Windows")
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

var fileMimeTypeDetector
try {
  fileMimeTypeDetector = new ActiveXObject("DOpusScriptingExtensions.FileMimeTypeDetector")
} catch (e) {
  WScript.Echo("Failed to get FileMimeTypeDetector object:\n" + e.message)
}

function FileMimeTypeDetector_works_with_binary_files() {
  var res = fileMimeTypeDetector.DetectMimeType("C:/Program Files/Git/usr/bin/echo.exe")
  assertEqual(res.MimeType, "application/vnd.microsoft.portable-executable")
  assertEqual(res.Encoding, "binary")
}

function FileMimeTypeDetector_works_with_directories() {
  var res = fileMimeTypeDetector.DetectMimeType("C:/Program Files")
  assertEqual(res.MimeType, "inode/directory")
  assertEqual(res.Encoding, "binary")
}

function FileMimeTypeDetector_fails_if_fail_doesn_not_exist() {
  var exeptionMessage = assertThrows(function () { fileMimeTypeDetector.DetectMimeType("C:/non_existent_file") })
  assertStringContains(exeptionMessage, "CFileMimeTypeDetector::DetectMimeType:")
  assertStringContains(exeptionMessage, "The file not found 'C:/non_existent_file'")
}

runTest(ProcessRunner_can_run_an_executable_without_parameters)
runTest(ProcessRunner_can_run_an_executable_that_print_to_stdErr)
runTest(ProcessRunner_can_run_an_executable_with_parameters)
runTest(ProcessRunner_can_run_a_file_exe)
runTest(ProcessRunner_can_run_a_powershell_command)
runTest(ProcessRunner_can_receive_very_big_std_input)
runTest(ProcessRunner_fails_if_exe_does_not_exist)
runTest(ProcessRunner_fails_if_argument_has_wrong_format)
runTest(ProcessRunner_can_run_cmd_exe_with_pipe)
runTest(ProcessRunner_can_run_with_env_variable)
runTest(ProcessRunner_fails_if_env_variable_does_not_exist)
runTest(ProcessRunner_if_wrong_type_is_passed_it_ignores_it)
runTest(ProcessRunner_fails_if_the_exe_is_not_executable)
runTest(ProcessRunner_can_specify_working_directory)

runTest(FileMimeTypeDetector_works_with_binary_files)
runTest(FileMimeTypeDetector_works_with_directories)
runTest(FileMimeTypeDetector_fails_if_fail_doesn_not_exist)
