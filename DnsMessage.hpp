#ifndef DNS_MESSAGE_HPP
#define DNS_MESSAGE_HPP

#include <vector>
#include <string>

class DnsMessage
{
  public:
    enum ResponseCode
    {
        RCODE_OK = 0,
        RCODE_FORMAT_ERROR = 1,
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
                bool unicastResponse):
            name(std::move(name)), type(type),
            unicastResponse(unicastResponse) {}
    };

    struct Resource
    {
        std::vector<std::string> name;
        Type type;
        std::string data;

        Resource(std::vector<std::string> name, Type type, std::string data):
            name(std::move(name)), type(type), data(std::move(data)) {}
    };

    explicit DnsMessage(bool response): response_(response) {}
    explicit DnsMessage(const std::vector<uint8_t>& bytes);
    std::vector<uint8_t> serialize() const;
    bool isResponse() const { return response_; }
    void addQuestion(Question question);
    void addAnswer(Resource answer);
    void addAdditional(Resource additional);
    
  private:
    bool response_;
    std::vector<Question> questions_;
    std::vector<Resource> answers_;
    std::vector<Resource> additionals_;
};

#endif
