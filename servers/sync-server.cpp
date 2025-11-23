#ifndef MJ_SYNC_SERVER_REFACTOR_HPP
#define MJ_SYNC_SERVER_REFACTOR_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <functional>
#include <system_error>
#include "image-processor.hpp" // your processor chain

namespace mj {

class SyncServer {
public:
    SyncServer(std::string host, std::string port);
    ~SyncServer();

    // Run the server (blocking)
    void run();

private:
    std::string _host;
    std::string _port;

    // session handling
    void do_session(boost::asio::ip::tcp::socket socket);

    // route handlers
    void handle_root_get(boost::asio::ip::tcp::socket &socket, boost::beast::http::request<boost::beast::http::string_body> const &req);
    void handle_root_post(boost::asio::ip::tcp::socket &socket, boost::beast::http::request<boost::beast::http::string_body> const &req);

    // helpers
    bool decode_base64_image(const std::string &b64, std::vector<unsigned char> &out);
    cv::Mat decode_image_mat(const std::string &b64, std::string &err_msg);
    void send_json_response(boost::asio::ip::tcp::socket &socket, nlohmann::json const &j, unsigned version, bool keep_alive);
    void send_error(boost::asio::ip::tcp::socket &socket, boost::beast::http::status status, std::string const &message, unsigned version, bool keep_alive);
};

} // namespace mj

#endif // MJ_SYNC_SERVER_REFACTOR_HPP

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------

#include <cstdlib>
#include <cctype>

using namespace std;
using json = nlohmann::json;
using base64 = cppcodec::base64_rfc4648;
using boost::asio::ip::tcp;
namespace http = boost::beast::http;
using namespace mj;

// Constants
static constexpr std::size_t MAX_REQUEST_BODY = 10 * 1024 * 1024; // 10 MB limit

SyncServer::SyncServer(std::string host, std::string port) : _host(std::move(host)), _port(std::move(port)) {}
SyncServer::~SyncServer() {}

void SyncServer::run()
{
    if (_host.empty() || _port.empty())
    {
        std::cerr << "Invalid host or port\n";
        return;
    }

    boost::asio::io_context ioc{1};

    try
    {
        auto const address = boost::asio::ip::make_address(_host);
        unsigned short port_num = static_cast<unsigned short>(std::atoi(_port.c_str()));

        tcp::acceptor acceptor{ioc, {address, port_num}};
        std::cerr << "Server is listening on " << _host << ":" << _port << std::endl;

        for (;;)
        {
            boost::system::error_code ec;
            tcp::socket socket{ioc};
            acceptor.accept(socket, ec);
            if (ec)
            {
                std::cerr << "Accept failed: " << ec.message() << std::endl;
                continue;
            }

            // Spawn thread to serve this connection
            std::thread(&SyncServer::do_session, this, std::move(socket)).detach();
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Fatal server error: " << e.what() << std::endl;
    }
}

void SyncServer::do_session(tcp::socket socket)
{
    boost::beast::flat_buffer buffer;
    boost::system::error_code ec;

    for (;;)
    {
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);

        if (ec == http::error::end_of_stream)
        {
            break;
        }
        if (ec)
        {
            std::cerr << "read error: " << ec.message() << std::endl;
            break;
        }

        // Basic body size protection
        if (req.body().size() > MAX_REQUEST_BODY)
        {
            send_error(socket, http::status::payload_too_large, "Request body too large", req.version(), req.keep_alive());
            break;
        }

        // route matching (path only, ignore query for now)
        std::string target = req.target();
        auto pos = target.find('?');
        if (pos != std::string::npos)
            target.resize(pos);

        try
        {
            if (target == "/")
            {
                if (req.method() == http::verb::get)
                    handle_root_get(socket, req);
                else if (req.method() == http::verb::post)
                    handle_root_post(socket, req);
                else
                    send_error(socket, http::status::method_not_allowed, "Method not allowed", req.version(), req.keep_alive());
            }
            else if (target == "/stream")
            {
                // If you want streaming, implement handle_stream() separately.
                send_error(socket, http::status::not_implemented, "Stream endpoint not implemented in this refactor", req.version(), req.keep_alive());
            }
            else
            {
                send_error(socket, http::status::not_found, "Route not found", req.version(), req.keep_alive());
            }
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Handler exception: " << ex.what() << std::endl;
            send_error(socket, http::status::internal_server_error, ex.what(), req.version(), req.keep_alive());
            // do not break here; allow socket to continue depending on client
        }
        catch (...)
        {
            std::cerr << "Unknown handler exception\n";
            send_error(socket, http::status::internal_server_error, "Unknown error", req.version(), req.keep_alive());
        }

        // If connection is not keep-alive, close after one request
        if (!req.keep_alive())
            break;
    }

    // shutdown
    boost::system::error_code shutdown_ec;
    socket.shutdown(tcp::socket::shutdown_send, shutdown_ec);
}

void SyncServer::handle_root_get(tcp::socket &socket, http::request<http::string_body> const &req)
{
    boost::system::error_code ec;

    // Example: serve static image from disk (you may want to embed or generate)
    std::string path = "./x.jpg";
    http::file_body::value_type body;
    body.open(path.c_str(), boost::beast::file_mode::read, ec);
    if (ec)
    {
        send_error(socket, http::status::not_found, "Image not found", req.version(), req.keep_alive());
        return;
    }

    auto const size = body.size();

    http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};

    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "image/jpeg");
    res.content_length(size);
    res.keep_alive(req.keep_alive());

    http::write(socket, res, ec);
    if (ec)
        std::cerr << "write error (GET): " << ec.message() << std::endl;
}

void SyncServer::handle_root_post(tcp::socket &socket, http::request<http::string_body> const &req)
{
    // Parse JSON safely
    json request_json;
    try
    {
        request_json = json::parse(req.body());
    }
    catch (const std::exception &e)
    {
        send_error(socket, http::status::bad_request, std::string("Invalid JSON: ") + e.what(), req.version(), req.keep_alive());
        return;
    }

    // Validate img field
    if (!request_json.contains("img") || !request_json["img"].is_string())
    {
        send_error(socket, http::status::bad_request, "Missing or invalid 'img' field", req.version(), req.keep_alive());
        return;
    }

    // Decode image into cv::Mat (no temporary file)
    std::string err_msg;
    cv::Mat image = decode_image_mat(request_json["img"].get<std::string>(), err_msg);
    if (image.empty())
    {
        send_error(socket, http::status::bad_request, std::string("Failed to decode image: ") + err_msg, req.version(), req.keep_alive());
        return;
    }

    // Build processor chain using described options
    std::unique_ptr<ImageProcessor> processor = std::make_unique<BaseProcessor>();

    // Use safe checks for each optional field before accessing
    if (request_json.value("ConvertColorToGray", false))
    {
        processor = std::make_unique<GrayscaleProcessor>(std::move(processor));
    }

    if (request_json.contains("Resize") && request_json["Resize"].is_object())
    {
        int w = request_json["Resize"].value("width", 0);
        int h = request_json["Resize"].value("height", 0);
        if (w > 0 && h > 0)
            processor = std::make_unique<ResizeProcessor>(std::move(processor), w, h);
    }

    if (request_json.contains("Blur") && request_json["Blur"].is_object())
    {
        int kernel = request_json["Blur"].value("kernel_size", 0);
        if (kernel > 0)
            processor = std::make_unique<BlurProcessor>(std::move(processor), kernel);
    }

    if (request_json.value("DetectEdges", false))
    {
        processor = std::make_unique<EdgeDetectionProcessor>(std::move(processor));
    }

    if (request_json.contains("RotateImage") && request_json["RotateImage"].is_object())
    {
        double angle = request_json["RotateImage"].value("angle", 0.0);
        processor = std::make_unique<RotateProcessor>(std::move(processor), angle);
    }

    if (request_json.contains("AdjustBrightnessContrast") && request_json["AdjustBrightnessContrast"].is_object())
    {
        int brightness = request_json["AdjustBrightnessContrast"].value("brightness", 0);
        double contrast = request_json["AdjustBrightnessContrast"].value("contrast", 1.0);
        processor = std::make_unique<BrightnessContrastProcessor>(std::move(processor), brightness, contrast);
    }

    if (request_json.value("ApplySharpening", false))
    {
        processor = std::make_unique<SharpenProcessor>(std::move(processor));
    }

    if (request_json.value("EqualizeHistogram", false))
    {
        processor = std::make_unique<EqualizeHistogramProcessor>(std::move(processor));
    }

    if (request_json.contains("ApplyGammaCorrection") && request_json["ApplyGammaCorrection"].is_object())
    {
        double gamma = request_json["ApplyGammaCorrection"].value("gamma", 1.0);
        processor = std::make_unique<GammaCorrectionProcessor>(std::move(processor), gamma);
    }

    if (request_json.contains("ApplyWatermark") && request_json["ApplyWatermark"].is_object())
    {
        std::string text = request_json["ApplyWatermark"].value("text", std::string());
        if (!text.empty())
            processor = std::make_unique<WatermarkProcessor>(std::move(processor), text);
    }

    if (request_json.value("InvertColors", false))
    {
        processor = std::make_unique<ColorInversionProcessor>(std::move(processor));
    }

    if (request_json.value("ApplySepia", false))
    {
        processor = std::make_unique<SepiaProcessor>(std::move(processor));
    }

    if (request_json.contains("ApplyMedianBlur") && request_json["ApplyMedianBlur"].is_object())
    {
        int kernel = request_json["ApplyMedianBlur"].value("kernel", 0);
        if (kernel > 0)
            processor = std::make_unique<MedianBlurProcessor>(std::move(processor), kernel);
    }

    if (request_json.value("StretchHistogram", false))
    {
        processor = std::make_unique<HistogramStretchProcessor>(std::move(processor));
    }

    if (request_json.contains("ApplyUnsharpMask") && request_json["ApplyUnsharpMask"].is_object())
    {
        double strength = request_json["ApplyUnsharpMask"].value("strength", 1.0);
        processor = std::make_unique<UnsharpMaskProcessor>(std::move(processor), strength);
    }

    if (request_json.contains("ApplyDilation") && request_json["ApplyDilation"].is_object())
    {
        int kernel = request_json["ApplyDilation"].value("kernel", 0);
        if (kernel > 0)
            processor = std::make_unique<DilationProcessor>(std::move(processor), kernel);
    }

    if (request_json.contains("ApplyErosion") && request_json["ApplyErosion"].is_object())
    {
        int kernel = request_json["ApplyErosion"].value("kernel", 0);
        if (kernel > 0)
            processor = std::make_unique<ErosionProcessor>(std::move(processor), kernel);
    }

    if (request_json.contains("ApplyCLAHE") && request_json["ApplyCLAHE"].is_object())
    {
        double clip_limit = request_json["ApplyCLAHE"].value("clip_limit", 2.0);
        processor = std::make_unique<CLAHEProcessor>(std::move(processor), clip_limit);
    }

    // Process image
    cv::Mat processed = processor->process(image);

    // Encode to JPEG in memory then base64
    std::vector<unsigned char> out_buf;
    if (!cv::imencode(".jpg", processed, out_buf))
    {
        send_error(socket, http::status::internal_server_error, "Failed to encode processed image", req.version(), req.keep_alive());
        return;
    }

    std::string encoded = base64::encode(out_buf);

    json response_json;
    response_json["processed_image"] = encoded;

    send_json_response(socket, response_json, req.version(), req.keep_alive());
}

bool SyncServer::decode_base64_image(const std::string &b64, std::vector<unsigned char> &out)
{
    try
    {
        out = base64::decode(b64);
        return !out.empty();
    }
    catch (...) { return false; }
}

cv::Mat SyncServer::decode_image_mat(const std::string &b64, std::string &err_msg)
{
    std::vector<unsigned char> img_data;
    if (!decode_base64_image(b64, img_data))
    {
        err_msg = "Base64 decode failed";
        return {};
    }

    cv::Mat buf(1, static_cast<int>(img_data.size()), CV_8U, img_data.data());
    cv::Mat img = cv::imdecode(buf, cv::IMREAD_COLOR);
    if (img.empty())
        err_msg = "OpenCV imdecode failed";
    return img;
}

void SyncServer::send_json_response(tcp::socket &socket, json const &j, unsigned version, bool keep_alive)
{
    boost::system::error_code ec;
    http::response<http::string_body> res{http::status::ok, version};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.body() = j.dump();
    res.content_length(res.body().size());
    res.keep_alive(keep_alive);

    http::write(socket, res, ec);
    if (ec)
        std::cerr << "write error (JSON response): " << ec.message() << std::endl;
}

void SyncServer::send_error(tcp::socket &socket, http::status status, std::string const &message, unsigned version, bool keep_alive)
{
    json j;
    j["error"] = message;

    boost::system::error_code ec;
    http::response<http::string_body> res{status, version};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.body() = j.dump();
    res.content_length(res.body().size());
    res.keep_alive(keep_alive);

    http::write(socket, res, ec);
    if (ec)
        std::cerr << "write error (send_error): " << ec.message() << std::endl;
}

// End of file
