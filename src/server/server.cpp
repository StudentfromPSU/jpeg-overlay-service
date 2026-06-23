#include "server.hpp"
#include <iostream>
#include <grpcpp/server_builder.h>
#include <stdexcept>
#include <thread>
#include <cstdint>
#include "hello.grpc.pb.h"

class CallData
{
public:
    CallData(helloworld::Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq, Server* server)
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
        try
        {
            switch (status_)
            {
                case CREATE:
                {
                    status_ = PROCESS;
                    service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, this);
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
                    reply_.set_message("Hello, " + request_.name());
                    status_ = FINISH;
                    responder_.Finish(reply_, grpc::Status::OK, this);
                    break;
                }

                case FINISH:
                {
                    delete this;
                    return;
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception in CallData::Proceed: " << e.what() << std::endl;
            if (status_ != FINISH)
            {
                status_ = FINISH;
                responder_.FinishWithError(
                    grpc::Status(grpc::StatusCode::INTERNAL, "Internal server error"),
                    this);
            }
        }
    }

private:
    enum CallStatus { CREATE, PROCESS, FINISH };
    helloworld::Greeter::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    Server* server_;
    grpc::ServerContext ctx_;
    helloworld::HelloRequest request_;
    helloworld::HelloReply reply_;
    grpc::ServerAsyncResponseWriter<helloworld::HelloReply> responder_;
    CallStatus status_;
    bool connection_acquired_;
};

Server::Server(std::string server_address, std::string server_name, int max_connections)
    : server_address_(std::move(server_address)), server_name_(std::move(server_name)), max_connections_(max_connections)
{
    if (max_connections <= 0)
    {
        throw std::invalid_argument("max_connections must be positive");
    }
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

    async_service_ = std::make_unique<helloworld::Greeter::AsyncService>();
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

    worker_threads_.reserve(num_workers);
    for (unsigned int i = 0; i < num_workers; ++i)
    {
        worker_threads_.emplace_back(&Server::HandleRpcs, this);
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

void Server::RequestNewCall(helloworld::Greeter::AsyncService* service)
{
    new CallData(service, completion_queue_.get(), this);
}
