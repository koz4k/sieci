#ifndef TCP_MEASURER_HPP
#define TCP_MEASURER_HPP

#include "Measurer.hpp"
#include <boost/asio.hpp>

class TcpMeasurer: public Measurer
{
  public:
    TcpMeasurer(MeasurementCollector& collector, int typeIndex);
    static std::unique_ptr<Measurer> create(MeasurementCollector& collector,
            int typeIndex);

  protected:
    void startMeasurement_();

  private:
    boost::asio::ip::tcp::endpoint endpoint_;
    boost::asio::ip::tcp::socket socket_;
};

#endif
