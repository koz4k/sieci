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

        Measurements* ms = mc.collect("127.0.0.1");
        ms->addUdp(123);
        ms->addUdp(0);
        ms->addTcp(123);
        ms->addIcmp(123);
        ms->addTcp(12);

        ms = mc.collect("kurzastopa.pl");
        ms->addUdp(321);
        ms->addIcmp(23);

        ms = mc.collect("hubbabubbabubbabubba");

        io_service io;
        TelnetServer server(mc, io, 2015);
        io.run();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
