#ifndef SERVICE_DISCOVERER_HPP
#define SERVICE_DISCOVERER_HPP

#include "MeasurementManager.hpp"
#include <boost/asio.hpp>

class ServiceDiscoverer
{
  public:
    explicit ServiceDiscoverer(MeasurementManager& manager);

  private:
    void discover_();

    MeasurementManager& manager_;
    boost::asio::deadline_timer timer_;
};

#endif
