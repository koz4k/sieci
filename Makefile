TARGET: opoznienia

CC	= g++
CFLAGS	= -std=c++11 -O2
LFLAGS	= -lboost_system -lpthread

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

opoznienia: DnsMessage.o io.o main.o MeasurementCollector.o MeasurementManager.o MeasurementService.o Measurer.o ServiceDiscoverer.o TcpMeasurer.o TelnetServer.o TelnetSession.o utils.o UdpMeasurer.o UdpServer.o IcmpPinger.o IcmpMeasurer.o options.o
	$(CC) $^ -o $@ $(LFLAGS)

.PHONY: clean TARGET
clean:
	rm -f opoznienia *.o *~ *.bak
