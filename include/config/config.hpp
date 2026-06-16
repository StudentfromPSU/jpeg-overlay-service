#pragma once

#include <string>

// Read environment variable or return default value
inline std::string get_env(const std::string& key, const std::string& default_value = "")
{
#ifdef _WIN32
    char* buffer = nullptr;
    size_t size = 0;

    if (_dupenv_s(&buffer, &size, key.c_str()) == 0 &&
        buffer != nullptr)
    {
        std::string value(buffer);
        free(buffer);
        return value;
    }

    return default_value;
#else
    const char* value = std::getenv(key.c_str());
    return value ? value : default_value;
#endif
}

struct Config
{
    std::string host;
    std::string port;

    static Config New()
    {
        Config config;
        config.host = get_env("HOST", "0.0.0.0");
        config.port = get_env("PORT", "50051");
        return config;
    }
};