#include "UdpServer.hpp"
#include "io.hpp"
#include "utils.hpp"
#include "options.hpp"
#include <iostream>
#include <cstring>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::udp;

UdpServer::UdpServer():
    socket_(io, udp::endpoint(udp::v4(), udpPort))
{
    receive_();
}

void UdpServer::receive_()
{
    socket_.async_receive_from(buffer(buffer_), senderEndpoint_,
            std::bind(&UdpServer::onReceive_, this, std::placeholders::_1,
            std::placeholders::_2));
}

void UdpServer::onReceive_(const error_code& error, size_t len)
{
    if(!error)
    {
        if(len == 8)
        {
            auto data = std::make_shared<std::array<uint8_t, 16>>();
            memcpy(&(*data)[0], &buffer_[0], 8);
            uint64_t t = htonll(microtime());
            memcpy(&(*data)[8], &t, 8);

            socket_.async_send_to(buffer(*data), senderEndpoint_,
                    [data](const error_code& error, size_t len)
                    {
                        if(error)
                        {
                            std::cerr << "UdpServer, send error: "
                                      << error.message() << std::endl;
                        }
                    });
        }
        else
        {
            std::cerr << "UdpServer, got " << len << " bytes, expected 8"
                      << std::endl;
        }
    }
    else
    {
        std::cerr << "UdpServer, receive error: " << error.message()
                  << std::endl;
    }

    receive_();
}
