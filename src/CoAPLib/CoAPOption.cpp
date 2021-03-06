#include "CoAPOption.h"

CoAPOption::CoAPOption() : number_(0), value_() {}

/** Creates Block2 option **/
CoAPOption::CoAPOption(const Block2 &block2) : number_(OPTION_BLOCK2) {
//    switch (block2) {
//        case 1:
        unsigned char value = (unsigned char) ((block2.num << 4) | ((block2.m << 3) & 0x08) | (block2.szx - 4));
         value_.pushBack(value);
//            break;
//        case 2:
//            // TODO
//            break;
//        case 3:
//            // TODO
//            break;
//        default:
//            // TODO
//            break;
}

/** Creates Option with given number and value **/
CoAPOption::CoAPOption(unsigned int number, String value) : CoAPOption() {
    ByteArray value_bytes(value.length());
    for (int i = 0; i < value.length(); ++i) {
        value_bytes.pushBack((const unsigned char &) value[i]);
    }

    number_ = number;
    value_ = value_bytes;
}

CoAPOption::CoAPOption(unsigned int number, ByteArray value) : number_(number), value_(value) {}

/** Writes options from array into unsigned char array **/
void CoAPOption::serialize(unsigned char *&cursor, const OptionArray &options) {
    if (options.size() > 0) {
        unsigned int delta = options[0].getNumber();
        options[0].serialize(cursor, delta);

        for (unsigned int i = 1; i < options.size(); ++i) {
            delta = options[i].getNumber() - options[i - 1].getNumber();
            options[i].serialize(cursor, delta);
        }
    }
}

/** Writes options from unsigned char array into OptionArray */
void CoAPOption::deserialize(unsigned char *&cursor, unsigned char *buffer_end, OptionArray &options)  {
    unsigned int delta_sum = 0;

    while (cursor != buffer_end && *cursor != PAYLOAD_MARKER) {
        CoAPOption option;
        option.deserialize(cursor, buffer_end, delta_sum);
        options.pushBack(option);

        delta_sum = option.getNumber();
    }

    if (cursor != buffer_end)
        ++cursor;
}

void CoAPOption::serialize(unsigned char* &cursor, unsigned int delta) const {
    insert(cursor, delta, value_.size());
    insert(cursor, value_);
}

/** Takes care of setting proper option delta **/
void CoAPOption::insert(unsigned char* &cursor, unsigned int delta, unsigned int length) const {
    unsigned char header_delta = 0;
    unsigned char header_length = 0;

    prepareExtendable(header_delta, delta);
    prepareExtendable(header_length, length);

    insertHeaderValues(cursor, header_delta, header_length);
    insertExtendableValue(cursor, header_delta, delta);
    insertExtendableValue(cursor, header_length, length);
}

void CoAPOption::prepareExtendable(unsigned char &header_value, unsigned int &extendable_value) const {
    if (extendable_value < 13) {
        header_value = (unsigned char) extendable_value;
        extendable_value = 0;
    }
    else if (extendable_value < 269) {
        header_value = 13;
        extendable_value -= 13;
    }
    else {
        header_value = 14;
        extendable_value -= 269;
    }
}

void CoAPOption::insertHeaderValues(unsigned char* &cursor, unsigned char &header_delta, unsigned char &header_length) const {
    *cursor = (header_delta << OFFSET_DELTA) & MASK_DELTA | (header_length & MASK_LENGTH);
    ++cursor;
}

void CoAPOption::insertExtendableValue(unsigned char* &cursor, unsigned char header_value, unsigned int extendable_value) const {
    if (header_value < 13) {
        return;
    }
    else if (header_value == 13) {
        *cursor = extendable_value;
    }
    else if (header_value == 14) {
        *cursor = (extendable_value >> OFFSET_EXTENDABLE) & MASK_EXTENDABLE;
        *++cursor = extendable_value & MASK_EXTENDABLE;
    }

    ++cursor;
}

void CoAPOption::insert(unsigned char* &cursor, const ByteArray &bytes) const {
    bytes.serialize(cursor);
    cursor += bytes.size();
}

/** Creates single option form part of the char array **/
void CoAPOption::deserialize(unsigned char* &cursor, unsigned char* &buffer_end, unsigned int delta_sum) {
    unsigned int delta = 0;
    unsigned int length = 0;
    
    extractExtendables(cursor, delta, length);
    extractValue(cursor, length);
    number_ = delta_sum + delta;
}

void CoAPOption::extractExtendables(unsigned char *&cursor, unsigned int &delta, unsigned int &length) {
    unsigned char header_delta = 0;
    unsigned char header_length = 0;

    extractHeaderValues(cursor, header_delta, header_length);
    extractExtendableValue(cursor, header_delta, delta);
    extractExtendableValue(cursor, header_length, length);
}

void CoAPOption::extractHeaderValues(unsigned char* &cursor, unsigned char &header_delta, unsigned char &header_length) const {
    header_delta = (*cursor & MASK_DELTA) >> OFFSET_DELTA;
    header_length = *cursor & MASK_LENGTH;
    ++cursor;
}

void CoAPOption::extractExtendableValue(unsigned char *&cursor, unsigned char header_value, unsigned int &extendable_value) {
    if (header_value < 13) {
        extendable_value = header_value;
    }
    else if (header_value == 13) {
        extendable_value += 13;
    }
    else if (header_value == 14) {
        extendable_value += 269;
    }
}


void CoAPOption::extractValue(unsigned char *&cursor, unsigned int num) {
    value_.deserialize(cursor, num);
    cursor += num;
}

CoAPOption &CoAPOption::operator=(const CoAPOption &option) {
    if(&option != this) {
        number_ = option.number_;
        value_ = option.value_;
    }
    return *this;
}

unsigned int CoAPOption::getNumber() const {
    return number_;
}

const ByteArray &CoAPOption::getValue() const {
    return value_;
}

void CoAPOption::print() const {
    PRINT("\t");
    PRINT(number_);
    PRINT(": ");

    switch (number_) {
        case OPTION_BLOCK2:
            toBlock2().print();
            break;
        default:
            PRINT(toString());
            break;
    }

    PRINT("\n");
}

const String CoAPOption::toString() const {
    String s;

    for(int i = 0; i < value_.size(); ++i){
        s += char(value_[i]);
    }

    return s;
}

const Block2 CoAPOption::toBlock2() const {
    Block2 result;

    switch (value_.size()) {
        case 1:
            result.num = value_[0] >> 4;
            result.m = (unsigned int) ((value_[0] >> 3) & 0x01);
            result.szx = (unsigned int) (value_[0] & 0x07) + 4;
            break;
        case 2:
            // TODO
            break;
        case 3:
            // TODO
            break;
        default:
            // TODO
            break;
    }

    return result;
}
