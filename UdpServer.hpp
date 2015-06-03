#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP

#include <boost/asio.hpp>

class UdpServer
{
  public:
    UdpServer();

  private:
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint senderEndpoint_;
    std::array<uint8_t, 8> buffer_;

    void receive_();
    void onReceive_(const boost::system::error_code& error, size_t len);
};

#endif
