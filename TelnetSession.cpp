#include "TelnetSession.hpp"
#include <boost/bind.hpp>
#include <utility>
#include <iostream>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;

TelnetSession::TelnetSession(tcp::socket&& socket):
    socket_(std::move(socket))
{
    read_();
}

void TelnetSession::read_()
{
    socket_.async_read_some(buffer(data_, 256),
            boost::bind(&TelnetSession::onRead_, this,
                        placeholders::error,
                        placeholders::bytes_transferred));
}

void TelnetSession::onRead_(const error_code& error, size_t len)
{
    if(error)
    {
        error_("read", error);
        return;
    }

    async_write(socket_, buffer(data_, len),
                boost::bind(&TelnetSession::onWrite_, this,
                            placeholders::error));
}

void TelnetSession::onWrite_(const error_code& error)
{
    if(error)
    {
        error_("write", error);
        return;
    }

    read_();
}

void TelnetSession::error_(const char* action, const error_code& error)
{
    std::cerr << "TelnetSession with "
              << socket_.remote_endpoint().address().to_string() << ", "
              << action << " error: " << error << std::endl;
    delete this;
}
