#include "client.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace CommandLineParser
{
    void PrintUsage(const char* programName)
    {
        std::cout << "Usage: " << programName << " <input_image> <output_directory> <text> <retry_timeout_ms>" << std::endl;
        std::cout << std::endl;
        std::cout << "Arguments:" << std::endl;
        std::cout << "  input_image       Path to input JPEG image (e.g., images/input.jpg)" << std::endl;
        std::cout << "  output_directory  Directory to save processed image (e.g., results/)" << std::endl;
        std::cout << "  text              Text to overlay on image (can contain spaces)" << std::endl;
        std::cout << "  retry_timeout_ms  Retry timeout in milliseconds (e.g., 1000 for 1 second)" << std::endl;
        std::cout << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "  " << programName << " input.jpg output/ \"Hello World\" 1000" << std::endl;
    }

    ClientArguments Parse(int argc, char* argv[])
    {
        if (argc != 5)
        {
            PrintUsage(argv[0]);
            throw std::invalid_argument("Expected 4 arguments: input_image output_directory text retry_timeout_ms");
        }

        ClientArguments args;
        args.inputPath = argv[1];
        args.outputDirectory = argv[2];
        args.text = argv[3];

        try
        {
            args.retryTimeoutMs = std::stoi(argv[4]);
            if (args.retryTimeoutMs < 0)
            {
                throw std::invalid_argument("retry_timeout_ms must be non-negative.");
            }
        }
        catch (const std::exception&)
        {
            throw std::invalid_argument("retry_timeout_ms must be a valid non-negative integer.");
        }

        return args;
    }
}

namespace FileValidator
{
    void ValidateInputFile(const std::string& filepath)
    {
        if (!fs::exists(filepath))
        {
            throw std::runtime_error("Error: Input file not found: " + filepath);
        }

        if (!fs::is_regular_file(filepath))
        {
            throw std::runtime_error("Error: Input path is not a file: " + filepath);
        }

        try
        {
            if (fs::file_size(filepath) == 0)
            {
                throw std::runtime_error("Error: Input image is empty: " + filepath);
            }
        }
        catch (const fs::filesystem_error& e)
        {
            throw std::runtime_error("Error: Cannot access input file: " + std::string(e.what()));
        }
    }

    void ValidateOutputDirectory(const std::string& dirpath)
    {
        if (!fs::exists(dirpath))
        {
            throw std::runtime_error("Error: Output directory not found: " + dirpath);
        }

        if (!fs::is_directory(dirpath))
        {
            throw std::runtime_error("Error: Output path is not a directory: " + dirpath);
        }

        try
        {
            fs::path test_file = fs::path(dirpath) / ".write_test";
            std::ofstream test_stream(test_file);
            if (!test_stream.is_open())
            {
                throw std::runtime_error("Error: Cannot write to output directory: " + dirpath);
            }
            test_stream.close();
            fs::remove(test_file);
        }
        catch (const fs::filesystem_error& e)
        {
            throw std::runtime_error("Error: Cannot write to output directory: " + std::string(e.what()));
        }
        catch (const std::runtime_error&)
        {
            throw;
        }
    }

    std::string BuildOutputPath(const std::string& inputPath, const std::string& outputDir)
    {
        fs::path input_file_path(inputPath);
        std::string output_filename = "processed_" + input_file_path.filename().string();
        return (fs::path(outputDir) / output_filename).string();
    }
}

namespace ImageFileManager
{
    std::vector<uint8_t> LoadImageFile(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Cannot open input file: " + filepath);
        }

        file.seekg(0, std::ios::end);
        std::streampos pos = file.tellg();

        if (pos == std::streampos(-1))
        {
            throw std::runtime_error("Failed to determine file size: " + filepath);
        }

        size_t size = static_cast<size_t>(pos);

        if (size == 0)
        {
            throw std::runtime_error("Input image is empty: " + filepath);
        }

        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), size);

        if (!file || file.gcount() != static_cast<std::streamsize>(size))
        {
            throw std::runtime_error("Failed to read input image: " + filepath);
        }

        return buffer;
    }

    void SaveImageFile(const std::string& filepath, const std::string& imageData)
    {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Cannot create output file: " + filepath);
        }

        file.write(imageData.data(), imageData.size());

        if (!file)
        {
            throw std::runtime_error("Failed to write output file: " + filepath);
        }
    }
}

ImageProcessorClient::ImageProcessorClient(std::shared_ptr<Channel> channel)
    : stub_(ImageProcessor::NewStub(channel)) {}

ProcessResult ImageProcessorClient::ProcessImage(
    const std::vector<uint8_t>& imageData,
    const std::string& text,
    std::string& processedImageOut)
{
    ImageRequest request;
    request.set_image(imageData.data(), imageData.size());
    request.set_text(text);

    ImageResponse reply;
    ClientContext context;

    Status status = stub_->ProcessImage(&context, request, &reply);

    if (!status.ok())
    {
        if (status.error_code() == grpc::StatusCode::RESOURCE_EXHAUSTED)
        {
            return ProcessResult::ServerBusy;
        }
        else
        {
            std::cerr << "RPC error: " << status.error_message() << std::endl;
            return ProcessResult::RpcError;
        }
    }

    if (reply.image().empty())
    {
        std::cerr << "Server returned empty image." << std::endl;
        return ProcessResult::RpcError;
    }

    processedImageOut = reply.image();
    return ProcessResult::Success;
}

ClientApplication::ClientApplication(
    const ClientArguments& args,
    std::shared_ptr<ImageProcessorClient> grpcClient)
    : args_(args),
      grpc_client_(grpcClient)
{
}

int ClientApplication::Run()
{
    FileValidator::ValidateInputFile(args_.inputPath);
    FileValidator::ValidateOutputDirectory(args_.outputDirectory);

    std::string output_path = FileValidator::BuildOutputPath(
        args_.inputPath,
        args_.outputDirectory);

    std::vector<uint8_t> image_data = ImageFileManager::LoadImageFile(args_.inputPath);

    std::cout << "Connecting to server..." << std::endl;

    while (true)
    {
        std::string processed_image;
        ProcessResult result = grpc_client_->ProcessImage(
            image_data,
            args_.text,
            processed_image);

        if (result == ProcessResult::Success)
        {
            ImageFileManager::SaveImageFile(output_path, processed_image);
            std::cout << "Image processed successfully." << std::endl;
            std::cout << "Output saved to: " << output_path << std::endl;
            return 0;
        }
        else if (result == ProcessResult::ServerBusy)
        {
            std::cout << "Server is busy. Retrying in " << args_.retryTimeoutMs << " ms..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(args_.retryTimeoutMs));
        }
        else if (result == ProcessResult::RpcError)
        {
            std::cerr << "Error: RPC request failed." << std::endl;
            return 1;
        }
    }
}
