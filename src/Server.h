#pragma once

#include <cstdint>
#include <boost/asio.hpp>

#include "Storage.h"

using boost::asio::ip::tcp;

class Server
{
public:

    Server(boost::asio::io_service& io_service, short port, const std::string& data, size_t cacheSize = 16);

private:

    void doAcept();

private:
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    Storage m_storage;
};

