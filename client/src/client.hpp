#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <grpcpp/grpcpp.h>
#include "hello.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using imageprocessor::ImageProcessor;
using imageprocessor::ImageRequest;
using imageprocessor::ImageResponse;

enum class ProcessResult
{
    Success,
    ServerBusy,
    RpcError
};

struct ClientArguments
{
    std::string inputPath;
    std::string outputDirectory;
    std::string text;
    int retryTimeoutMs;
};

class ImageProcessorClient
{
public:
    explicit ImageProcessorClient(std::shared_ptr<Channel> channel);

    ProcessResult ProcessImage(
        const std::vector<uint8_t>& imageData,
        const std::string& text,
        std::string& processedImageOut);

private:
    std::unique_ptr<ImageProcessor::Stub> stub_;
};

class ClientApplication
{
public:
    ClientApplication(
        const ClientArguments& args,
        std::shared_ptr<ImageProcessorClient> grpcClient);

    int Run();

private:
    ClientArguments args_;
    std::shared_ptr<ImageProcessorClient> grpc_client_;
};
