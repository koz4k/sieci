#include "ServiceDiscoverer.hpp"
#include "io.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

ServiceDiscoverer::ServiceDiscoverer(MeasurementManager& manager):
    manager_(manager), timer_(io)
{
    discover_();
}

void ServiceDiscoverer::discover_()
{
    std::unordered_map<std::string,
            std::vector<const MeasurementService*>> hosts;
    std::vector<const MeasurementService*> services{&manager_.getServices()[0]};
    hosts["127.0.0.1"] = hosts["0.0.0.0"] = services;
    manager_.setHosts(hosts);

    timer_.expires_from_now(boost::posix_time::seconds(10));
    timer_.async_wait(std::bind(&ServiceDiscoverer::discover_, this));
}
