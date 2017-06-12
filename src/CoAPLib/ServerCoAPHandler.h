#ifndef COAPLIB_SERVERCOAPHANDLER_H
#define COAPLIB_SERVERCOAPHANDLER_H


#include "CoAPMessage.h"
#include "../RadioLib/RadioMessage.hpp"

class ServerCoAPHandler {
private:
    CoAPMessage* coAP_message_to_send_;
    RadioMessage* radio_message_to_send_;

    Array<CoAPMessage> pending_messages_;

    void handleRequest(CoAPMessage &message);
    void handleGet(CoAPMessage &message);
    void handlePut(CoAPMessage &message);
    void handleBadRequest(CoAPMessage &message);

    RadioMessage* createRadioMessage(unsigned short message_id, int code, String uri) const;

public:
    ServerCoAPHandler() : coAP_message_to_send_(nullptr), radio_message_to_send_(nullptr) {}

    void handleMessage(CoAPMessage &message);
    void createResponse(RadioMessage &radioMessage);

    CoAPMessage *popCoAPMessageToSend();
    RadioMessage *popRadioMessageToSend();
};


#endif //COAPLIB_SERVERCOAPHANDLER_H
