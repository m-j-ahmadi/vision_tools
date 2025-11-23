#include "sync-server.hpp"
#include <cppcodec/base64_rfc4648.hpp>
#include <fstream>
#include "image-processor.hpp"
#include <memory>
using namespace std;
using json = nlohmann::json;
using base64 = cppcodec::base64_rfc4648;
using boost::asio::ip::tcp;
using namespace mj;

namespace http = boost::beast::http;
SyncServer::SyncServer(std::string host, std::string port) : _host(host),
                                                             _port(port)
{
}

SyncServer::~SyncServer()
{
}

void SyncServer::run()
{

  if (!_host.empty() && !_port.empty())
  {
    try
    {
      auto const address = boost::asio::ip::make_address(_host);
      auto const port = static_cast<unsigned short>(std::atoi(_port.c_str()));
      auto const doc_root = std::make_shared<std::string>("/");
      boost::asio::io_context ioc;
      tcp::acceptor acceptor{ioc, {address, port}};
      std::cerr << "Server is listening on port: " << _port << std::endl;
      for (;;)
      {
        tcp::socket socket(ioc);
        acceptor.accept(socket);
        std::thread{std::bind(
                        &do_session,
                        std::move(socket),
                        doc_root)}
            .detach();
      }
    }
    catch (std::exception &e)
    {
      std::cerr << _host << "," << _port << e.what() << std::endl;
    }
  }
  else
  {
    std::cerr << "Invalid host or port" << std::endl;
  }
}



void SyncServer::do_session(
    tcp::socket &socket,
    std::shared_ptr<std::string const> const &doc_root)
{
    boost::beast::flat_buffer buffer;
    boost::system::error_code err;
    for (;;)
    {
        http::request<http::string_body> req;
        http::read(socket, buffer, req, err);
        if (err == http::error::end_of_stream)
            break;
        if (err)
            std::cerr << "read: " << err.message() << "\n";

        if (req.target() == "/")
        {
            if (req.method() == http::verb::get)
            {
                // send img
                http::file_body::value_type body;
                std::string path = "./x.jpg";
                body.open(path.c_str(), boost::beast::file_mode::read, err);
                auto const size = body.size();
                std::cerr << "IMG SIZE = " << size;

                http::response<http::file_body> res{std::piecewise_construct,
                                                    std::make_tuple(std::move(body)),
                                                    std::make_tuple(http::status::ok, req.version())};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "image/jpeg");
                res.content_length(size);
                res.keep_alive(req.keep_alive());
                http::write(socket, res, err);
            }
            if (req.method() == http::verb::post)
            {
              // Parse the JSON request
              json request_json = json::parse(req.body());

              // Extract base64 image
              std::string base64_img = request_json["img"];
              std::vector<unsigned char> img_data = base64::decode(base64_img);

              // Save image temporarily
              std::ofstream img_file("received_image.jpeg", std::ios::binary);
              img_file.write(reinterpret_cast<const char *>(img_data.data()), img_data.size());
              img_file.close();

              // Read the image with OpenCV
              Mat image = imread("received_image.jpeg", IMREAD_COLOR);
              if (image.empty())
              {
                throw std::runtime_error("Failed to load image");
              }

              // Create base processor
              std::unique_ptr<ImageProcessor> processor = std::make_unique<BaseProcessor>();

              if (request_json.contains("ConvertColorToGray") && request_json["ConvertColorToGray"].get<bool>())
              {
                std::cout << "ConvertColorToGray" << std::endl;
                processor = std::make_unique<GrayscaleProcessor>(std::move(processor));
              }

              if (request_json.contains("Resize"))
              {
                int w = request_json["Resize"]["width"].get<int>();
                int h = request_json["Resize"]["height"].get<int>();
                processor = std::make_unique<ResizeProcessor>(std::move(processor), w, h);
              }

              if (request_json.contains("Blur"))
              {
                int kernel = request_json["Blur"]["kernel_size"].get<int>();
                processor = std::make_unique<BlurProcessor>(std::move(processor), kernel);
              }

              if (request_json.contains("DetectEdges") && request_json["DetectEdges"].get<bool>())
              {
                processor = std::make_unique<EdgeDetectionProcessor>(std::move(processor));
              }

              if (request_json.contains("RotateImage"))
              {
                double angle = request_json["RotateImage"]["angle"];
                processor = std::make_unique<RotateProcessor>(std::move(processor), angle);
              }

              if (request_json.contains("AdjustBrightnessContrast"))
              {
                int brightness = request_json["AdjustBrightnessContrast"]["brightness"];
                double contrast = request_json["AdjustBrightnessContrast"]["contrast"];
                processor = std::make_unique<BrightnessContrastProcessor>(std::move(processor), brightness, contrast);
              }

              if (request_json.contains("ApplySharpening") && request_json["ApplySharpening"].get<bool>())
              {
                processor = std::make_unique<SharpenProcessor>(std::move(processor));
              }

              if (request_json.contains("EqualizeHistogram") && request_json["EqualizeHistogram"].get<bool>())
              {
                processor = std::make_unique<EqualizeHistogramProcessor>(std::move(processor));
              }

              if (request_json.contains("ApplyGammaCorrection"))
              {
                double gamma = request_json["ApplyGammaCorrection"]["gamma"];
                processor = std::make_unique<GammaCorrectionProcessor>(std::move(processor), gamma);
              }

              if (request_json.contains("ApplyWatermark"))
              {
                std::cout << "Apply water mark before" << std::endl;
                std::string text = request_json["ApplyWatermark"]["text"];
                std::cout << "Apply water mark after" << std::endl;
                processor = std::make_unique<WatermarkProcessor>(std::move(processor), text);
              }

              if (request_json.contains("InvertColors") && request_json["InvertColors"].get<bool>())
              {
                processor = std::make_unique<ColorInversionProcessor>(std::move(processor));
              }

              if (request_json.contains("ApplySepia") && request_json["ApplySepia"].get<bool>())
              {
                processor = std::make_unique<SepiaProcessor>(std::move(processor));
              }

              if (request_json.contains("ApplyMedianBlur"))
              {
                int kernel = request_json["ApplyMedianBlur"]["kernel"];
                processor = std::make_unique<MedianBlurProcessor>(std::move(processor), kernel);
              }

              if (request_json.contains("StretchHistogram") && request_json["StretchHistogram"].get<bool>())
              {
                processor = std::make_unique<HistogramStretchProcessor>(std::move(processor));
              }

              if (request_json.contains("ApplyUnsharpMask"))
              {
                double strength = request_json["ApplyUnsharpMask"]["strength"];
                processor = std::make_unique<UnsharpMaskProcessor>(std::move(processor), strength);
              }

              if (request_json.contains("ApplyDilation"))
              {
                int kernel = request_json["ApplyDilation"]["kernel"];
                processor = std::make_unique<DilationProcessor>(std::move(processor), kernel);
              }

              if (request_json.contains("ApplyErosion"))
              {
                int kernel = request_json["ApplyErosion"]["kernel"];
                processor = std::make_unique<ErosionProcessor>(std::move(processor), kernel);
              }

              if (request_json.contains("ApplyCLAHE"))
              {
                double clip_limit = request_json["ApplyCLAHE"]["clip_limit"];
                processor = std::make_unique<CLAHEProcessor>(std::move(processor), clip_limit);
              }

              // Finally, process the image
              image = processor->process(image);

              // Encode the processed image to base64
              std::vector<unsigned char> processed_image_data;
              imencode(".jpg", image, processed_image_data);
              std::string encoded_image = base64::encode(processed_image_data);

              // Prepare the JSON response
              json response_json;
              response_json["processed_image"] = encoded_image;
              std::string response_body = response_json.dump();

              // Send the response
              http::response<http::string_body> res{http::status::ok, req.version()};
              res.set(http::field::content_type, "application/json");
              res.body() = response_body;
              res.content_length(response_body.size());
              res.keep_alive(req.keep_alive());

              http::write(socket, res);
            }

        }
        else if (req.target() == "/stream")
        {
            // cv::Mat frame;
            // std::vector<uchar> buffer;
            // cv::Mat grayframe;
            // cv::VideoCapture cap("./file_example_MP4_480_1_5MG.mp4");

            // if (!cap.isOpened())
            // {
            //     std::cerr << "Err opening video stream!!!";
            //     // return 1;
            // }

            // http::response<http::empty_body> res{http::status::ok, req.version()};
            // res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            // res.set(http::field::content_type, "multipart/x-mixed-replace; boundary=frame");
            // res.keep_alive();
            // http::response_serializer<http::empty_body> sr{res};
            // http::write_header(socket, sr);

            // while (cap.isOpened())
            // {
            //     cap >> frame;
            //     if (!frame.empty())
            //     {

            //         cv::cvtColor(frame, grayframe, cv::COLOR_BGR2GRAY);
            //         // cv:imshow("frame", frame);
            //         cv::waitKey(500);

            //         cv::imencode(".jpg", grayframe, buffer, std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
            //         auto const size = buffer.size();

            //         // do not use http::response<>
            //         // hack: write to socket the multipart message
            //         std::string message{"\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: "};
            //         message += std::to_string(size);
            //         message += "\r\n\r\n";
            //         auto bytesSent = socket.send(boost::asio::buffer(message), 0, err);
            //         if (!bytesSent)
            //         {
            //             break;
            //         }
            //         bytesSent = socket.send(boost::asio::buffer(buffer), 0, err);
            //         if (!bytesSent)
            //         {
            //             break;
            //         }
            //     }
            // }
        
        
        }
    }

    // Send a TCP shutdown
    socket.shutdown(tcp::socket::shutdown_send, err);
}