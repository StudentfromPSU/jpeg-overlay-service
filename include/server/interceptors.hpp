#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/interceptor.h>
#include <grpcpp/support/server_interceptor.h>
#include <string>
#include <atomic>
#include <memory>
#include <iostream>

// Shared state for connection limiting
class ConnectionLimiter
{
public:
    explicit ConnectionLimiter(int max_connections)
        : max_connections_(max_connections), active_connections_(0) {}

    bool TryAcquire()
    {
        int current = active_connections_.load();
        while (current < max_connections_)
        {
            if (active_connections_.compare_exchange_weak(current, current + 1))
            {
                return true;
            }
        }
        return false;
    }

    void Release()
    {
        active_connections_--;
    }

    int GetActiveConnections() const
    {
        return active_connections_.load();
    }

    int GetMaxConnections() const
    {
        return max_connections_;
    }

private:
    int max_connections_;
    std::atomic<int> active_connections_;
};

// RAII guard for automatic connection release
class ConnectionGuard
{
public:
    explicit ConnectionGuard(std::shared_ptr<ConnectionLimiter> limiter, bool acquired)
        : limiter_(limiter), acquired_(acquired) {}

    ~ConnectionGuard()
    {
        if (acquired_)
        {
            limiter_->Release();
        }
    }

    bool IsAcquired() const { return acquired_; }

private:
    std::shared_ptr<ConnectionLimiter> limiter_;
    bool acquired_;
};

// Logs every incoming RPC request
class LoggerInterceptor final : public grpc::experimental::Interceptor
{
public:
    explicit LoggerInterceptor(grpc::experimental::ServerRpcInfo* info)
    {
        const std::string method = info->method();

        if (method == "unknown")
        {
            std::cout << "Unimplemented Rpc called" << std::endl;
            return;
        }

        std::cout << "Rpc called: " << method << std::endl;
    }

    void Intercept(grpc::experimental::InterceptorBatchMethods* methods) override
    {
        methods->Proceed();
    }
};

// Factory used by gRPC to create interceptors
class LoggerInterceptorFactory : public grpc::experimental::ServerInterceptorFactoryInterface
{
public:
    grpc::experimental::Interceptor* CreateServerInterceptor(grpc::experimental::ServerRpcInfo* info) override
    {
        return new LoggerInterceptor(info);
    }
};