#ifndef TELNET_SESSION_HPP
#define TELNET_SESSION_HPP

#include "TelnetServer.hpp"
#include <boost/asio.hpp>
#include <array>

class TelnetServer;

class TelnetSession
{
  public:
    TelnetSession(TelnetServer& server, boost::asio::ip::tcp::socket socket);
    void updateScreen();

  private:
    void read_();
    void onRead_(const boost::system::error_code& error, size_t len);
    void error_(const char* action,
                const boost::system::error_code& error);

    TelnetServer& server_;
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 16> buffer_;
    bool ready_;
    int line_;
};

#endif
