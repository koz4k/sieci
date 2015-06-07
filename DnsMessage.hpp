#ifndef DNS_MESSAGE_HPP
#define DNS_MESSAGE_HPP

#include <vector>
#include <string>
#include <stdexcept>

struct DnsMessage
{
    enum ResponseCode
    {
        RCODE_OK = 0,
        RCODE_FORMAT_ERROR = 1,
        RCODE_SERVER_FAILURE = 2,
        RCODE_NOT_IMPLEMENTED = 4
    };

    enum Type
    {
        TYPE_A = 1,
        TYPE_PTR = 12
    };

    struct Question
    {
        std::vector<std::string> name;
        Type type;
        bool unicastResponse;

        Question(std::vector<std::string> name, Type type,
                bool unicastResponse = false):
            name(std::move(name)), type(type),
            unicastResponse(unicastResponse) {}
    };

    struct Resource
    {
        std::vector<std::string> name;
        Type type;
        int ttl;
        std::vector<uint8_t> data;
        std::vector<std::string> dname;

        Resource(std::vector<std::string> name, Type type, int ttl,
                std::vector<uint8_t> data):
            name(std::move(name)), type(type), ttl(ttl),
            data(std::move(data)) {}
        Resource(std::vector<std::string> name, Type type, int ttl,
                std::vector<std::string> dname):
            name(std::move(name)), type(type), ttl(ttl),
            dname(std::move(dname)) {}
    };

    class FormatError: public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    explicit DnsMessage(bool isResponse = false, bool authoritative = false);
    DnsMessage(const std::vector<uint8_t>& bytes, int len);
    std::vector<uint8_t> serialize() const;
    bool isEmpty() const;
    
    int id;
    bool isResponse;
    bool authoritative;
    ResponseCode responseCode;

    std::vector<Question> questions;
    std::vector<Resource> answers;
    std::vector<Resource> authorities;
    std::vector<Resource> additionals;
};

#endif
