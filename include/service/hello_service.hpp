#pragma once

#include <mutex>


#include <grpcpp/grpcpp.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include <grpcpp/server_interface.h>

#include <google/protobuf/map.h>

#include "hello.grpc.pb.h"
#include "hello.pb.h"

namespace hw = helloworld;

using grpc::ServerContext;
using grpc::Status;

class HelloService final : public hw::Greeter::Service
{
public:
    HelloService();

    Status SayHello(ServerContext* context, const hw::HelloRequest* request, hw::HelloReply* response) override;

};