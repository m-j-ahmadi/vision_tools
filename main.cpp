#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <stdio.h>
#include "dep/json/include/nlohmann/json.hpp"
#include <iostream>
#include "/home/ahmadi/Codes/vision-tools/dep/boost_lib/libs/asio/include/boost/asio.hpp"
#include "/home/ahmadi/Codes/vision-tools/dep/boost_lib/libs/bind/include/boost/bind.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

#include <ctime>
using namespace cv;
using namespace std;
using json = nlohmann::json;
int edgeThresh = 1;
int edgeThreshScharr=1;
Mat image, gray, blurImage, edge1, edge2, cedge;
const char* window_name1 = "Edge map : Canny default (Sobel gradient)";
const char* window_name2 = "Edge map : Canny with custom gradient (Scharr)";
// define a trackbar callback
static void onTrackbar(int, void*)
{
    blur(gray, blurImage, Size(3,3));
    // Run the edge detector on grayscale
    Canny(blurImage, edge1, edgeThresh, edgeThresh*3, 3);
    cedge = Scalar::all(0);
    image.copyTo(cedge, edge1);
    imshow(window_name1, cedge);
    Mat dx,dy;
    Scharr(blurImage,dx,CV_16S,1,0);
    Scharr(blurImage,dy,CV_16S,0,1);
    Canny( dx,dy, edge2, edgeThreshScharr, edgeThreshScharr*3 );
    cedge = Scalar::all(0);
    image.copyTo(cedge, edge2);
    imshow(window_name2, cedge);
}
static void help(const char** argv)
{
    printf("\nThis sample demonstrates Canny edge detection\n"
           "Call:\n"
           "    %s [image_name -- Default is fruits.jpg]\n\n", argv[0]);
}
const char* keys =
{
    "{help h||}{@image |fruits.jpg|input image name}"
};

void print(const boost::system::error_code& /*e*/,
    boost::asio::deadline_timer* t, int* count)
{
  if (*count < 5)
  {
    std::cout << *count << std::endl;
    ++(*count);

    t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
    t->async_wait(boost::bind(print,
          boost::asio::placeholders::error, t, count));
  }
}

int main( int argc, const char** argv )
{
    
   boost::asio::io_context io;

  int count = 0;
  boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));
  t.async_wait(boost::bind(print,
        boost::asio::placeholders::error, &t, &count));

  io.run();

  std::cout << "Final count is " << count << std::endl;
    
    // create an array value
    json array = {1, 2, 3, 4, 5};

    // get an iterator to the first element
    json::iterator it = array.begin();

    // serialize the element that the iterator points to
    std::cout << *it << '\n';
    
    help(argv);
    CommandLineParser parser(argc, argv, keys);
    string filename = parser.get<string>(0);
    image = imread(samples::findFile(filename), IMREAD_COLOR);
    if(image.empty())
    {
        printf("Cannot read image file: %s\n", filename.c_str());
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
    //onTrackbar(0, 0);
    // Wait for a key stroke; the same function arranges events processing
    waitKey(0);
    return 0;
}
