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
    Measurements* collect(const std::string& address);
    std::vector<Measurements::Data> getData();

  private:
    std::unordered_map<std::string,
            std::unique_ptr<Measurements>> measurements_;
};

#endif
