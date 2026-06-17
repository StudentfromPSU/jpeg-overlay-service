#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>

#include "hello.grpc.pb.h"
#include "hello.pb.h"
#include "server/interceptors.hpp"

namespace hw = helloworld;

using grpc::ServerContext;
using grpc::Status;

class HelloService final : public hw::Greeter::Service
{
public:
    explicit HelloService(std::shared_ptr<ConnectionLimiter> limiter = nullptr);

    Status SayHello(ServerContext* context, const hw::HelloRequest* request, hw::HelloReply* response) override;

private:
    std::shared_ptr<ConnectionLimiter> limiter_;
};