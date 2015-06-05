#include "DnsMessage.hpp"
#include <arpa/inet.h>
#include <algorithm>
#include <cstring>

struct __attribute__((packed, aligned(1))) PackedHeader
{
    uint16_t id;
    uint8_t flags1;
    uint8_t flags2;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;

    PackedHeader(): id(0), flags1(0), flags2(0),
        qdcount(0), ancount(0), nscount(0), arcount(0)
    {
    }
};

struct __attribute__((packed, aligned(1))) PackedQuestionFooter
{
    uint16_t qtype;
    uint16_t qclass;

    PackedQuestionFooter(): qtype(0), qclass(htons(1))
    {
    }
};

struct __attribute__((packed, aligned(1))) PackedResourceMiddle
{
    uint16_t type;
    uint16_t _class;
    uint32_t ttl;
    uint16_t rdlength;

    PackedResourceMiddle(): type(0), _class(htons(1)), ttl(0), rdlength(0)
    {
    }
};

char toLowerC(char c)
{
    return c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c;
}

std::string toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), toLowerC);
    return str;
}

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
        std::string lpart = toLower(part);
        bytes[i] = lpart.size();
        memcpy(&bytes[i + 1], lpart.c_str(), lpart.size());
        i += 1 + lpart.size();
    }
    bytes[i] = 0;
}

std::vector<std::string> deserializeName(const std::vector<uint8_t>& bytes,
        int& index)
{
    int start = index;
    std::vector<std::string> name;
    while(bytes[index])
    {
        // jesli 2 gorne bity sa zapalone, to offset
        if((bytes[index] & 0xc0) == 0xc0)
        {
            int offset = (bytes[index] & ~0xc0) << 8 | bytes[index + 1];

            if(offset >= start)
                throw DnsMessage::FormatError(
                        "Name offset references a later position");

            index += 2;
            std::vector<std::string> tail = deserializeName(bytes, offset);
            name.insert(name.end(), tail.begin(), tail.end());

            return name;
        }

        int len = bytes[index];
        index += 1;
        name.push_back(toLower(std::string(&bytes[index], &bytes[index + len])));
        index += len;
    }
    index += 1;

    return name;
}

template<typename T>
void serializePrimitive(std::vector<uint8_t>& bytes, const T& primitive)
{
    int i = bytes.size();
    bytes.resize(bytes.size() + sizeof(T));
    memcpy(&bytes[i], &primitive, sizeof(T));
}

template<typename T>
T deserializePrimitive(const std::vector<uint8_t>& bytes, int& index)
{
    T primitive;
    memcpy(&primitive, &bytes[index], sizeof(T));
    index += sizeof(T);
    return primitive;
}

void serializeQuestion(std::vector<uint8_t>& bytes,
        const DnsMessage::Question& question)
{
    serializeName(bytes, question.name);

    PackedQuestionFooter footer;
    footer.qtype = htons((uint16_t) question.type);
    if(question.unicastResponse)
        footer.qclass = htons(0x8001);
    serializePrimitive(bytes, footer);
}

DnsMessage::Question deserializeQuestion(const std::vector<uint8_t>& bytes,
        int& index)
{
    std::vector<std::string> name = deserializeName(bytes, index);

    PackedQuestionFooter footer =
            deserializePrimitive<PackedQuestionFooter>(bytes, index);

    return DnsMessage::Question(std::move(name),
            (DnsMessage::Type) ntohs(footer.qtype),
            ntohs(footer.qclass) & 0x8000);
}

void serializeResource(std::vector<uint8_t>& bytes,
        const DnsMessage::Resource& resource)
{
    serializeName(bytes, resource.name);

    PackedResourceMiddle middle;
    middle.type = htons((uint16_t) resource.type);
    middle.ttl = htonl((uint32_t) resource.ttl);

    std::vector<uint8_t> data;
    if(resource.type == DnsMessage::TYPE_PTR)
        serializeName(data, resource.dname);
    else
        data = resource.data;

    middle.rdlength = htons(data.size());
    serializePrimitive(bytes, middle);
    
    bytes.insert(bytes.end(), data.begin(), data.end());
}

DnsMessage::Resource deserializeResource(const std::vector<uint8_t>& bytes,
        int& index)
{
    std::vector<std::string> name = deserializeName(bytes, index);

    PackedResourceMiddle middle =
            deserializePrimitive<PackedResourceMiddle>(bytes, index);
    DnsMessage::Type type = (DnsMessage::Type) ntohs(middle.type);

    int len = ntohs(middle.rdlength);
    std::vector<uint8_t> data(&bytes[index], &bytes[index + len]);

    std::vector<std::string> dname;
    if(type == DnsMessage::TYPE_PTR)
    {
        std::vector<std::string> dname = deserializeName(bytes, index);
        return DnsMessage::Resource(std::move(name), type, ntohl(middle.ttl),
                std::move(dname));
    }
    else
    {
        index += len;
        return DnsMessage::Resource(std::move(name), type, ntohl(middle.ttl),
                std::move(data));
    }
}

DnsMessage::DnsMessage(bool isResponse, bool recursionDesired):
    id(0), isResponse(isResponse), recursionDesired(recursionDesired),
    responseCode(RCODE_OK)
{
}

DnsMessage::DnsMessage(const std::vector<uint8_t>& bytes):
    id(0), isResponse(false), recursionDesired(false), responseCode(RCODE_OK)
{
    int index = 0;

    PackedHeader header = deserializePrimitive<PackedHeader>(bytes, index);
    id = ntohs(header.id);
    isResponse = header.flags1 & 0x80; // QR
    recursionDesired = header.flags1 & 0x01; // RD
    responseCode = (ResponseCode) (header.flags2 & 0x0f); // RCODE

    for(int i = 0; i < ntohs(header.qdcount); ++i)
        questions.push_back(deserializeQuestion(bytes, index));

    for(int i = 0; i < ntohs(header.ancount); ++i)
        answers.push_back(deserializeResource(bytes, index));

    for(int i = 0; i < ntohs(header.nscount); ++i)
        authorities.push_back(deserializeResource(bytes, index));

    for(int i = 0; i < ntohs(header.arcount); ++i)
        additionals.push_back(deserializeResource(bytes, index));
}

std::vector<uint8_t> DnsMessage::serialize() const
{
    std::vector<uint8_t> bytes;

    PackedHeader header;
    header.id = htons(id);
    header.flags1 = (isResponse ? 0x80 : 0x00)
                  | (recursionDesired ? 0x01 : 0x00);
    header.flags2 = (uint8_t) responseCode;
    header.qdcount = htons(questions.size());
    header.ancount = htons(answers.size());
    header.arcount = htons(additionals.size());
    serializePrimitive(bytes, header);

    for(const Question& question : questions)
        serializeQuestion(bytes, question);

    for(const Resource& answer : answers)
        serializeResource(bytes, answer);

    for(const Resource& authority : authorities)
        serializeResource(bytes, authority);

    for(const Resource& additional : additionals)
        serializeResource(bytes, additional);

    return bytes;
}

bool DnsMessage::isEmpty() const
{
    return questions.empty() && answers.empty() && authorities.empty() &&
        additionals.empty();
}
