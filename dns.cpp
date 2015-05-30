#include "DnsMessage.hpp"
#include "io.hpp"
#include <boost/asio.hpp>

using namespace boost::asio;
using boost::asio::ip::udp;


void send(udp::socket& socket, const udp::endpoint& endpoint)
{
    DnsMessage message(false);
    message.addQuestion(DnsMessage::Question({"google", "com"},
                DnsMessage::TYPE_A, false));
    std::vector<uint8_t> bytes = message.serialize();
    socket.send_to(buffer(bytes), endpoint);
}

int main(int argc, char** argv)
{
    udp::socket socket(io);
    udp::endpoint endpoint(ip::address::from_string("8.8.8.8"), 53);
    socket.open(udp::v4());

    for(int i = 0; i < 10; ++i)
        send(socket, endpoint);

    return 0;
}
