#include "DnsMessage.hpp"
#include "io.hpp"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;
using boost::asio::ip::udp;

void send(udp::socket& socket, const udp::endpoint& endpoint,
        DnsMessage::Type type, std::vector<std::string> name)
{
    DnsMessage message(false, true);
    message.questions.push_back(DnsMessage::Question(std::move(name),
            type, false));
    std::vector<uint8_t> bytes = message.serialize();
    socket.send_to(buffer(bytes), endpoint);
}

void printResponseCode(DnsMessage::ResponseCode code)
{
    switch(code)
    {
      case DnsMessage::RCODE_OK:
        std::cout << "ok";
        break;

      case DnsMessage::RCODE_FORMAT_ERROR:
        std::cout << "format error";
        break;

      case DnsMessage::RCODE_SERVER_FAILURE:
        std::cout << "server failure";
        break;

      case DnsMessage::RCODE_NOT_IMPLEMENTED:
        std::cout << "not implemented";
        break;

      default:
        std::cout << "unknown";
    }
}

void printType(DnsMessage::Type type)
{
    switch(type)
    {
      case DnsMessage::TYPE_A:
        std::cout << "A";
        break;

      case DnsMessage::TYPE_PTR:
        std::cout << "PTR";
        break;

      default:
        std::cout << "unknown";
    }
}

void printName(const std::vector<std::string>& name)
{
    for(int i = 0; i < name.size(); ++i)
    {
        std::cout << name[i];
        if(i < name.size() - 1)
            std::cout << " . ";
    }
}

void printByte(uint8_t byte)
{
    for(uint8_t mask = 1 << 7; mask; mask >>= 1)
        std::cout << (byte & mask ? 1 : 0);
}

void printResource(const DnsMessage::Resource& resource)
{
    std::cout << " type ";
    printType(resource.type);
    std::cout << " for name ";
    printName(resource.name);
    std::cout << ": ";

    switch(resource.type)
    {
      case DnsMessage::TYPE_A:
        std::cout << (uint32_t) resource.data[0] << "."
                  << (uint32_t) resource.data[1] << "."
                  << (uint32_t) resource.data[2] << "."
                  << (uint32_t) resource.data[3];
        break;

      case DnsMessage::TYPE_PTR:
        printName(resource.dname);
        break;

      default:
        std::cout << std::string(resource.data.begin(), resource.data.end());
    }

    std::cout << " (ttl: " << resource.ttl << ")";

    std::cout << std::endl;
}

void receive(udp::socket& socket)
{
    static std::vector<uint8_t> buf(16384);
    udp::endpoint endpoint;
    int size = socket.receive_from(buffer(buf), endpoint);

    std::cout << "Data received:" << std::endl;
    for(int i = 0; i < size; i += 2)
    {
        printByte(buf[i]);
        std::cout << " ";
        printByte(buf[i + 1]);
        std::cout << std::endl;
    }
    std::cout << std::endl;

    DnsMessage message(buf);
    if(!message.isResponse)
    {
        std::cout << "Query with id " << message.id
                  << " (size: " << size << ")" << std::endl;
    }
    else
    {
        std::cout << "Response with id " << message.id
                  << " (response code: ";
        printResponseCode(message.responseCode);
        std::cout << ", size: " << size << ")" << std::endl;
    }

    for(const DnsMessage::Question& question : message.questions)
    {
        std::cout << "Question type ";
        printType(question.type);
        std::cout << " for name ";
        printName(question.name);
        std::cout << std::endl;
    }

    for(const DnsMessage::Resource& answer : message.answers)
    {
        std::cout << "Answer";
        printResource(answer);
    }

    for(const DnsMessage::Resource& authority : message.authorities)
    {
        std::cout << "Authority";
        printResource(authority);
    }

    for(const DnsMessage::Resource& additional : message.additionals)
    {
        std::cout << "Additional";
        printResource(additional);
    }

    std::cout << std::endl << std::endl;;
}

int main(int argc, char** argv)
{
    udp::socket socket(io);
    udp::endpoint endpoint(ip::address_v4({224, 0, 0, 251}), 5353);
    socket.open(udp::v4());

    std::string type = argv[1];

    std::vector<std::string> name;
    for(int i = 2; i < argc; ++i)
        name.push_back(argv[i]);

    send(socket, endpoint,
            type == "a" ? DnsMessage::TYPE_A : DnsMessage::TYPE_PTR, name);
    receive(socket);

    return 0;
}
