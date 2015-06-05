#include "UdpMeasurer.hpp"
#include "io.hpp"
#include "utils.hpp"
#include "options.hpp"
#include <iostream>
#include <cstring>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::udp;

UdpMeasurer::UdpMeasurer(MeasurementCollector& collector, int typeIndex):
    Measurer(collector, typeIndex),
    endpoint_(ip::address::from_string(collector.getAddress()), udpPort),
    socket_(io, udp::v4())
{
}

std::unique_ptr<Measurer> UdpMeasurer::create(MeasurementCollector& collector,
        int typeIndex)
{
    return std::unique_ptr<Measurer>(new UdpMeasurer(collector, typeIndex));
}

void UdpMeasurer::startMeasurement_()
{
    uint64_t t = htonll(microtime());
    auto toSend = std::make_shared<std::array<uint8_t, 8>>();
    memcpy(&(*toSend)[0], &t, 8);

    socket_.async_send_to(buffer(*toSend), endpoint_,
            [this, toSend](const error_code& error, size_t len)
            {
                if(!error)
                {
                    auto received = std::make_shared<std::array<uint8_t, 16>>();
                    socket_.async_receive_from(buffer(*received), endpoint_,
                            [this, received](const error_code& error,
                                    size_t len)
                            {
                                if(!error)
                                {
                                    if(len == 16)
                                    {
                                        uint64_t t = 0;
                                        memcpy(&t, &(*received)[0], 8);
                                        t = ntohll(t);
                                        endMeasurement_(microtime() - t);
                                    }
                                    else
                                    {
                                        std::cerr << "UdpMeasurer assigned to "
                                                  << endpoint_.address()
                                                  << ", got " << len
                                                  << " bytes, expected 16"
                                                  << std::endl;
                                    }
                                }
                                else
                                {
                                    std::cerr << "UdpMeasurer assigned to "
                                              << endpoint_.address().to_string()
                                              << ", receive error: "
                                              << error.message() << std::endl;
                                }
                            });
                }
                else
                {
                    std::cerr << "UdpMeasurer assigned to "
                              << endpoint_.address().to_string()
                              << ", send error: " << error.message()
                              << std::endl;
                }
            });
}
