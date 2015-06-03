#ifndef UDP_MEASURER_HPP
#define UDP_MEASURER_HPP

#include "Measurer.hpp"
#include <boost/asio.hpp>

class UdpMeasurer: public Measurer
{
  public:
    UdpMeasurer(MeasurementCollector& collector, int typeIndex);
    static std::unique_ptr<Measurer> create(MeasurementCollector& collector,
            int typeIndex);

  protected:
    void startMeasurement_();

  private:
    boost::asio::ip::udp::endpoint endpoint_;
    boost::asio::ip::udp::socket socket_;
};

#endif
