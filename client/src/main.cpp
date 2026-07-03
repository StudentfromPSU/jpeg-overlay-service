#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <grpcpp/grpcpp.h>
#include "config/client_config.hpp"
#include "hello.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloRequest;
using helloworld::HelloReply;

class GreeterClient
{
public:
    GreeterClient(std::shared_ptr<Channel> channel)
        : stub_(Greeter::NewStub(channel)) {
    }

    std::string SayHello(const std::string& user)
    {
        HelloRequest request;
        request.set_name(user);

        HelloReply reply;
        ClientContext context;

        Status status = stub_->SayHello(&context, request, &reply);

        if (status.ok())
        {
            return reply.message();
        }
        else
        {
            return std::string("RPC failed with code ") +
                std::to_string(status.error_code()) + ": " +
                status.error_message();
        }
    }

private:
    std::unique_ptr<Greeter::Stub> stub_;
};

void SingleRequest(const std::string& target_str)
{
    std::cout << "\n=== Single Request ===" << std::endl;

    GreeterClient greeter(grpc::CreateChannel(
        target_str, grpc::InsecureChannelCredentials()));

    std::string reply = greeter.SayHello("World");
    std::cout << "Greeter received: " << reply << std::endl;
}

void MultipleRequests(const std::string& target_str, int count)
{
    std::cout << "\n=== " << count << " Sequential Requests ===" << std::endl;

    GreeterClient greeter(grpc::CreateChannel(
        target_str, grpc::InsecureChannelCredentials()));

    for (int i = 0; i < count; ++i)
    {
        std::string name = "Client" + std::to_string(i + 1);
        std::string reply = greeter.SayHello(name);
        std::cout << "[" << i + 1 << "/" << count << "] " << reply << std::endl;
    }
}

void ConnectionLimitTestParallel(const std::string& target_str, int num_clients)
{
    std::cout << "\n=== Connection Limit Test - Parallel Requests ===" << std::endl;
    std::cout << "Creating " << num_clients << " channels and sending requests in parallel...\n"
        << std::endl;

    std::vector<std::shared_ptr<Channel>> channels;
    std::vector<std::unique_ptr<GreeterClient>> clients;
    std::vector<int> results(num_clients, 0);
    std::mutex result_mutex;

    channels.reserve(num_clients);
    clients.reserve(num_clients);

    for (int i = 0; i < num_clients; ++i)
    {
        auto channel = grpc::CreateChannel(
            target_str, grpc::InsecureChannelCredentials());
        channels.push_back(channel);
        clients.push_back(std::make_unique<GreeterClient>(channel));
    }

    std::vector<std::thread> threads;
    threads.reserve(num_clients);

    for (int i = 0; i < num_clients; ++i)
    {
        threads.emplace_back([&, i]()
        {
            try
            {
                std::string name = "Client" + std::to_string(i + 1);
                std::cout << "[Client " << i + 1 << "] Sending RPC request..." << std::endl;

                std::string reply = clients[i]->SayHello(name);

                {
                    std::lock_guard<std::mutex> lock(result_mutex);
                    if (reply.find("Hello") != std::string::npos)
                    {
                        std::cout << "[Client " << i + 1 << "] SUCCESS: " << reply << std::endl;
                        results[i] = 1;
                    }
                    else if (reply.find("ResourceExhausted") != std::string::npos)
                    {
                        std::cout << "[Client " << i + 1 << "] REJECTED: Server at max connections"
                            << std::endl;
                        results[i] = -1;
                    }
                    else
                    {
                        std::cout << "[Client " << i + 1 << "] ? ERROR: " << reply << std::endl;
                        results[i] = -1;
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::lock_guard<std::mutex> lock(result_mutex);
                std::cout << "[Client " << i + 1 << "] ? EXCEPTION: " << e.what() << std::endl;
                results[i] = -1;
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    int success_count = 0;
    int rejected_count = 0;
    for (int result : results)
    {
        if (result == 1) success_count++;
        else if (result == -1) rejected_count++;
    }

    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Successful: " << success_count << std::endl;
    std::cout << "Rejected: " << rejected_count << std::endl;
    std::cout << "Total: " << success_count + rejected_count << std::endl;
}

void PerformanceTest(const std::string& target_str, int num_requests)
{
    std::cout << "\n=== Performance Test (" << num_requests << " requests) ===" << std::endl;

    GreeterClient greeter(grpc::CreateChannel(
        target_str, grpc::InsecureChannelCredentials()));

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_requests; ++i)
    {
        std::string name = "PerfTest" + std::to_string(i);
        greeter.SayHello(name);

        if ((i + 1) % 100 == 0)
        {
            std::cout << "Completed: " << i + 1 << "/" << num_requests << std::endl;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "Total requests: " << num_requests << std::endl;
    std::cout << "Time: " << duration.count() << " ms" << std::endl;
    std::cout << "Average speed: " << (num_requests * 1000.0 / duration.count()) << " req/s"
        << std::endl;
}

void PrintUsage()
{
    std::cout << "\n=== gRPC Client ===" << std::endl;
    std::cout << "\nAvailable tests:" << std::endl;
    std::cout << "  1 - Single request" << std::endl;
    std::cout << "  2 - Multiple sequential requests (10)" << std::endl;
    std::cout << "  3 - Connection limit test PARALLEL (5 clients)" << std::endl;
    std::cout << "  4 - Performance test (500 requests)" << std::endl;
    std::cout << "  5 - All tests" << std::endl;
    std::cout << "\nUsage examples:" << std::endl;
    std::cout << "  client 1                    (single request)" << std::endl;
    std::cout << "  client 3                    (parallel connection test)" << std::endl;
    std::cout << "  client 5                    (all tests)" << std::endl;
    std::cout << "\nServer parameters:" << std::endl;
    std::cout << "  Address: localhost:50051" << std::endl;
    std::cout << "  Max connections: 3" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    ClientConfig config = ClientConfig::New();

    std::string target_str = config.target;

    if (argc < 2)
    {
        PrintUsage();
        std::cout << "Usage: " << argv[0] << " <test_number>" << std::endl;
        return 1;
    }

    int test_num = std::stoi(argv[1]);

    std::cout << "Connecting to server: " << target_str << std::endl;

    try
    {
        switch (test_num)
        {
        case 1:
            SingleRequest(target_str);
            break;

        case 2:
            MultipleRequests(target_str, 10);
            break;

        case 3:
            ConnectionLimitTestParallel(target_str, 5);
            break;

        case 4:
            PerformanceTest(target_str, 500);
            break;

        case 5:
            SingleRequest(target_str);
            MultipleRequests(target_str, 10);
            ConnectionLimitTestParallel(target_str, 5);;
            PerformanceTest(target_str, 500);
            break;

        default:
            std::cerr << "Unknown test: " << test_num << std::endl;
            PrintUsage();
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}