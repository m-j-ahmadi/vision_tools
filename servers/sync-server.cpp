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
               //Parse the JSON request
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

               // Finally, process the image
               image = processor->process(image);

               //  // Check if we need to convert to grayscale
               //  if (request_json.contains("ConvertColorToGray"))
               //  {
               //      bool convert_to_gray = request_json["ConvertColorToGray"];
               //      if (convert_to_gray)
               //      {
               //          Mat gray_image;
               //          cvtColor(image, gray_image, COLOR_BGR2GRAY);
               //          imwrite("gray_image.jpg", gray_image);
               //          image = gray_image; // Replace the original image with the grayscale version
               //      }
               //  }
               //  // Check if we need to apply edge detection
               //  if (request_json.contains("DetectEdges"))
               //  {
               //      bool detect_edges = request_json["DetectEdges"];
               //      if (detect_edges)
               //      {
               //          Mat edges_image;
               //          // Apply Canny edge detection
               //          Canny(image, edges_image, 100, 200);
               //          imwrite("edges_image.jpg", edges_image);
               //          image = edges_image; // Replace the original image with the edge-detected version
               //      }
               //  }

               //  // Check if we need to resize the image
               //  if (request_json.contains("ResizeImage"))
               //  {
               //      int new_width = request_json["ResizeImage"]["width"];
               //      int new_height = request_json["ResizeImage"]["height"];
               //      resize(image, image, Size(new_width, new_height));
               //  }

               //  // Check if we need to blur the image
               //  if (request_json.contains("ApplyBlur"))
               //  {
               //      bool apply_blur = request_json["ApplyBlur"];
               //      if (apply_blur)
               //      {
               //          int blur_kernel_size = request_json["BlurKernelSize"];
               //          GaussianBlur(image, image, Size(blur_kernel_size, blur_kernel_size), 0);
               //      }
               //  }
               //  // Check if we need to rotate the image
               //  if (request_json.contains("RotateImage"))
               //  {
               //      double angle = request_json["RotateImage"]["angle"];
               //      Point2f center(image.cols / 2.0, image.rows / 2.0);
               //      Mat rotation_matrix = getRotationMatrix2D(center, angle, 1.0);
               //      warpAffine(image, image, rotation_matrix, image.size());
               //  }
               //  // Check if we need to adjust brightness and contrast
               //  if (request_json.contains("AdjustBrightnessContrast"))
               //  {
               //      int brightness = request_json["AdjustBrightnessContrast"]["brightness"];
               //      double contrast = request_json["AdjustBrightnessContrast"]["contrast"];
               //      image.convertTo(image, -1, contrast, brightness);
               //  }

               //  // Check if we need to apply sharpening
               //  if (request_json.contains("ApplySharpening"))
               //  {
               //      bool apply_sharpening = request_json["ApplySharpening"];
               //      if (apply_sharpening)
               //      {
               //          Mat sharpen_kernel = (Mat_<double>(3, 3) << 0, -1, 0,
               //                                -1, 5, -1,
               //                                0, -1, 0);
               //          filter2D(image, image, -1, sharpen_kernel);
               //      }
               //  }
               //  if (request_json.contains("EqualizeHistogram"))
               //  {
               //      bool equalize_histogram = request_json["EqualizeHistogram"];
               //      if (equalize_histogram)
               //      {
               //          Mat equalized_image;
               //          cvtColor(image, equalized_image, COLOR_BGR2GRAY); // Convert to grayscale first
               //          equalizeHist(equalized_image, equalized_image);
               //          imwrite("equalized_image.jpg", equalized_image);
               //          image = equalized_image; // Replace original image with the equalized version
               //      }
               //  }
               //  if (request_json.contains("ApplyGammaCorrection"))
               //  {
               //      double gamma = request_json["gamma"];
               //      Mat lut(1, 256, CV_8UC1);
               //      for (int i = 0; i < 256; i++)
               //      {
               //          lut.at<uchar>(i) = pow(i / 255.0, gamma) * 255.0;
               //      }
               //      LUT(image, lut, image);
               //  }
               //  if (request_json.contains("ApplyWatermark"))
               //  {
               //      std::string watermark_text = request_json["text"];
               //      int font_face = FONT_HERSHEY_SIMPLEX;
               //      double font_scale = 2.0;
               //      int thickness = 2;
               //      Point origin(image.cols - image.cols * 0.3, image.rows - image.rows * 0.5 );
               //      putText(image, watermark_text, origin, font_face, font_scale, Scalar(255, 255, 255), thickness);
               //  }
               //  if (request_json.contains("InvertColors"))
               //  {
               //      bool invert_colors = request_json["InvertColors"];
               //      if (invert_colors)
               //      {
               //          bitwise_not(image, image);
               //      }
               //  }
               //  if (request_json.contains("ApplySepia"))
               //  {
               //      bool apply_sepia = request_json["ApplySepia"];
               //      if (apply_sepia)
               //      {
               //          Mat sepia_image = (Mat_<float>(3, 3) << 1.272, 0.534, 0.131,
               //                             0.349, 1.686, 1.168,
               //                             0.393, 0.769, 1.189);
               //          transform(image, image, sepia_image);
               //      }
               //  }
               //  if (request_json.contains("ApplyMedianBlur"))
               //  {
               //      int kernel_size = request_json["MedianBlurKernelSize"];
               //      medianBlur(image, image, kernel_size);
               //  }
               //  if (request_json.contains("StretchHistogram"))
               //  {
               //      double min_val, max_val;
               //      minMaxLoc(image, &min_val, &max_val);
               //      image.convertTo(image, CV_8U, 255.0 / (max_val - min_val), -min_val * 255.0 / (max_val - min_val));
               //  }
               //  if (request_json.contains("ApplyUnsharpMask"))
               //  {
               //      double strength = request_json["UnsharpMaskStrength"];
               //      Mat blurred;
               //      GaussianBlur(image, blurred, Size(0, 0), 5);
               //      addWeighted(image, 1 + strength, blurred, -strength, 0, image);
               //  }
               //  if (request_json.contains("ApplyDilation"))
               //  {
               //      int kernel_size = request_json["DilationKernelSize"];
               //      Mat kernel = getStructuringElement(MORPH_RECT, Size(kernel_size, kernel_size));
               //      dilate(image, image, kernel);
               //  }
               //  if (request_json.contains("ApplyErosion"))
               //  {
               //      int kernel_size = request_json["ErosionKernelSize"];
               //      Mat kernel = getStructuringElement(MORPH_RECT, Size(kernel_size, kernel_size));
               //      erode(image, image, kernel);
               //  }
               //  if (request_json.contains("ApplyCLAHE"))
               //  {
               //      Ptr<CLAHE> clahe = createCLAHE();
               //      clahe->setClipLimit(request_json["CLAHEClipLimit"]);
               //      Mat lab_image;
               //      cvtColor(image, lab_image, COLOR_BGR2Lab);
               //      std::vector<Mat> lab_planes(3);
               //      split(lab_image, lab_planes);
               //      clahe->apply(lab_planes[0], lab_planes[0]);
               //      merge(lab_planes, lab_image);
               //      cvtColor(lab_image, image, COLOR_Lab2BGR);
               //  }

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