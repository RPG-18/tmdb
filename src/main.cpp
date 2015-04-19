#include <cstdint>
#include <iostream>

#include <getopt.h>
#include <boost/asio.hpp>

#include <glog/logging.h>

#include "Server.h"

int main(int argc, char* argv[])
{
    struct option long_options[] =
    {
        {"port", required_argument, NULL, 'p' },
        {"data", required_argument, NULL, 'd' },
        {"cache",required_argument, NULL, 'c'},
        {0, 0, 0, 0 }
    };

    std::string data;
    size_t cacheSize = 16; //16Mb
    uint16_t port = 0;

    int opt = 0;
    while ((opt = getopt(argc, argv, "p:d:c:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            port = std::atoi(optarg);
            break;
        case 'c':
            cacheSize = std::atoi(optarg);
            break;
        case 'd':
            data = std::string(optarg);
            break;
        }
    }
    google::InitGoogleLogging(argv[0]);
    try
    {

        boost::asio::io_service io_service;

        Server s(io_service, port, data, cacheSize);

        io_service.run();
    } catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
