#include "MeasurementCollector.hpp"
#include "TelnetServer.hpp"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;

int main(int argc, char** argv)
{
    try
    {
        MeasurementCollector mc;

        Measurements& m1 = mc.collect("127.0.0.1");
        m1.addUdp(123);
        m1.addUdp(0);
        m1.addTcp(123);
        m1.addIcmp(123);
        m1.addTcp(12);

        Measurements& m2 = mc.collect("kurzastopa.pl");
        m2.addUdp(321);
        m2.addIcmp(23);

        mc.collect("hubbabubbabubbabubba");

        for(char c = 'a'; c <= 'z'; ++c)
            mc.collect(std::string(16, c)).addTcp('z' - c + 1);

        io_service io;
        TelnetServer server(mc, io, 2015);
        io.run();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
