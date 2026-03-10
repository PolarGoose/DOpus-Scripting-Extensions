#include "pch.h"
#include <doctest/doctest.h>
#include "ProcessHandlesService/LockedFilesProvider.h"

TEST_CASE("collects only file handles with a resolvable path") {
  LockedFilesProvider lockedFilesProvider;
  ProcessHandlesService::LockedFilesAndProcessInfos result;

  auto start = std::chrono::high_resolution_clock::now();
  lockedFilesProvider.GetAllLockedFiles(result);
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
  std::cout << std::format("Time taken: {}ms\n", duration);

  REQUIRE(result.locked_files().size() > 0);
}
