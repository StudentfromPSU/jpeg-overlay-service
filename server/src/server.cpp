#include "server.hpp"
#include <iostream>
#include <grpcpp/server_builder.h>
#include <stdexcept>
#include <thread>
#include <cstdint>
#include "hello.grpc.pb.h"
#include "image_processor/ImageProcessor.h"
#include "image_processor/Exceptions.h"

class CallData
{
public:
    CallData(imageprocessor::ImageProcessor::AsyncService* service, grpc::ServerCompletionQueue* cq, Server* server)
        : service_(service), cq_(cq), server_(server), responder_(&ctx_), status_(CREATE), connection_acquired_(false)
    {
        Proceed(true);
    }

    ~CallData()
    {
        if (connection_acquired_)
        {
            server_->ReleaseConnection();
        }
    }

    void Proceed(bool ok)
    {
        switch (status_)
        {
        case CREATE:
        {
            status_ = PROCESS;
            service_->RequestProcessImage(&ctx_, &request_, &responder_, cq_, cq_, this);
            break;
        }

        case PROCESS:
        {
            if (!ok)
            {
                delete this;
                return;
            }

            new CallData(service_, cq_, server_);

            if (!server_->TryAcquireConnection())
            {
                std::cout << "Connection limit reached, rejecting request" << std::endl;
                status_ = FINISH;
                responder_.FinishWithError(
                    grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Server at max connections"), this);
                return;
            }

            connection_acquired_ = true;

            try
            {
                ImageProcessor::ImageProcessor processor;
                std::vector<unsigned char> imageBytes(request_.image().begin(), request_.image().end());
                std::vector<unsigned char> processedImage = processor.Process(imageBytes, request_.text());
                reply_.set_image(processedImage.data(), processedImage.size());
                status_ = FINISH;
                responder_.Finish(reply_, grpc::Status::OK, this);
            }
            catch (const ImageProcessor::ImageException& e)
            {
                std::cerr << "Image processing error: " << e.what() << std::endl;
                status_ = FINISH;
                responder_.FinishWithError(
                    grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, e.what()), this);
            }
            catch (const std::exception& e)
            {
                std::cerr << "Unexpected error: " << e.what() << std::endl;
                status_ = FINISH;
                responder_.FinishWithError(
                    grpc::Status(grpc::StatusCode::INTERNAL, "Internal server error"), this);
            }
            break;
        }

        case FINISH:
        {
            delete this;
            return;
        }
        }
    }

private:
    imageprocessor::ImageProcessor::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    Server* server_;
    grpc::ServerContext ctx_;
    imageprocessor::ImageRequest request_;
    imageprocessor::ImageResponse reply_;
    grpc::ServerAsyncResponseWriter<imageprocessor::ImageResponse> responder_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;
    bool connection_acquired_;
};

Server::Server(std::string server_address, std::string server_name, int max_connections)
    : server_address_(std::move(server_address)), server_name_(std::move(server_name)), max_connections_(max_connections)
{
    assert(("max_connections must be positive", max_connections > 0));
}

Server::~Server()
{
    Stop();
}

bool Server::TryAcquireConnection()
{
    uint32_t current = active_connections_.load(std::memory_order_relaxed);
    while (current < max_connections_)
    {
        if (active_connections_.compare_exchange_strong(current, current + 1, std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            return true;
        }
    }
    return false;
}

void Server::ReleaseConnection()
{
    active_connections_.fetch_sub(1, std::memory_order_release);
}

void Server::Start()
{
    grpc::ServerBuilder builder;

    builder.AddListeningPort(this->server_address_, grpc::InsecureServerCredentials());

    completion_queue_ = builder.AddCompletionQueue();

    async_service_ = std::make_unique<imageprocessor::ImageProcessor::AsyncService>();
    builder.RegisterService(async_service_.get());

    server_ = builder.BuildAndStart();
    if (!server_)
    {
        throw std::runtime_error(std::string("Failed to start server on ") + server_address_);
    }

    std::cout << this->server_name_ << " listening on " << this->server_address_ << std::endl;
    std::cout << "Max connections: " << max_connections_ << std::endl;
    std::cout << "Async server ready to accept connections" << std::endl;

    RequestNewCall(async_service_.get());

    const unsigned int hardware_threads = std::thread::hardware_concurrency();
    const unsigned int num_workers = std::max(1u, hardware_threads);

    std::cout << "Available CPU cores: " << hardware_threads
        << ", spawning " << num_workers
        << " worker threads" << std::endl;

    worker_threads_.resize(num_workers);

    for (auto& thread : worker_threads_)
    {
        thread = std::thread(&Server::HandleRpcs, this);
    }
}

void Server::Stop()
{
    bool expected = false;
    if (!shutdown_requested_.compare_exchange_strong(expected, true, std::memory_order_acq_rel, std::memory_order_acquire))
    {
        return;
    }

    if (!server_) return;

    std::cout << this->server_name_ << " shutting down, please wait..." << std::endl;

    server_->Shutdown();

    if (completion_queue_)
    {
        completion_queue_->Shutdown();
    }

    for (auto& t : worker_threads_)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    std::cout << this->server_name_ << " shutdown complete" << std::endl;
}

void Server::HandleRpcs()
{
    void* tag;
    bool ok;

    while (completion_queue_->Next(&tag, &ok))
    {
        if (!tag)
        {
            continue;
        }

        CallData* call = static_cast<CallData*>(tag);
        call->Proceed(ok);
    }
}

void Server::RequestNewCall(imageprocessor::ImageProcessor::AsyncService* service)
{
    new CallData(service, completion_queue_.get(), this);
}
