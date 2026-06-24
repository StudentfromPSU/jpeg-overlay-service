#pragma once

#include <string>

struct ClientConfig
{
    std::string host;
    std::string port;
    std::string target;

    static ClientConfig New()
    {
        ClientConfig config;

        config.host = "127.0.0.1";
        config.port = "50051";
        config.target = config.host + ":" + config.port;

        return config;
    }
};