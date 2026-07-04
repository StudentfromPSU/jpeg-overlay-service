#include <iostream>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "config/client_config.hpp"
#include "client.hpp"

namespace CommandLineParser
{
    ClientArguments Parse(int argc, char* argv[]);
}

int main(int argc, char* argv[])
{
    try
    {
        ClientArguments args = CommandLineParser::Parse(argc, argv);

        ClientConfig config = ClientConfig::New();

        auto channel = grpc::CreateChannel(config.target, grpc::InsecureChannelCredentials());
        auto grpc_client = std::make_shared<ImageProcessorClient>(channel);

        ClientApplication app(args, grpc_client);

        return app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
