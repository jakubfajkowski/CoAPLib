#include "CoAPMessage.h"

CoAPMessage::CoAPMessage() {
    header_ = {DEFAULT_VERSION, 0, 0, 0, 0};
}

/** Puts values from CoAPMessage into unsigned char array, returns length**/
unsigned int CoAPMessage::serialize(unsigned char* buffer_begin) const {
    unsigned char* cursor = buffer_begin;

    insert(cursor, header_);
    insert(cursor, token_);
    insert(cursor, options_);
    insert(cursor, PAYLOAD_MARKER);
    insert(cursor, payload_);

    return cursor - buffer_begin - 1;
}

/** Puts values from header into unsigned char array **/
void CoAPMessage::insert(unsigned char* &cursor, const Header &header) const {
    *cursor = (header.Ver << OFFSET_VER) | (header.T << OFFSET_T) | header.TKL;
	*++cursor = header.Code;
	*++cursor = header.MessageId >> OFFSET_MESSAGE_ID;
	*++cursor = header.MessageId & MASK_MESSAGE_ID;
    ++cursor;
}

/** Puts values from ByteArray into unsigned char array **/
void CoAPMessage::insert(unsigned char* &cursor, const ByteArray &bytes) const {
    bytes.serialize(cursor);
    cursor += bytes.size();
}

/** Puts values from OptionArray into unsigned char array **/
void CoAPMessage::insert(unsigned char* &cursor, const OptionArray &options) const {
    CoAPOption::serialize(cursor, options);
}

/** Puts values unsigned char into unsigned char array **/
void CoAPMessage::insert(unsigned char *&cursor, unsigned char value) const {
    *cursor = value;
    ++cursor;
}

/** Fills Message with values extracted from unsigned char array**/
void CoAPMessage::deserialize(unsigned char *buffer_begin, unsigned int num) {
    unsigned char* cursor = buffer_begin;
    unsigned char* buffer_end = buffer_begin + num;

    extractHeader(cursor, buffer_end);
    extractToken(cursor, buffer_end);
    extractOptions(cursor, buffer_end);
    extractPayload(cursor, buffer_end);
}

/** Fills header with values extracted from unsigned char array**/
void CoAPMessage::extractHeader(unsigned char *&cursor, unsigned char* buffer_end) {
    header_.Ver = (*cursor & MASK_VER) >> OFFSET_VER;
    header_.T = (*cursor & MASK_T) >> OFFSET_T;
    header_.TKL = (*cursor & MASK_TKL);
    header_.Code = (*++cursor);
    header_.MessageId = ((*++cursor) << OFFSET_MESSAGE_ID) | (*++cursor);
    ++cursor;
}

/** Fills Token with value extracted from unsigned char array**/
void CoAPMessage::extractToken(unsigned char *&cursor, unsigned char* buffer_end) {
    token_.deserialize(cursor, header_.TKL);
    cursor += header_.TKL;
}

/** Fills Options with values extracted from unsigned char array**/
void CoAPMessage::extractOptions(unsigned char* &cursor, unsigned char* buffer_end) {
    CoAPOption::deserialize(cursor, buffer_end, options_);
}

/** Fills Payload with values extracted from unsigned char array**/
void CoAPMessage::extractPayload(unsigned char *&cursor, unsigned char* buffer_end) {
    payload_.deserialize(cursor, buffer_end - cursor);
    cursor += buffer_end - cursor;
}

unsigned short CoAPMessage::getVer() const {
    return header_.Ver;
}

unsigned short CoAPMessage::getT() const {
    return header_.T;
}

void CoAPMessage::setT(unsigned short T) {
    header_.T = T;
}

unsigned short CoAPMessage::getTKL() const {
    return header_.TKL;
}

unsigned short CoAPMessage::getCode() const {
    return header_.Code;
}

void CoAPMessage::setCode(unsigned short Code) {
    header_.Code = Code;
}

unsigned short CoAPMessage::getMessageId() const {
    return header_.MessageId;
}

void CoAPMessage::setMessageId(unsigned short MessageId) {
    header_.MessageId = MessageId;
}

const ByteArray &CoAPMessage::getToken() const {
    return token_;
}

/** Sets token and appropriate value in TKL **/
void CoAPMessage::setToken(const ByteArray &token) {
    token_ = token;
    header_.TKL = (unsigned short) token.size();
}

const OptionArray &CoAPMessage::getOptions() const {
    return options_;
}

/** Adds single Option to Option Array**/
void CoAPMessage::addOption(const CoAPOption &option) {
    for (unsigned int i = 0; i < options_.size(); ++i) {
        if (option.getNumber() < options_[i].getNumber()) {
            options_.insert(option, i);
            return;
        }
    }

    options_.pushBack(option);
}

const ByteArray &CoAPMessage::getPayload() const {
    return payload_;
}

void CoAPMessage::setPayload(const ByteArray &payload) {
    payload_ = payload;
}

/** In debug mode prints contents of message using SPI or std::cout, depending on platform **/
void CoAPMessage::print() const {
    PRINTLN("---CoAP message---");
    PRINT("Version: ");
    PRINTLN(TO_STRING(header_.Ver));
    PRINT("Type: ");
    PRINTLN(TO_STRING(header_.T));
    PRINT("Token length: ");
    PRINTLN(TO_STRING(header_.TKL));
    PRINT("Code: ");
    PRINTLN(TO_STRING(header_.Code));
    PRINT("Message ID: ");
    PRINTLN(TO_STRING(header_.MessageId));

    if (token_.size() != 0) {
        PRINT("Token:");
        PRINTLN(CoAPMessage::toString(payload_));
    }

    if (options_.size() != 0) {
        PRINT("Options:\n");
        print(options_);
    }

    if (payload_.size() != 0) {
        PRINT("Payload: \n\t");
        PRINTLN(CoAPMessage::toString(payload_));
    }

    PRINTLN("");
}

/** Converts ByteArray into String**/
const String CoAPMessage::toString(const ByteArray &byte_array) {
    String s;
  
    for(int i = 0; i < byte_array.size(); ++i){
        s += char(byte_array[i]);
    }

    return s;
}

void CoAPMessage::print(const OptionArray &options) const {
    for (int i = 0; i < options.size(); ++i) {
        options[i].print();
    }
}
