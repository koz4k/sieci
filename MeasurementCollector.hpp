#ifndef MEASUREMENT_COLLECTOR_HPP
#define MEASUREMENT_COLLECTOR_HPP

#include "Measurements.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

class MeasurementCollector
{
  public:
    explicit MeasurementCollector(int typeCount):
        typeCount_(typeCount) {}
    Measurements& startCollecting(const std::string& address);
    std::vector<Measurements::Data> getData() const;

  private:
    std::unordered_map<std::string,
            std::unique_ptr<Measurements>> measurements_;
    int typeCount_;
};

#endif
