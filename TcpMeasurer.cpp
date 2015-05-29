#include "TcpMeasurer.hpp"
#include "io.hpp"
#include <iostream>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::tcp;

TcpMeasurer::TcpMeasurer(MeasurementCollector& collector, int typeIndex):
    Measurer(collector, typeIndex), resolver_(io), socket_(io)
{
    tcp::resolver::query query(tcp::v4(), collector.getAddress(), "22");
    resolver_.async_resolve(query,
            [this](const error_code& error, tcp::resolver::iterator eit)
            {
                if(error)
                {
                    std::cerr << "TcpMeasurer, resolve error: "
                              << error.message() << std::endl;
                }
                else
                    eit_ = eit;
            });
}

std::unique_ptr<Measurer> TcpMeasurer::create(MeasurementCollector& collector,
        int typeIndex)
{
    return std::unique_ptr<Measurer>(new TcpMeasurer(collector, typeIndex));
}

void TcpMeasurer::startMeasurement_()
{
    if(eit_ == tcp::resolver::iterator())
        return;

    async_connect(socket_, eit_,
            [this](const error_code& error, tcp::resolver::iterator eit)
            {
                if(error)
                {
                    std::cerr << "TcpMeasurer, connect error: "
                              << error.message() << std::endl;
                }
                else
                {
                    endMeasurement_();
                    socket_.close();
                    eit_ = eit;
                }
            });
}
