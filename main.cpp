#include "MeasurementManager.hpp"
#include "TelnetServer.hpp"
#include "TcpMeasurer.hpp"
#include "io.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    //try
    {
        MeasurementManager mm({MeasurementType(TcpMeasurer::create)});

        mm.startCollecting("0.0.0.0");
        mm.startCollecting("127.0.0.1");

        TelnetServer server(mm, 2015);
        io.run();
    }
    /*catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }*/

    return 0;
}
