#ifndef MEASURER_HPP
#define MEASURER_HPP

#include "MeasurementCollector.hpp"
#include <cstdint>

class MeasurementCollector;

class Measurer
{
  public:
    Measurer(MeasurementCollector& collector, int typeIndex):
        active_(false), collector_(collector), typeIndex_(typeIndex) {}
    void measure();
    bool isActive() const { return active_; }
    void setActive(bool active) { active_ = active; }

  protected:
    virtual void startMeasurement_() = 0;
    void endMeasurement_(uint64_t measurement);

    bool active_;

  private:
    MeasurementCollector& collector_;
    int typeIndex_;
};

#endif
