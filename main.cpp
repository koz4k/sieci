#include "MeasurementManager.hpp"
#include "TelnetServer.hpp"
#include "TcpMeasurer.hpp"
#include "ServiceDiscoverer.hpp"
#include "UdpServer.hpp"
#include "UdpMeasurer.hpp"
#include "IcmpMeasurer.hpp"
#include "io.hpp"
#include "options.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    if(!parseOptions(argc, argv))
    {
        std::cerr << "Usage: " << argv[0] << " [-u port] [-U port] [-t seconds]"
                  << " [-T seconds] [-v seconds] [-s]" << std::endl;
        return 0;
    }

    //try
    {
        MeasurementManager mm({
                MeasurementService("_opoznienia._udp", UdpMeasurer::create),
                MeasurementService("_ssh._tcp", TcpMeasurer::create,
                        !announceTcp),
                MeasurementService("_opoznienia._udp", IcmpMeasurer::create,
                        true),
        });
        ServiceDiscoverer d("hUbBaBuBbA", mm);
        TelnetServer t(mm);
        UdpServer us;
        io.run();
    }
    /*catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }*/

    return 0;
}
