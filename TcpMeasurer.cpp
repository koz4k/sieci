#include "TcpMeasurer.hpp"
#include "io.hpp"
#include <iostream>

using namespace boost::asio;
using namespace boost::system;

TcpMeasurer::TcpMeasurer(MeasurementCollector& collector, int typeIndex):
    Measurer(collector, typeIndex),
    endpoint_(ip::address::from_string(collector.getAddress()), 22),
    socket_(io)
{
}

std::unique_ptr<Measurer> TcpMeasurer::create(MeasurementCollector& collector,
        int typeIndex)
{
    return std::unique_ptr<Measurer>(new TcpMeasurer(collector, typeIndex));
}

void TcpMeasurer::startMeasurement_()
{
    socket_.async_connect(endpoint_,
            [this](const error_code& error)
            {
                if(!error)
                    endMeasurement_();
                else
                    std::cerr << "TcpMeasurer assigned to "
                              << endpoint_.address().to_string()
                              << ", connect error: " << error.message()
                              << std::endl;

                socket_.close();
            });
}
