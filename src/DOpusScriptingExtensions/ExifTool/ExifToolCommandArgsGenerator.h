#pragma once

// Generates the command line arguments string that can be written to the ExifTool stdin pipe.
// ExifTool requires so that each argument should be separated by a newline character.
class ExifToolCommandArgsGenerator : boost::noncopyable {
public:
  ExifToolCommandArgsGenerator(const std::wstring_view filePath) {
    if (!std::filesystem::exists(filePath)) {
      THROW_WEXCEPTION(L"File not found '{}'", filePath);
    }

    if (std::filesystem::is_directory(filePath)) {
      THROW_WEXCEPTION(L"File is a directory '{}'", filePath);
    }

    commandLineArgs = ToUtf8(filePath) + "\n";
  }

  void AddTagNames(const std::vector<std::string>& tagNames) {
    for (const auto& tagName : tagNames) {
      // ExifTags must be in the format "Group0:TagName"
      static const std::regex exifTagFormat(R"(^[A-Za-z0-9_-]+:[A-Za-z0-9_-]+$)");

      if (!std::regex_match(tagName, exifTagFormat)) {
        THROW_WEXCEPTION(L"ExifTool tag name '{}' is incorrect. The tag name should be in format 'Group0:TagName', for example 'AIFF:FormatVersionTime'", ToUtf16(tagName));
      }

      commandLineArgs += std::format("-{}\n", tagName);
    }
  }

  void AddCommandLineArgs(const std::vector<std::string>& args) {
    for(const auto& arg : args) {
      commandLineArgs += arg + "\n";
    }
  }

  // Returns a string like:
  //   "C:\\file.jpg\n-decimal\n-json\n-long\n-unknown\n-groupNames:0:1\n-tag1\n-tag2\n-echo4\n{readyErr}\n-execute\n"
  std::string GenerateExifToolInput() const {
    return commandLineArgs + "-echo4\n{readyErr}\n-execute\n";
  }

private:
  std::string commandLineArgs;
};
