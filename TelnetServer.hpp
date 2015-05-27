#ifndef TELNET_SERVER_HPP
#define TELNET_SERVER_HPP

#include "MeasurementCollector.hpp"
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

class TelnetServer
{
  public:
    TelnetServer(MeasurementCollector& mc, boost::asio::io_service& io,
                 uint16_t port);
    size_t getScreenHeight() const { return screen_.size(); }
    std::string getScreen(size_t startLine);

  private:
    void accept_();
    void onAccept_(const boost::system::error_code& error);
    void updateScreen_();

    MeasurementCollector& mc_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    std::vector<std::string> screen_;
    boost::asio::deadline_timer timer_;
};

#endif
