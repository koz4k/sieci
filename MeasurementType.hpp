#ifndef MEASUREMENT_TYPE_HPP
#define MEASUREMENT_TYPE_HPP

#include "Measurer.hpp"
#include <functional>
#include <memory>

class Measurer;
class MeasurementCollector;

class MeasurementType
{
    typedef std::function<
            std::unique_ptr<Measurer>(MeasurementCollector&, int)> 
                    MeasurerFactory_;

  public:
    explicit MeasurementType(MeasurerFactory_ measurerFactory):
            measurerFactory_(measurerFactory), index_(0) {}
    std::unique_ptr<Measurer> createMeasurer(MeasurementCollector& collector) const;
    int getIndex() const { return index_; }
    void setIndex(int index) { index_ = index; }

  private:
    MeasurerFactory_ measurerFactory_;
    int index_;
};

#endif
