#include "server.hpp"
#include <iostream>
#include <grpcpp/server_builder.h>
#include <stdexcept>
//#include <thread>
//#include <chrono>

enum CallStatus { CREATE, PROCESS, FINISH };

class CallData
{
public:
    CallData(helloworld::Greeter::AsyncService* service,
             grpc::ServerCompletionQueue* cq,
             Server* server)
        : service_(service), cq_(cq), server_(server), responder_(&ctx_), status_(CREATE), connection_acquired_(false)
    {
        Proceed();
    }

    ~CallData()
    {
        if (connection_acquired_)
        {
            server_->ReleaseConnection();
        }
    }

    void Proceed()
    {
        if (status_ == CREATE)
        {
            status_ = PROCESS;
            service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, this);
        }
        else if (status_ == PROCESS)
        {
            new CallData(service_, cq_, server_);

            if (!server_->TryAcquireConnection())
            {
                std::cout << "Connection limit reached, rejecting request" << std::endl;
                status_ = FINISH;
                responder_.FinishWithError(
                    grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Server at max connections"),
                    this);
                return;
            }

            connection_acquired_ = true;
            std::cout << "RPC called: /helloworld.Greeter/SayHello" << std::endl;
            //testing...
            //std::this_thread::sleep_for(std::chrono::seconds(10));
            reply_.set_message("Hello, " + request_.name());
            status_ = FINISH;
            responder_.Finish(reply_, grpc::Status::OK, this);
        }
        else
        {
            if (status_ != FINISH)
            {
                std::cerr << "Invalid CallData status: " << status_ << std::endl;
                return;
            }
            delete this;
        }
    }

private:
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
    : server_address_(server_address), server_name_(server_name), max_connections_(max_connections)
{
}

Server::~Server()
{
    Stop();
}

bool Server::TryAcquireConnection()
{
    std::lock_guard<std::mutex> lock(connections_mutex_);
    if (active_connections_ < max_connections_)
    {
        active_connections_++;
        std::cout << "Connection acquired (active: " << active_connections_ << "/" << max_connections_ << ")" << std::endl;
        return true;
    }
    return false;
}

void Server::ReleaseConnection()
{
    std::lock_guard<std::mutex> lock(connections_mutex_);
    active_connections_--;
    std::cout << "Connection released (active: " << active_connections_ << "/" << max_connections_ << ")" << std::endl;
}

void Server::Start()
{
    grpc::ServerBuilder builder;

    builder.AddListeningPort(this->server_address_, grpc::InsecureServerCredentials());

    completion_queue_ = builder.AddCompletionQueue();

    async_service_ = std::make_unique<helloworld::Greeter::AsyncService>();
    builder.RegisterService(async_service_.get());

    server_ = builder.BuildAndStart();

    std::cout << this->server_name_ << " listening on " << this->server_address_ << std::endl;
    std::cout << "Max connections: " << max_connections_ << std::endl;
    std::cout << "Async server ready to accept connections" << std::endl;

    RequestNewCall(async_service_.get());

    int num_workers = 4;
    for (int i = 0; i < num_workers; ++i)
    {
        worker_threads_.emplace_back(&Server::HandleRpcs, this);
    }

    for (auto& t : worker_threads_)
    {
        t.join();
    }
}

void Server::Stop()
{
    if (!server_) return;

    std::cout << this->server_name_ << " shutting down, please wait..." << std::endl;

    shutdown_requested_ = true;
    server_->Shutdown();
    completion_queue_->Shutdown();

    std::cout << this->server_name_ << " shutdown" << std::endl;
}

void Server::HandleRpcs()
{
    void* tag;
    bool ok;

    while (completion_queue_->Next(&tag, &ok))
    {
        if (!ok)
        {
            std::cerr << "Completion queue error" << std::endl;
            continue;
        }
        CallData* call = static_cast<CallData*>(tag);
        call->Proceed();
    }
}

void Server::RequestNewCall(helloworld::Greeter::AsyncService* service)
{
    new CallData(service, completion_queue_.get(), this);
}
