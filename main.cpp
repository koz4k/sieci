#include "MeasurementManager.hpp"
#include "TelnetServer.hpp"
#include "DummyMeasurer.hpp"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;

int main(int argc, char** argv)
{
    try
    {
        MeasurementManager mm({MeasurementType(DummyMeasurer::create),
                MeasurementType(DummyMeasurer::create)});

        mm.startCollecting("127.0.0.1");
        mm.measure();
        mm.startCollecting("kurzastopa.pl");
        mm.measure();
        mm.startCollecting("hubbabubbabubbabubba");
        mm.measure();
        for(char c = 'a'; c <= 'z'; ++c)
            mm.startCollecting(std::string(16, c));
        mm.measure();

        io_service io;
        TelnetServer server(mm, io, 2015);
        io.run();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
