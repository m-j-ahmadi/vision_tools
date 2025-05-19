#ifndef SYNC_SERVER
#define SYNC_SERVER

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
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>




using boost::asio::ip::tcp;
namespace http = boost::beast::http;
using boost::asio::ip::tcp;
namespace mj {
class SyncServer
{
private:
    static void do_session(
    tcp::socket &socket,
    std::shared_ptr<std::string const> const &doc_root);
    std::string _host;
    std::string _port;
    /* data */
public:
   SyncServer(std::string host, std::string port);
   void run();
   ~SyncServer();

};

}
#endif