#pragma once

class ProcessHandlesRemoteItfServer final : public ProcessHandlesService::ProcessHandlesServiceGrpc::Service {

public:
  explicit ProcessHandlesRemoteItfServer(LockedFilesProvider& provider = {})
    : _provider(std::move(provider)) {
  }

  grpc::Status GetAllLockedFiles(grpc::ServerContext* context,
                                 const google::protobuf::Empty* request,
                                 ProcessHandlesService::LockedFilesAndLockedProcessInfos* response) override {
    if (!response) {
      return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "response is null");
    }

    try {
      // If not thread-safe, wrap with a lock:
      // std::scoped_lock lk(_mtx);

      _provider.GetAllLockedFiles(*response);
      return grpc::Status::OK;
    }
    catch (const std::exception& e) {
      return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
    catch (...) {
      return grpc::Status(grpc::StatusCode::INTERNAL, "unknown error");
    }
  }

private:
  LockedFilesProvider _provider;

  // If your driver/NtDll objects are not thread-safe, uncomment:
  // std::mutex _mtx;
};
