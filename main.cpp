#include "MeasurementManager.hpp"
#include "TelnetServer.hpp"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;

int main(int argc, char** argv)
{
    try
    {
        MeasurementManager mm(3);

        MeasurementCollector& c1 = mm.startCollecting("127.0.0.1");
        c1.collect(0, 123);
        c1.collect(0, 0);
        c1.collect(1, 123);
        c1.collect(2, 123);
        c1.collect(1, 12);

        MeasurementCollector& c2 = mm.startCollecting("kurzastopa.pl");
        c2.collect(0, 321);
        c2.collect(2, 23);

        mm.startCollecting("hubbabubbabubbabubba");

        for(char c = 'a'; c <= 'z'; ++c)
            mm.startCollecting(std::string(16, c)).collect(1, 'z' - c + 1);

        io_service io;
        TelnetServer server(mm, io, 2015);
        io.run();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
