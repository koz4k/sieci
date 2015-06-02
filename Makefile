TARGET: opoznienia

CC	= g++
CFLAGS	= -Wall -O2 -std=c++11
LFLAGS	= -Wall -lboost_system

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

opoznienia: DnsMessage.o io.o main.o MeasurementCollector.o MeasurementManager.o MeasurementService.o Measurer.o ServiceDiscoverer.o TcpMeasurer.o TelnetServer.o TelnetSession.o utils.o
	$(CC) $(LFLAGS) $^ -o $@

.PHONY: clean TARGET
clean:
	rm -f opoznienia *.o *~ *.bak