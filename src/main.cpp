#include <iostream>
#include <csignal>
#include <condition_variable>
#include <mutex>

#include "config/config.hpp"
#include "service/hello_service.hpp"
#include "server/server.hpp"

std::unique_ptr<Server> g_server = nullptr;
bool shutdown_requested = false;
std::mutex shutdown_mutex;
std::condition_variable shutdown_cv;

void signal_handler(int signal)
{
    std::cout << "Received signal " << signal << std::endl;

    shutdown_requested = true;
    shutdown_cv.notify_one();
}

int main()
{
    try
    {
        // Register signal handlers for graceful shutdown
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        // Load configuration and initialize service
        Config config = Config::New();
        std::shared_ptr<HelloService> oService = std::make_shared<HelloService>();
        g_server = std::make_unique<Server>(config.host + ":" + config.port, oService, hw::Greeter::service_full_name());

        // Wait for shutdown signal in a separate thread
        std::thread shutdown_thread(
            [&]()
            {
                std::unique_lock<std::mutex> lock(shutdown_mutex);

                shutdown_cv.wait(lock,
                    [&]()
                    {
                        return shutdown_requested;
                    });

                g_server->Stop();
            });
        // Start serving incoming RPC requests
        g_server->Start();

        shutdown_thread.join();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}