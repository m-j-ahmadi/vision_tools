//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, synchronous
//
//------------------------------------------------------------------------------

//[example_http_client

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "../dep/json/include/nlohmann/json.hpp"
#include <cppcodec/base64_rfc4648.hpp>
#include <string>
#define POST

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using json = nlohmann::json;
using base64 = cppcodec::base64_rfc4648;
// Performs an HTTP GET and prints the response
int main(int argc, char **argv)
{
    try
    {
        // Check command line arguments.
        if (argc != 4 && argc != 5)
        {
            std::cerr << "Usage: http-client-sync <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n"
                      << "Example:\n"
                      << "    http-client-sync www.example.com 80 /\n"
                      << "    http-client-sync www.example.com 80 / 1.0\n";
            return EXIT_FAILURE;
        }
        auto const host = argv[1];
        auto const port = argv[2];
        auto const target = argv[3];
        int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);
         tcp::socket socket(ioc);
        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup

        try
        {
            auto x = net::connect(socket, results.begin(), results.end());
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
            return 1;
        }

#ifdef POST
        // Set up an HTTP POST request message
        // send img
        std::ifstream file(target, std::ios::binary);
        std::vector<unsigned char> image_data((std::istreambuf_iterator<char>(file)),
                                               std::istreambuf_iterator<char>());
        std::string encoded_image = base64::encode(image_data);

        // Create the JSON body
        json json_body;
        json_body["img"] = encoded_image;
        json_body["ConvertColorToGray"] = true;  // Set flag to convert to grayscale

        // Serialize JSON to string
        std::string body = json_body.dump();

        // Create HTTP request
        http::request<http::string_body> req{http::verb::post, "/", 11};
        req.set(http::field::host, host);
        req.set(http::field::content_type, "application/json");
        req.body() = body;
        req.content_length(body.size());

        // Send the request
        http::write(socket, req);
        
        // Receive the HTTP response

             // Receive the response
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(socket, buffer, res);

        std::cout << res << std::endl;

        // Save the response image (assumed base64 encoded)
        json response_json = json::parse(beast::buffers_to_string(res.body().data()));
        std::string response_image_base64 = response_json["processed_image"];
        std::vector<unsigned char> decoded_image = base64::decode(response_image_base64);

        std::ofstream output_file("received_gray_image.jpg", std::ios::binary);
        output_file.write(reinterpret_cast<const char*>(decoded_image.data()), decoded_image.size());
        output_file.close();

#endif
#ifdef GET
        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Write the message to standard out
        std::cout << res.body().size() << std::endl;
        // res.body.data();
        FILE *pFile;
        pFile = fopen("./myfile.jpeg", "w");
        fwrite(boost::beast::buffers_to_string(res.body().data()).data(), 1, res.body().size(), pFile);
        fclose(pFile);
#endif
        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if (ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//]
