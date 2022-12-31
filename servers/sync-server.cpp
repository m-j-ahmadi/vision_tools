#include "sync-server.hpp"

using namespace cv;
using namespace std;
using json = nlohmann::json;
using boost::asio::ip::tcp;
using namespace mj;

namespace http = boost::beast::http;
 SyncServer::SyncServer(/* args */)
{

}

 SyncServer::~SyncServer()
{
}

void SyncServer::run()
{
    auto const address = boost::asio::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi("1112"));
    auto const doc_root = std::make_shared<std::string>("/");
     try
    {
        boost::asio::io_context ioc;
        tcp::acceptor acceptor{ioc, {address, port}};
        
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
        std::cerr << e.what() << std::endl;
        //return 1;
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

    // Send a TCP shutdown
    socket.shutdown(tcp::socket::shutdown_send, err);
}