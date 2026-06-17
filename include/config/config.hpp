#pragma once

#include <string>

struct Config
{
    std::string host;
    std::string port;
    int max_connections;

    static Config New()
    {
        Config config;
        config.host = "0.0.0.0";
        config.port = "50051";
        config.max_connections = 5;
        return config;
    }
};