#include "pch.h"
#include <doctest/doctest.h>
#include "ProcessHandlesService/ProcessHandlesServiceGrpcImpl.h"

static void AssertContainsProcess(const ProcessHandlesService::LockingProcessInfos& lockingProcessInfos,
                                  std::function<bool(const ProcessHandlesService::ProcessInfo&)> predicate,
                                  const std::string_view errorMessage) {
  for (const auto& [pid, processInfo] : lockingProcessInfos.process_infos()) {
    if (predicate(processInfo)) {
      return;
    }
  }
  FAIL(errorMessage);
}

static bool ContainsElement(const google::protobuf::RepeatedPtrField<std::string>& array, const std::string& element) {
  for (const auto& item : array) {
    if (item == element) {
      return true;
    }
  }
  return false;
}

TEST_CASE("ProcessHandlesServiceGrpcServer test") {
  ProcessHandlesServiceGrpcImpl processHandlesServiceGrpcImpl;

  grpc::ServerBuilder builder;
  int selectedPort = 0;
  builder.AddListeningPort("127.0.0.1:0", grpc::InsecureServerCredentials(), &selectedPort);
  builder.RegisterService(&processHandlesServiceGrpcImpl);
  const auto grpcServer = builder.BuildAndStart();

  auto client = ProcessHandlesService::ProcessHandlesServiceGrpc::NewStub(grpc::CreateChannel("127.0.0.1:" + std::to_string(selectedPort), grpc::InsecureChannelCredentials()));

  ProcessHandlesService::LockingProcessInfos response;
  grpc::ClientContext context;

  auto start = std::chrono::high_resolution_clock::now();
  const auto& status = client->GetLockingProcessInfos(&context, google::protobuf::Empty(), &response);
  std::cout << std::format("GetLockingProcessInfos time taken: {}ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count());

  REQUIRE(status.ok());
  REQUIRE(response.process_infos_size() > 0);
  REQUIRE(response.process_infos().contains(4)); // Contains "System" process

  AssertContainsProcess(response,
    [](const ProcessHandlesService::ProcessInfo& info) { return info.executable_full_name() == "C:\\Windows\\System32\\backgroundTaskHost.exe"; },
    "Should have process 'C:\\Windows\\System32\\backgroundTaskHost.exe'");

  AssertContainsProcess(response,
    [](const ProcessHandlesService::ProcessInfo& info) { return info.domain_name() == "NT AUTHORITY"; },
    "Should have process with domain_name = 'NT AUTHORITY'");

  AssertContainsProcess(response,
    [](const ProcessHandlesService::ProcessInfo& info) { return info.user_name() == "SYSTEM"; },
    "Should have process with user_name = 'SYSTEM'");

  AssertContainsProcess(response,
    [](const ProcessHandlesService::ProcessInfo& info) { return ContainsElement(info.modules(), "C:\\WINDOWS\\SYSTEM32\\ntdll.dll"); },
    "Should contain module 'C:\\WINDOWS\\SYSTEM32\\ntdll.dll'");

  AssertContainsProcess(response,
    [](const ProcessHandlesService::ProcessInfo& info) { return ContainsElement(info.locked_files(), "C:\\Windows\\System32\\en-US\\svchost.exe.mui"); },
    "Should contain locked file 'C:\\Windows\\System32\\en-US\\svchost.exe.mui'");

  AssertContainsProcess(response,
    [](const ProcessHandlesService::ProcessInfo& info) { return ContainsElement(info.locked_files(), "C:\\Windows\\System32\\"); },
    "Should contain locked folder 'C:\\Windows\\System32\\'");
}
