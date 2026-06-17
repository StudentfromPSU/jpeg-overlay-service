#include "server/server.hpp"

Server::Server(std::string server_address, std::shared_ptr<grpc::Service> service, std::string server_name, int max_connections)
    : server_address_(server_address), service_(service), server_name_(server_name),
      connection_limiter_(std::make_shared<ConnectionLimiter>(max_connections))
{
    this->interceptors_creators_.emplace_back(std::make_unique<LoggerInterceptorFactory>());
}

void Server::Start()
{
    grpc::ServerBuilder builder;

    // Adding the listening port
    builder.AddListeningPort(this->server_address_, grpc::InsecureServerCredentials());

    // Registering the service
    builder.RegisterService(this->service_.get());

    // Setting interceptors for the server
    builder.experimental().SetInterceptorCreators(std::move(this->interceptors_creators_));

    // Building and starting the server
    this->server_ = builder.BuildAndStart();

    std::cout << this->server_name_ << " listening on " << this->server_address_ << std::endl;
    std::cout << "Max connections: " << this->connection_limiter_->GetMaxConnections() << std::endl;

    // Waiting for the server to shutdown
    this->server_->Wait();
}

void Server::Stop()
{
    std::cout << this->server_name_ << " shutting down, please wait..." << std::endl;
    this->server_->Shutdown();
    std::cout << this->server_name_ << " shutdown" << std::endl;
}