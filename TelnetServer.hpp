#ifndef TELNET_SERVER_HPP
#define TELNET_SERVER_HPP

#include "MeasurementManager.hpp"
#include "TelnetSession.hpp"
#include "io.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <cstdint>

class TelnetSession;

class TelnetServer
{
  public:
    TelnetServer(MeasurementManager& mm, uint16_t port);
    int getScreenHeight() const { return screen_.size(); }
    std::string getScreen(int startLine) const;
    void endSession(TelnetSession& session);

  private:
    void accept_();
    void onAccept_(const boost::system::error_code& error);
    void updateScreen_();

    std::unordered_map<TelnetSession*,
                       std::unique_ptr<TelnetSession>> sessions_;
    MeasurementManager& mm_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    std::vector<std::string> screen_;
    boost::asio::deadline_timer timer_;
};

#endif
