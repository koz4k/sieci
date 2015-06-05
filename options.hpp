#ifndef OPTIONS_HPP
#define OPTIONS_HPP

extern int udpPort;
extern int telnetPort;
extern double measurementPeriod;
extern double discoveryPeriod;
extern double uiRefreshPeriod;
extern bool announceTcp;

bool parseOptions(int argc, char* argv[]);

#endif
