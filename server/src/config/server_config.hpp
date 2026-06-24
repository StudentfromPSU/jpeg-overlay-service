#pragma once

#include <string>

struct Config
{
    std::string host;
    std::string port;

    static constexpr Config New()
    {
        Config config;
        config.host = "0.0.0.0";
        config.port = "50051";
        return config;
    }
};