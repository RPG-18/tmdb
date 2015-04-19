#include "Server.h"
#include "Session.h"

Server::Server(boost::asio::io_service& io_service, short port,
        const std::string& data, size_t cacheSize) :
        m_acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
        m_socket(io_service),
        m_storage(data, cacheSize)
{
    doAcept();
}

void Server::doAcept()
{
    m_acceptor.async_accept(m_socket, [this](boost::system::error_code ec)
    {
        if (!ec)
        {
            std::make_shared<Session>(m_storage, std::move(m_socket))->start();
        }

        doAcept();
    });
}
