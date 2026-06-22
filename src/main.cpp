#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

#include "config/config.hpp"
#include "server/server.hpp"
#include "hello.grpc.pb.h"

namespace hw = helloworld;

std::atomic<bool> shutdown_requested{false};

void signal_handler(int signal)
{
    (void)signal;
    shutdown_requested.store(true, std::memory_order_release);
}

int main()
{
    try
    {
        if (std::signal(SIGINT, signal_handler) == SIG_ERR)
        {
            throw std::runtime_error("Failed to register SIGINT handler");
        }
        if (std::signal(SIGTERM, signal_handler) == SIG_ERR)
        {
            throw std::runtime_error("Failed to register SIGTERM handler");
        }

        Config config = Config::New();
        auto server = std::make_unique<Server>(config.host + ":" + config.port, hw::Greeter::service_full_name(), 3);

        server->Start();

        while (!shutdown_requested.load(std::memory_order_acquire))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        server->Stop();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}