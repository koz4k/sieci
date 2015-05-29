#ifndef MEASUREMENT_MANAGER_HPP
#define MEASUREMENT_MANAGER_HPP

#include "MeasurementCollector.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

class MeasurementManager
{
  public:
    explicit MeasurementManager(int typeCount):
        typeCount_(typeCount) {}
    MeasurementCollector& startCollecting(const std::string& address);
    std::vector<MeasurementCollector::Data> getData() const;

  private:
    std::unordered_map<std::string,
            std::unique_ptr<MeasurementCollector>> collectors_;
    int typeCount_;
};

#endif
