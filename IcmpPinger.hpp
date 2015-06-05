#ifndef ICMP_PINGER_HPP
#define ICMP_PINGER_HPP

#include "constants.hpp"
#include <boost/asio.hpp>
#include <functional>
#include <unordered_map>

class IcmpPinger
{
    IcmpPinger(const IcmpPinger&) = delete;
    IcmpPinger(IcmpPinger&&) = delete;
    IcmpPinger& operator=(const IcmpPinger&) = delete;
    IcmpPinger& operator=(IcmpPinger&&) = delete;

  public:
    static IcmpPinger& getInstance();
    void ping(const boost::asio::ip::icmp::endpoint& endpoint,
            std::function<void()> handler);

  private:
    IcmpPinger();
    void receive_();

    boost::asio::ip::icmp::socket socket_;
    std::unordered_map<uint16_t, std::function<void()>> handlers_;
    uint16_t sequence_;
    std::array<uint8_t, MAX_PACKET_SIZE> buffer_;
    static std::unique_ptr<IcmpPinger> instance_;
};

#endif
