# Computer Vision Server Application

## Overview

This application is a server-based solution for computer vision and image processing, utilizing OpenCV and Boost libraries. It exposes a RESTful API, allowing clients to interact with image processing functionalities over HTTP.

## Features

- Image processing using OpenCV
- RESTful API for easy integration with web and mobile clients
- Support for various image formats (JPEG, PNG, etc.)
- Perform operations such as image filtering, object detection, and more

## Requirements

- C++14 or later
- OpenCV 4.x
- Boost 1.70 or later
- CMake 3.x
- A compatible compiler (e.g., GCC, Clang, MSVC)

## Installation

1. **Clone the repository:**

   ```bash
   git clone https://github.com:m-j-ahmadi/vision_tools.git
   cd your-repo-name
   ```

2. **Install dependencies:**

   - **Ubuntu:**

     ```bash
     sudo apt-get install libopencv-dev libboost-all-dev
     ```

   - **Windows:**
     - Download and install OpenCV and Boost from their respective official sites.

3. **Build the application:**

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

## Usage

1. **Start the server:**

   ```bash
   ./vision_tools 127.0.0.1 2020
   ```

2. **API Endpoints:**

   - **POST /process-image**
     - Description: Upload an image for processing.
     - Request body: `multipart/form-data`
     - Response: JSON with processing results.

   - **GET /status**
     - Description: Check server status.
     - Response: JSON indicating server status.

## Examples

### Processing an Image

Use a tool like `curl` or Postman to send a request to the API.

```bash
curl -X POST http://127.0.0.1:2020/ \
  -H "Content-Type: application/json" \
  -d '{
        "img": "<encoded_image>",
        "ResizeImage": {
            "width": 100,
            "height": 100
        }
      }'
```

### Check Server Status

```bash
curl http://localhost:8000/status
```

## Contributing

Contributions are welcome! Please read the [CONTRIBUTING.md](CONTRIBUTING.md) for details on how to contribute.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- OpenCV for its powerful image processing capabilities.
- Boost for its extensive C++ library support.

## Contact

For inquiries, please contact [mohamad.j.ahmadi@gmail.com].
```

Feel free to modify the text, endpoints, and examples according to your application specifics!