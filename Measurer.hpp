#ifndef MEASURER_HPP
#define MEASURER_HPP

#include "MeasurementCollector.hpp"
#include <cstdint>

class MeasurementCollector;

class Measurer
{
  public:
    Measurer(MeasurementCollector& collector, int typeIndex):
        collector_(collector), typeIndex_(typeIndex), startTime_(0) {}
    void measure();

  protected:
    virtual void startMeasurement_() = 0;
    void endMeasurement_();

  private:
    MeasurementCollector& collector_;
    int typeIndex_;
    uint64_t startTime_;
};

#endif
