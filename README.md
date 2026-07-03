# JPEG Overlay Service

Asynchronous gRPC service for processing JPEG images with text overlay.

The project consists of a reusable image processing library, an asynchronous gRPC server, and a console client application. The server accepts JPEG images, overlays arbitrary text, and returns the processed image.

---

# Requirements

Before building the project, install:

- C++20 compatible compiler
- CMake 3.20+
- OpenCV
- gRPC
- Protocol Buffers
- Git

---

# Architecture

The project consists of three independent modules.

## image_processor

A reusable C++ library responsible for JPEG processing.

Responsibilities:

- JPEG decoding
- Text rendering
- JPEG encoding
- Exception-based error handling

Public API:

```cpp
class ImageProcessor {
public:
    std::vector<Byte> Process(
        const std::vector<Byte>& imageBytes,
        const std::string& text
    );
};
```

---

## grpc_server

Asynchronous gRPC server implementing the `ImageProcessor` service.

Features:

- CompletionQueue-based architecture
- Asynchronous request lifecycle

```
CREATE → PROCESS → FINISH
```

- Thread pool sized according to available CPU cores
- Configurable maximum number of concurrent connections
- Graceful error handling

Default server address:

```
0.0.0.0:50051
```

---

## image_processor_app

Console client for sending a single image to the server.

Features:

- Reads JPEG image from disk
- Sends processing request
- Saves processed image
- Automatic retry on `RESOURCE_EXHAUSTED`
- Configurable retry timeout

---

# Build

## Clone repository

```bash
git clone https://github.com/username/jpeg-overlay-service.git
cd jpeg-overlay-service
```

## Configure

```bash
mkdir build
cd build

cmake ..
```

## Build

```bash
cmake --build . --config Release
```

---

# Running

## Start server

```bash
./grpc_server
```

Expected output:

```
Server listening on 0.0.0.0:50051
Max connections: 3
Async server ready to accept connections
Available CPU cores: 8, spawning 8 worker threads
```

Stop the server using **Ctrl+C**.

---

## Run client

```bash
./image_processor_app <input_image> <output_directory> <text> <retry_timeout_ms>
```

Example:

```bash
./image_processor_app input.jpg ./results "Hello World" 1000
```

Arguments:

| Argument | Description |
|----------|-------------|
| input_image | Path to input JPEG file |
| output_directory | Directory for processed image |
| text | Text to overlay |
| retry_timeout_ms | Retry interval in milliseconds |

Successful execution:

```
Connecting to server: 127.0.0.1:50051
Image processed successfully.
Output saved to: results/processed_input.jpg
```

Possible errors:

```
Input file not found
Output directory not found
Input image is empty
Cannot write output file
Server is busy
Server unavailable
```

---

# gRPC API

## ImageRequest

```protobuf
message ImageRequest {
    bytes image = 1;
    string text = 2;
}
```

## ImageResponse

```protobuf
message ImageResponse {
    bytes image = 1;
}
```

---

# gRPC Status Codes

| Code | Description |
|------|-------------|
| RESOURCE_EXHAUSTED | Server reached maximum connection limit |
| INVALID_ARGUMENT | Invalid image or request |
| INTERNAL | Internal server error |
| UNAVAILABLE | Server unavailable |

---

## Git Flow

The project follows a simple feature-branch workflow.

1. Create an issue describing the task or bug.
2. Create a new branch from the `main` branch:

   ```
   git checkout main
   git pull
   git checkout -b feature/<issue-name>
   ```
3. Implement the required changes and commit them to the feature branch.
4. Push the branch to the remote repository:

   ```
   git push origin feature/<issue-name>
   ```
5. Create a Pull Request from `feature/<issue-name>` to `main`, referencing the corresponding issue (for example, using `Closes #<issue_number>`).
6. After code review and approval, merge the Pull Request into `main`.
7. Delete the feature branch locally and remotely after the merge.

