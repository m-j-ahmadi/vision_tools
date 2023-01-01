#ifndef SYNC_SERVER
#define SYNC_SERVER
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <stdio.h>
#include "../dep/json/include/nlohmann/json.hpp"
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
using boost::asio::ip::tcp;
namespace mj {
class sync_server
{
private:
    static void do_session(
    tcp::socket &socket,
    std::shared_ptr<std::string const> const &doc_root);
    /* data */
public:
   sync_server(/* args */);
   void run();
   ~sync_server();

};

}
#endif