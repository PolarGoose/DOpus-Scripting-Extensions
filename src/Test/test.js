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

function GetComObject(name) {
  try {
    return new ActiveXObject(name)
  } catch (e) {
    WScript.Echo("Failed to get COM object '" + name + "':\n" + e.message)
    WScript.Quit(1)
  }
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

var processRunner = GetComObject("DOpusScriptingExtensions.ProcessRunner")

function ProcessRunner_can_run_an_executable_without_parameters() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", [])
  assertEqual(res.ExitCode, 0)

  var res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe")
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
  var exeptionMessage = assertThrows(function() { processRunner.Run("C:/non_existent_executable") })
  assertStringContains(exeptionMessage, "ProcessRunner.h:")
  assertStringContains(exeptionMessage, "The executable not found 'C:/non_existent_executable'")
}

function ProcessRunner_fails_if_argument_has_wrong_format() {
  var exeptionMessage = assertThrows(function() { processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", [1]) })
  assertStringContains(exeptionMessage, "ComUtils.h:")
  assertStringContains(exeptionMessage, "the element with the index 0 is not a string but a variant type #3")
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
  var exeptionMessage = assertThrows(function () { processRunner.Run("%nonExistentEnvVar%") })
  assertStringContains(exeptionMessage, "The executable not found '%nonExistentEnvVar%'")
}

function ProcessRunner_if_wrong_type_is_passed_it_ignores_it() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/echo.exe", ProcessRunner_if_wrong_type_is_passed_it_ignores_it)
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

function ProcessRunner_fails_if_the_exe_is_not_executable() {
  var exeptionMessage = assertThrows(function () { processRunner.Run("C:/Program Files") })
  assertStringContains(exeptionMessage, "ProcessRunner.h:")
  assertStringContains(exeptionMessage, "The file 'C:/Program Files' is not executable")

  var exeptionMessage = assertThrows(function () { processRunner.Run("C:/Windows/System32/mssecuser.dll") })
  assertStringContains(exeptionMessage, "ProcessRunner.h:")
  assertStringContains(exeptionMessage, "The file 'C:/Windows/System32/mssecuser.dll' is not executable")
}

function ProcessRunner_can_specify_working_directory() {
  var res = processRunner.Run("C:/Program Files/Git/usr/bin/file.exe", ["--mime-type", "--brief", "regedit.exe"], "C:/Windows")
  assertStringEmpty(res.StdErr)
  assertEqual(res.ExitCode, 0)
}

var fileMimeTypeDetector = GetComObject("DOpusScriptingExtensions.FileMimeTypeDetector")

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
  assertStringContains(exeptionMessage, "FileMimeTypeDetector.h:")
  assertStringContains(exeptionMessage, "The file not found 'C:/non_existent_file'")
}

function FileMimeTypeDetector_works_with_path_containing_unicode_characters() {
  var fso = new ActiveXObject("Scripting.FileSystemObject");
  var tempFolder = fso.GetSpecialFolder(2);
  var filePath = tempFolder.Path + "/DOpusScriptingExtensions testTextFile трじα.txt";
  var file = fso.CreateTextFile(filePath, true);
  file.Write("AAAA\n");
  file.Close();

  var res = fileMimeTypeDetector.DetectMimeType(filePath)
  assertEqual(res.MimeType, "text/plain")
  assertEqual(res.Encoding, "us-ascii")
}

var stringFormatter = GetComObject("DOpusScriptingExtensions.StringFormatter")

function StringFormatter_works_without_parameters() {
  var res = stringFormatter.Format("Test message");
  assertEqual(res, "Test message");
}

function StringFormatter_a_parameter_can_be_passed() {
  var res = stringFormatter.Format("Test message {}", 123);
  assertEqual(res, "Test message 123");
}

function StringFormatter_values_are_converted_to_string_and_passed_into_Format_method() {
  var res = stringFormatter.Format("Test message {} {}", 123, [1, 2, 3]);
  assertEqual(res, "Test message 123 1,2,3");
}

function StringFormatter_test_extra_parameter_are_ignored() {
  var res = stringFormatter.Format("Test message {}", 123, [1, 2, 3]);
  assertEqual(res, "Test message 123");
}

function StringFormatter_can_accept_12_parameters() {
  var res = stringFormatter.Format("Test message {}{}{}{}{}{}{}{}{}{}{}{}",
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
  assertEqual(res, "Test message 123456789101112");
}

function StringFormatter_format_with_specifiers() {
  var res = stringFormatter.Format("Test message {:>10}", "message");
  assertEqual(res, "Test message    message");
}

function StringFormatter_throws_if_invalid_specifier() {
  var res = assertThrows(function () {
    stringFormatter.Format("Test message {:X}", 1);
  });
  assertEqual(res, "Invalid presentation type for string");
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
runTest(FileMimeTypeDetector_works_with_path_containing_unicode_characters)

runTest(StringFormatter_works_without_parameters);
runTest(StringFormatter_a_parameter_can_be_passed);
runTest(StringFormatter_values_are_converted_to_string_and_passed_into_Format_method);
runTest(StringFormatter_test_extra_parameter_are_ignored);
runTest(StringFormatter_can_accept_12_parameters);
runTest(StringFormatter_format_with_specifiers);
runTest(StringFormatter_throws_if_invalid_specifier);
