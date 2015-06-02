#ifndef MEASUREMENT_SERVICE_HPP
#define MEASUREMENT_SERVICE_HPP

#include "Measurer.hpp"
#include <functional>
#include <memory>
#include <string>

class Measurer;
class MeasurementCollector;

class MeasurementService
{
    typedef std::function<
            std::unique_ptr<Measurer>(MeasurementCollector&, int)> 
                    MeasurerFactory_;

  public:
    explicit MeasurementService(std::string name,
            MeasurerFactory_ measurerFactory, bool hidden = false):
        name_(std::move(name)),
        measurerFactory_(measurerFactory), hidden_(hidden), index_(0) {}
    std::string getName() const { return name_; }
    bool isHidden() const { return hidden_; }
    std::unique_ptr<Measurer> createMeasurer(
            MeasurementCollector& collector) const;
    int getIndex() const { return index_; }
    void setIndex(int index) { index_ = index; }

  private:
    std::string name_;
    MeasurerFactory_ measurerFactory_;
    bool hidden_;
    int index_;
};

#endif
