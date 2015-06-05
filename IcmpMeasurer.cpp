#include "IcmpMeasurer.hpp"
#include "IcmpPinger.hpp"
#include "utils.hpp"

using namespace boost::asio;

IcmpMeasurer::IcmpMeasurer(MeasurementCollector& collector, int typeIndex):
    Measurer(collector, typeIndex),
    endpoint_(ip::address::from_string(collector.getAddress()), 0)
{
}

std::unique_ptr<Measurer> IcmpMeasurer::create(MeasurementCollector& collector,
        int typeIndex)
{
    return std::unique_ptr<Measurer>(new IcmpMeasurer(collector, typeIndex));
}

void IcmpMeasurer::startMeasurement_()
{
    uint64_t t = microtime();
    IcmpPinger::getInstance().ping(endpoint_,
            [this, t]()
            {
                endMeasurement_(microtime() - t);
            });
}
