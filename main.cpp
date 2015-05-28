#include "MeasurementCollector.hpp"
#include "TelnetServer.hpp"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;

int main(int argc, char** argv)
{
    try
    {
        MeasurementCollector mc(3);

        Measurements& m1 = mc.startCollecting("127.0.0.1");
        m1.collect(0, 123);
        m1.collect(0, 0);
        m1.collect(1, 123);
        m1.collect(2, 123);
        m1.collect(1, 12);

        Measurements& m2 = mc.startCollecting("kurzastopa.pl");
        m2.collect(0, 321);
        m2.collect(2, 23);

        mc.startCollecting("hubbabubbabubbabubba");

        for(char c = 'a'; c <= 'z'; ++c)
            mc.startCollecting(std::string(16, c)).collect(1, 'z' - c + 1);

        io_service io;
        TelnetServer server(mc, io, 2015);
        io.run();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
