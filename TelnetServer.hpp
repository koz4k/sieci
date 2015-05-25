#include <boost/asio.hpp>
#include <cstdint>

class TelnetServer
{
  public:
    TelnetServer(boost::asio::io_service& io, uint16_t port);

  private:
    void accept_();
    void onAccept_(const boost::system::error_code& error);

    boost::asio::io_service& io_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
};
