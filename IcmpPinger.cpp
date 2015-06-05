#include "IcmpPinger.hpp"
#include "io.hpp"
#include <iostream>
#include <cstring>

using namespace boost::asio;
using namespace boost::system;
using boost::asio::ip::icmp;

constexpr int MIN_IP_HEADER_SIZE = 20;

struct __attribute((packed, aligned(1))) IcmpMessage
{
    static constexpr uint8_t ECHO_REQUEST = 8;
    static constexpr uint8_t ECHO_REPLY = 0;

    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
    uint8_t index[3];
    uint8_t group;

    explicit IcmpMessage(uint16_t sequence = 0):
        type(ECHO_REQUEST), code(0), id(htons(0x13)),
        sequence(htons(sequence)), index{0x34, 0x69, 0x62}, group(2)
    {
        uint32_t sum = ((type << 8) | code) + ntohs(id) + sequence +
                ((index[0] << 8) | index[1]) + ((index[2] << 8) | group);

        sum = (sum >> 16) + (sum & 0xffff);
        sum += sum >> 16;
        checksum = htons(~sum);
    }
};

IcmpPinger::IcmpPinger():
    socket_(io, icmp::v4()), sequence_(0)
{
    receive_();
}

IcmpPinger& IcmpPinger::getInstance()
{
    if(!instance_)
        instance_.reset(new IcmpPinger());

    return *instance_;
}

void IcmpPinger::ping(const icmp::endpoint& endpoint,
        std::function<void()> handler)
{
    uint16_t sequence = sequence_++;
    IcmpMessage message(sequence);
    auto data = std::make_shared<std::array<uint8_t, sizeof(IcmpMessage)>>();
    memcpy(&(*data)[0], &message, sizeof(message));
    socket_.async_send_to(buffer(*data), endpoint,
            [this, data, handler, sequence, &endpoint]
                    (const error_code& error, size_t len)
            {
                if(!error)
                    handlers_[sequence] = std::move(handler);
                else
                {
                    std::cerr << "IcmpPinger, send to "
                              << endpoint.address().to_string() << " error: "
                              << error.message() << std::endl;
                }
            });
}

void IcmpPinger::receive_()
{
    socket_.async_receive(buffer(buffer_),
            [this](const error_code& error, size_t len)
            {
                if(!error)
                {
                    if(len >= MIN_IP_HEADER_SIZE + sizeof(IcmpMessage))
                    {
                        IcmpMessage message;
                        memcpy(&message, &buffer_[len - sizeof(IcmpMessage)],
                                sizeof(IcmpMessage));
                        if(message.type == IcmpMessage::ECHO_REPLY &&
                                ntohs(message.id) == 0x13)
                        {
                            if(message.index[0] == 0x34 &&
                                    message.index[1] == 0x69 &&
                                    message.index[2] == 0x62 &&
                                    message.group == 2)
                            {
                                uint16_t sequence = ntohs(message.sequence);
                                auto it = handlers_.find(sequence);
                                if(it != handlers_.end())
                                    it->second();
                                else
                                {
                                    std::cerr << "IcmpPinger, handler not found"
                                              << " for sequence number "
                                              << sequence << std::endl;
                                }
                            }
                            else
                            {
                                std::cerr << "IcmpPinger, wrong data received"
                                          << " in ping reply" << std::endl;
                            }
                        }
                    }
                }
                else
                {
                    std::cerr << "IcmpPinger, receive error: "
                              << error.message() << std::endl;
                }

                receive_();
            });
}

std::unique_ptr<IcmpPinger> IcmpPinger::instance_;
