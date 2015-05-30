#ifndef MEASUREMENT_SERVICE_HPP
#define MEASUREMENT_SERVICE_HPP

#include "Measurer.hpp"
#include <functional>
#include <memory>

class Measurer;
class MeasurementCollector;

class MeasurementService
{
    typedef std::function<
            std::unique_ptr<Measurer>(MeasurementCollector&, int)> 
                    MeasurerFactory_;

  public:
    explicit MeasurementService(MeasurerFactory_ measurerFactory):
            measurerFactory_(measurerFactory), index_(0) {}
    std::unique_ptr<Measurer> createMeasurer(MeasurementCollector& collector) const;
    int getIndex() const { return index_; }
    void setIndex(int index) { index_ = index; }

  private:
    MeasurerFactory_ measurerFactory_;
    int index_;
};

#endif
