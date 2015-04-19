#pragma once

#include <memory>
#include <boost/asio.hpp>

class Session: public std::enable_shared_from_this<Session>
{
public:

    Session(Storage& storage, boost::asio::ip::tcp::socket socket);

    void start();

private:

    void doRead();

    void addMeasurement(const std::string& meas, time_t timestamp, double value);

    void getSequence(const std::string& meas, time_t frmo, time_t to);

    void sayNextValue();

    void exit();

    void read();
private:

    Storage& m_storage;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::streambuf m_inputStream;
    boost::asio::streambuf m_outputStream;
    Storage::Iterator m_iter;
};

