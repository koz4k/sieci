#ifndef TELNET_SESSION_HPP
#define TELNET_SESSION_HPP

#include <boost/asio.hpp>

class TelnetSession
{
  public:
    TelnetSession(boost::asio::ip::tcp::socket&& socket);

  private:
    void read_();
    void onRead_(const boost::system::error_code& error, size_t len);
    void onWrite_(const boost::system::error_code& error);
    void error_(const char* action,
                const boost::system::error_code& error);

    boost::asio::ip::tcp::socket socket_;
    char data_[256];
};

#endif
