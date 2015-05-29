#ifndef DUMMY_MEASURER_HPP
#define DUMMY_MEASURER_HPP

#include "Measurer.hpp"

class DummyMeasurer: public Measurer
{
  public:
    using Measurer::Measurer;
    static std::unique_ptr<Measurer> create(MeasurementCollector& collector,
            int typeIndex);

  protected:
    void startMeasurement_();
};

#endif
