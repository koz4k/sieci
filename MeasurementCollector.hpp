#ifndef MEASUREMENT_COLLECTOR_HPP
#define MEASUREMENT_COLLECTOR_HPP

#include <deque>
#include <vector>
#include <ostream>

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

    MeasurementCollector(const std::string& address, int typeCount):
        address_(address), measurements_(typeCount), sums_(typeCount) {}
    void collect(int type, int measurement);
    Data getData() const;

  private:
    static void outputMeasurement_(std::ostream& os, double measurement);

    std::string address_;
    std::vector<std::deque<int>> measurements_;
    std::vector<int> sums_;
};

#endif
