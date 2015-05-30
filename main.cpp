#include "MeasurementManager.hpp"
#include "TelnetServer.hpp"
#include "TcpMeasurer.hpp"
#include "ServiceDiscoverer.hpp"
#include "io.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    //try
    {
        MeasurementManager mm({MeasurementService(TcpMeasurer::create)});

        ServiceDiscoverer d(mm);

        TelnetServer t(mm, 2015);
        io.run();
    }
    /*catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }*/

    return 0;
}
