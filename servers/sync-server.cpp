#include "sync-server.hpp"

using namespace cv;
using namespace std;
using json = nlohmann::json;
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
            std::cerr << _host << "," << _port<< e.what() << std::endl;          
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
        http::request<http::dynamic_body> req;
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
                // rec img
                std::cerr << "post: method " << req.body().size();
                FILE *pFile;
                const char *file_name = "./reciveid-from-client.jpeg";
                pFile = fopen(file_name, "w");
                fwrite(boost::beast::buffers_to_string(req.body().data()).data(), 1, req.body().size(), pFile);
                fclose(pFile);
                // show image
                Mat cedge, gray;
                auto image = imread(samples::findFile(file_name), IMREAD_COLOR);
                if (image.empty())
                {
                    printf("Cannot read image file: %s\n", file_name);
                    // help(argv);
                    // return -1;
                }
                nlohmann::json test;
                cedge.create(image.size(), image.type());
                cvtColor(image, gray, COLOR_BGR2GRAY);
                imwrite("gray.jpg", gray);
                // send back gray img
                http::file_body::value_type body;
                std::string path = "./gray.jpg";
                body.open(path.c_str(), boost::beast::file_mode::read, err);
                auto const size = body.size();
                std::cerr << "GRAY IMG SIZE = " << size;

                http::response<http::file_body> res{std::piecewise_construct,
                                                    std::make_tuple(std::move(body)),
                                                    std::make_tuple(http::status::ok, req.version())};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "image/jpeg");
                res.content_length(size);
                res.keep_alive(req.keep_alive());
                http::write(socket, res, err);
            }
        }
        else if (req.target() == "/stream")
        {
            cv::Mat frame;
            std::vector<uchar> buffer;
            cv::Mat grayframe;
            cv::VideoCapture cap("./file_example_MP4_480_1_5MG.mp4");

            if (!cap.isOpened())
            {
                std::cerr << "Err opening video stream!!!";
                // return 1;
            }

            http::response<http::empty_body> res{http::status::ok, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "multipart/x-mixed-replace; boundary=frame");
            res.keep_alive();
            http::response_serializer<http::empty_body> sr{res};
            http::write_header(socket, sr);

            while (cap.isOpened())
            {
                cap >> frame;
                if (!frame.empty())
                {

                    cv::cvtColor(frame, grayframe, cv::COLOR_BGR2GRAY);
                    // cv:imshow("frame", frame);
                    cv::waitKey(500);

                    cv::imencode(".jpg", grayframe, buffer, std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
                    auto const size = buffer.size();

                    // do not use http::response<>
                    // hack: write to socket the multipart message
                    std::string message{"\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: "};
                    message += std::to_string(size);
                    message += "\r\n\r\n";
                    auto bytesSent = socket.send(boost::asio::buffer(message), 0, err);
                    if (!bytesSent)
                    {
                        break;
                    }
                    bytesSent = socket.send(boost::asio::buffer(buffer), 0, err);
                    if (!bytesSent)
                    {
                        break;
                    }
                }
            }
        }
    }

    // Send a TCP shutdown
    socket.shutdown(tcp::socket::shutdown_send, err);
}