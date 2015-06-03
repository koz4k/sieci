#include "MeasurementManager.hpp"
#include "TelnetServer.hpp"
#include "TcpMeasurer.hpp"
#include "ServiceDiscoverer.hpp"
#include "UdpServer.hpp"
#include "UdpMeasurer.hpp"
#include "io.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    //try
    {
        MeasurementManager mm({
                MeasurementService("_ssh._tcp", TcpMeasurer::create),
                MeasurementService("_opoznienia._udp", UdpMeasurer::create),
        });
        ServiceDiscoverer d("hubbabubba", mm);
        TelnetServer t(mm, 2015);
        UdpServer us;
        io.run();
    }
    /*catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }*/

    return 0;
}
