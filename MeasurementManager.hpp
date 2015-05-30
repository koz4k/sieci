#ifndef MEASUREMENT_MANAGER_HPP
#define MEASUREMENT_MANAGER_HPP

#include "MeasurementService.hpp"
#include "MeasurementCollector.hpp"
#include <boost/asio.hpp>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

class MeasurementManager
{
  public:
    explicit MeasurementManager(std::vector<MeasurementService> services);
    void startCollecting(const std::string& address);
    std::vector<MeasurementCollector::Data> getData() const;

  private:
    void measure_();

    std::vector<MeasurementService> services_;
    std::unordered_map<std::string,
            std::unique_ptr<MeasurementCollector>> collectors_;
    boost::asio::deadline_timer timer_;
};

#endif
