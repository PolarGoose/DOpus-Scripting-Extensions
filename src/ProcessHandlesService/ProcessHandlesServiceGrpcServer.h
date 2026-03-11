#pragma once

#include <gen/ProcessHandlesService.pb.h>
#include <gen/ProcessHandlesService.grpc.pb.h>
#include "ProcessHandlesService/LockedFilesProvider.h"

class ProcessHandlesServiceGrpc final : public ProcessHandlesService::ProcessHandlesServiceGrpc::Service {
  LockedFilesProvider _lockedFilesProvider;

public:
  grpc::Status GetLockingProcessInfos(grpc::ServerContext* /* context */,
                                      const google::protobuf::Empty* /* request */,
                                      ProcessHandlesService::LockingProcessInfos* response) override {
    try {
      _lockedFilesProvider.GetLockingProcessInfos(*response);
      return grpc::Status::OK;
    }
    catch (const std::exception& ex) {
      return grpc::Status(grpc::StatusCode::INTERNAL, ex.what());
    }
  }
};
