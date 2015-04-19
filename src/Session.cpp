#include <iostream>
#include <glog/logging.h>

#include "Storage.h"
#include "Session.h"

using boost::asio::ip::tcp;

Session::Session(Storage& storage, boost::asio::ip::tcp::socket socket) :
        m_storage(storage),
        m_socket(std::move(socket))
{

}

void Session::start()
{
    doRead();
}

void Session::doRead()
{
    auto self(shared_from_this());
    boost::asio::async_read_until(m_socket, m_inputStream, "\n",
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
            {
                if(ec)
                {
                    LOG(ERROR)<<ec.message();
                    return;
                }
                read();
            });
}

void Session::read()
{
    std::istream is(&m_inputStream);
    is.exceptions(std::istream::failbit | std::istream::badbit);

    bool readMore = true;
    try
    {
        while (!is.eof())
        {

            std::string command;
            is >> command;
            if (command == "add")
            {
                std::string meas;
                size_t timestamp = 0;
                double value = 0;
                is >> meas >> timestamp >> value;
                addMeasurement(meas, timestamp, value);
            }
            else if (command == "get_sequence")
            {
                std::string meas;
                time_t from = 0;
                time_t to = 0;
                is >> meas >> from >> to;
                getSequence(meas, from, to);
            }
            if (command == "exit")
            {
                readMore = false;
                exit();
            }
        }
    } catch (...)
    {
    }

    if(readMore)
    {
        doRead();
    }
}

void Session::addMeasurement(const std::string& meas, time_t timestamp,
        double value)
{
    auto muid = m_storage.addMetric(meas);
    if (!m_storage.put(muid, timestamp, value))
    {
        LOG(ERROR)<<"Error put: "<<meas<<" time: "<<timestamp<<" value: "<<value;
    }
}

void Session::getSequence(const std::string& meas, time_t from, time_t to)
{
    const auto muid = m_storage.addMetric(meas);
    m_iter = m_storage.get(muid, from, to);
    auto self(shared_from_this());
    std::ostream os(&m_outputStream);
    if (m_iter.valid())
    {
        time_t timestamp = 0;
        double val = 0;
        std::tie(timestamp, val) = m_iter.value();
        os << timestamp << " " << val << std::endl;
        os.flush();

        boost::asio::async_write(m_socket, m_outputStream,
                [self, this](boost::system::error_code ec, std::size_t /*length*/)
                {
                    if (ec)
                    {
                        LOG(ERROR)<<ec.message();
                        return;
                    }
                    sayNextValue();
                });
    }
    else
    {
        os << "done" << std::endl;
        os.flush();
        boost::asio::async_write(m_socket, m_outputStream,
                [this, self](boost::system::error_code ec, std::size_t /*length*/)
                {
                    if (ec)
                    {
                        LOG(ERROR)<<ec.message();
                    }
                });
    }
}

void Session::sayNextValue()
{
    auto self(shared_from_this());
    std::ostream os(&m_outputStream);
    if (m_iter.valid())
    {
        m_iter.next();
        time_t timestamp = 0;
        double val = 0;
        std::tie(timestamp, val) = m_iter.value();
        os << timestamp << " " << val << std::endl;
        os.flush();

        boost::asio::async_write(m_socket, m_outputStream,
                [self, this](boost::system::error_code ec, std::size_t /*length*/)
                {
                    if (ec)
                    {
                        LOG(ERROR)<<ec.message();
                        return;
                    }
                    sayNextValue();
                });
    }
    else
    {
        os << "done" << std::endl;
        os.flush();
        boost::asio::async_write(m_socket, m_outputStream,
                [this, self](boost::system::error_code ec, std::size_t /*length*/)
                {
                    if (ec)
                    {
                        LOG(ERROR)<<ec.message();
                        return;
                    }
                });
    }
}

void Session::exit()
{
    auto self(shared_from_this());

    std::ostream os(&m_outputStream);
    os << "exit";
    os.flush();
    m_socket.shutdown(boost::asio::socket_base::shutdown_receive);

    boost::asio::async_write(m_socket, m_outputStream,
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (ec)
                {
                    LOG(ERROR)<<"Error exit";
                }
                m_socket.close();
            });
}

