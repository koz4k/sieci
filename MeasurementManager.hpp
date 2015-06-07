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
    const std::vector<MeasurementService>& getServices() const;
    void activateServiceForHost(const std::string& address,
            const MeasurementService* service);
    void deactivateServiceForHost(const std::string& address,
            const MeasurementService* service);
    void deactivateServicesForHost(const std::string& address);
    std::vector<MeasurementCollector::Data> getData() const;

  private:
    MeasurementCollector& getCollector_(const std::string& address);
    void measure_();

    std::vector<MeasurementService> services_;
    std::unordered_map<std::string,
            std::unique_ptr<MeasurementCollector>> collectors_;
    boost::asio::deadline_timer timer_;
};

#endif
