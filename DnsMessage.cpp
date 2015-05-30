#include "DnsMessage.hpp"
#include <arpa/inet.h>
#include <cstring>

uint16_t nowId = 0;

struct __attribute__((packed)) PackedHeader
{
    uint16_t id;
    uint8_t qr : 1;
    uint8_t opcode : 4;
    uint8_t aa : 1;
    uint8_t tc : 1;
    uint8_t rd : 1;
    uint8_t ra : 1;
    uint8_t z : 3;
    uint8_t rcode : 4;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;

    PackedHeader(): id(htons(nowId++)), qr(0), opcode(0), aa(0), tc(0), rd(0),
        ra(0), z(0), rcode(0), qdcount(0), ancount(0), nscount(0), arcount(0)
    {
    }
};

struct __attribute__((packed)) PackedQuestionFooter
{
    uint16_t qtype;
    //uint8_t qu : 1;
    //uint16_t qclass : 15;
    uint16_t qclass;

    PackedQuestionFooter(): qtype(0), /*qu(0),*/ qclass(htons(1))
    {
    }
};

struct __attribute__((packed)) PackedResourceMiddle
{
    uint16_t type;
    uint16_t _class;
    uint32_t ttl;
    uint16_t rdlength;

    PackedResourceMiddle(): type(0), _class(htons(1)), ttl(0), rdlength(0)
    {
    }
};

void serializeName(std::vector<uint8_t>& bytes,
        const std::vector<std::string>& name)
{
    int byteCount = 1;
    for(const std::string& part : name)
        byteCount += part.size() + 1;

    int i = bytes.size();
    bytes.resize(bytes.size() + byteCount);
    for(const std::string& part : name)
    {
        bytes[i] = part.size();
        memcpy(&bytes[i + 1], part.c_str(), part.size());
        i += 1 + part.size();
    }
    bytes[i] = 0;
}

template<typename T>
void serializePrimitive(std::vector<uint8_t>& bytes, const T& primitive)
{
    int i = bytes.size();
    bytes.resize(bytes.size() + sizeof(T));
    memcpy(&bytes[i], &primitive, sizeof(T));
}

void serializeString(std::vector<uint8_t>& bytes, const std::string& string)
{
    int i = bytes.size();
    bytes.resize(bytes.size() + string.size());
    memcpy(&bytes[i], string.c_str(), string.size());
}

void serializeQuestion(std::vector<uint8_t>& bytes,
        const DnsMessage::Question& question)
{
    serializeName(bytes, question.name);

    PackedQuestionFooter footer;
    footer.qtype = htons((uint16_t) question.type);
    //footer.qu = question.unicastResponse;
    serializePrimitive(bytes, footer);
}

void serializeResource(std::vector<uint8_t>& bytes,
        const DnsMessage::Resource& resource)
{
    serializeName(bytes, resource.name);

    PackedResourceMiddle middle;
    middle.type = htons((uint16_t) resource.type);
    middle.rdlength = htons(resource.data.size());
    serializePrimitive(bytes, middle);
    
    serializeString(bytes, resource.data);
}

DnsMessage::DnsMessage(const std::vector<uint8_t>& bytes)
{
}

std::vector<uint8_t> DnsMessage::serialize() const
{
    std::vector<uint8_t> bytes;

    PackedHeader header;
    header.qr = response_;
    header.qdcount = htons(questions_.size());
    header.ancount = htons(answers_.size());
    header.arcount = htons(additionals_.size());
    serializePrimitive(bytes, header);

    for(const Question& question : questions_)
        serializeQuestion(bytes, question);

    for(const Resource& answer : answers_)
        serializeResource(bytes, answer);

    for(const Resource& additional : additionals_)
        serializeResource(bytes, additional);

    return bytes;
}

void DnsMessage::addQuestion(Question question)
{
    questions_.push_back(std::move(question));
}

void DnsMessage::addAnswer(Resource answer)
{
    answers_.push_back(std::move(answer));
}

void DnsMessage::addAdditional(Resource additional)
{
    additionals_.push_back(std::move(additional));
}
