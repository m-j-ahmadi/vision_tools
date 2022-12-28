#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <stdio.h>
#include "dep/json/include/nlohmann/json.hpp"
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <cstdlib>

#include <ctime>
using namespace cv;
using namespace std;
using json = nlohmann::json;
using boost::asio::ip::tcp;
namespace http = boost::beast::http;

int edgeThresh = 1;
int edgeThreshScharr = 1;
Mat image, gray, blurImage, edge1, edge2, cedge;
const char *window_name1 = "Edge map : Canny default (Sobel gradient)";
const char *window_name2 = "Edge map : Canny with custom gradient (Scharr)";
// define a trackbar callback
static void onTrackbar(int, void *)
{
    blur(gray, blurImage, Size(3, 3));
    // Run the edge detector on grayscale
    Canny(blurImage, edge1, edgeThresh, edgeThresh * 3, 3);
    cedge = Scalar::all(0);
    image.copyTo(cedge, edge1);
    imshow(window_name1, cedge);
    Mat dx, dy;
    Scharr(blurImage, dx, CV_16S, 1, 0);
    Scharr(blurImage, dy, CV_16S, 0, 1);
    Canny(dx, dy, edge2, edgeThreshScharr, edgeThreshScharr * 3);
    cedge = Scalar::all(0);
    image.copyTo(cedge, edge2);
    imshow(window_name2, cedge);
}
static void help(const char **argv)
{
    printf("\nThis sample demonstrates Canny edge detection\n"
           "Call:\n"
           "    %s [image_name -- Default is fruits.jpg]\n\n",
           argv[0]);
}
const char *keys =
    {
        "{help h||}{@image |fruits.jpg|input image name}"};

int main(int argc, const char **argv)
{

    // // create an array value
    // json array = {1, 2, 3, 4, 5};

    // // get an iterator to the first element
    // json::iterator it = array.begin();

    // // serialize the element that the iterator points to
    // std::cout << *it << '\n';

    // help(argv);
    // CommandLineParser parser(argc, argv, keys);
    // string filename = parser.get<string>(0);
    // image = imread(samples::findFile(filename), IMREAD_COLOR);
    // if(image.empty())
    // {
    //     printf("Cannot read image file: %s\n", filename.c_str());
    //     help(argv);
    //     return -1;
    // }
    // nlohmann::json test;
    // cedge.create(image.size(), image.type());
    // cvtColor(image, gray, COLOR_BGR2GRAY);
    // imwrite("gray.png", gray);
    // // Create a window
    // namedWindow(window_name1, 1);
    // namedWindow(window_name2, 1);
    // // create a toolbar
    // createTrackbar("Canny threshold default", window_name1, &edgeThresh, 100, onTrackbar);
    // createTrackbar("Canny threshold Scharr", window_name2, &edgeThreshScharr, 400, onTrackbar);
    // // Show the image
    // //onTrackbar(0, 0);
    // // Wait for a key stroke; the same function arranges events processing
    // waitKey(0);

    try
    {
        boost::asio::io_context io_service;
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1112));
        boost::system::error_code err;

        for (;;)
        {
            tcp::socket socket(io_service);
            acceptor.accept(socket);
            boost::beast::flat_buffer buffer;
            http::request<http::dynamic_body> req;
            http::read(socket, buffer, req, err);
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
                image = imread(samples::findFile(file_name), IMREAD_COLOR);
                if (image.empty())
                {
                    printf("Cannot read image file: %s\n", file_name);
                    help(argv);
                    return -1;
                }
                nlohmann::json test;
                cedge.create(image.size(), image.type());
                cvtColor(image, gray, COLOR_BGR2GRAY);
                imwrite("gray.png", gray);
                // Create a window
                namedWindow(window_name1, 1);
                namedWindow(window_name2, 1);
                // create a toolbar
                createTrackbar("Canny threshold default", window_name1, &edgeThresh, 100, onTrackbar);
                createTrackbar("Canny threshold Scharr", window_name2, &edgeThreshScharr, 400, onTrackbar);
                // Show the image
                // onTrackbar(0, 0);
                // Wait for a key stroke; the same function arranges events processing
                waitKey(0);
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
