#ifndef MEASUREMENT_COLLECTOR_HPP
#define MEASUREMENT_COLLECTOR_HPP

#include "Measurer.hpp"
#include "MeasurementType.hpp"
#include <vector>
#include <memory>
#include <deque>
#include <ostream>

class Measurer;
class MeasurementType;

class MeasurementCollector
{
  public:
    struct Data
    {
        std::string address;
        std::string render;
        double mean;

        Data(): mean(0) {}
        bool operator<(const Data& other) const { return mean > other.mean; }
    };

    MeasurementCollector(const std::string& address,
            const std::vector<MeasurementType>& types);
    void measure();
    void collect(int type, int measurement);
    const std::string& getAddress() const { return address_; }
    Data getData() const;

  private:
    static void outputMeasurement_(std::ostream& os, double measurement);

    std::string address_;
    std::vector<std::unique_ptr<Measurer>> measurers_;
    std::vector<std::deque<int>> measurements_;
    std::vector<int> sums_;
};

#endif
