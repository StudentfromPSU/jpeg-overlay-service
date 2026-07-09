#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/security/server_credentials.h>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "hello.grpc.pb.h"

class Server
{
public:
    Server(std::string server_address = "0.0.0.0:50051", std::string server_name = "Server", int max_connections = 10);

    ~Server();

    void Start();
    void Stop();

    bool TryAcquireConnection();
    void ReleaseConnection();

    void EnqueueProcessingTask(std::function<void()> task);

private:
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<grpc::ServerCompletionQueue> completion_queue_;
    std::unique_ptr<imageprocessor::ImageProcessor::AsyncService> async_service_;
    std::string server_address_;
    std::string server_name_;
    std::vector<std::thread> worker_threads_;
    std::vector<std::thread> processing_threads_;
    std::atomic<bool> shutdown_requested_{ false };

    uint32_t max_connections_;
    std::atomic<uint32_t> active_connections_{ 0 };

    std::queue<std::function<void()>> processing_tasks_;
    std::mutex processing_mutex_;
    std::condition_variable processing_cv_;
    std::atomic<bool> processing_shutdown_{ false };

    void HandleRpcs();
    void ProcessTasks();
    void RequestNewCall(imageprocessor::ImageProcessor::AsyncService* service);
};