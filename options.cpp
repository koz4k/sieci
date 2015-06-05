#include "options.hpp"
#include <string>
#include <sstream>

int udpPort = 3382;
int telnetPort = 3637;
double measurementPeriod = 1;
double discoveryPeriod = 10;
double uiRefreshPeriod = 1;
bool announceTcp = false;

bool parseOptions(int argc, char* argv[])
{
    int i = 1;
    while(i < argc)
    {
        std::string option = argv[i++];
        if(option == "-s")
            announceTcp = true;
        else
        {
            if(i == argc)
                return false;

            std::stringstream ss;
            ss << argv[i++];
            if(option == "-u")
                ss >> udpPort;
            else if(option == "-U")
                ss >> telnetPort;
            else if(option == "-t")
                ss >> measurementPeriod;
            else if(option == "-T")
                ss >> discoveryPeriod;
            else if(option == "-v")
                ss >> uiRefreshPeriod;
            else
                return false;
        }
    }

    return true;
}
