#include <SPI.h>

#include <Ethernet.h>
#include <EthernetUdp.h>

#include <RF24Network.h>
#include <RF24.h>

#include <CoAPLib.h>
#include <RadioLib.h>

// Ethernet library variables:
IPAddress invalid(0, 0, 0, 0);
unsigned short local_port = 10000;              // Set local port
byte mac[] = {222, 222, 222, 222, 222, 222};    // Set MAC address
EthernetUDP Udp;
unsigned const int MAX_BUFFER = 64;            // Set max buffer size
unsigned char packet_buffer[MAX_BUFFER];        // Create buffer for incoming packets

// RF24 library variables:
const uint16_t this_node_id = 00;               // Address of the other node in Octal format
const uint16_t peer_node_id = 01;               // Address of this node in Octal format
const uint16_t channel = 100;                   // Channel on which transmission will occur
RF24NetworkHeader header(peer_node_id);         // Set header of radio messages
RF24 radio(7, 8);                               // nRF24L01(+) radio
RF24Network network(radio);                     // Network initialized with radio

// operator() of this struct is called when CoAPHandler needs to send a CoAP message.
// It takes care of serializing the message into ByteArray and puts it into udp packet.
struct : public CoAPMessageListener {
    void operator()(const CoAPMessage &message) {
        unsigned int size = message.serialize(packet_buffer);
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(packet_buffer, size);
        Udp.endPacket();
    }
} onCoAPMessageToSend;

// operator() of this struct is called when CoAPHandler needs to send a CoAP message.
// It sends a message using channel '100' to address '01'
struct : public RadioMessageListener {
    void operator()(const RadioMessage &message) {
        network.write(header, &message, sizeof(message));
    }
} onRadioMessageToSend;

CoAPHandler coAPHandler(onCoAPMessageToSend, onRadioMessageToSend);
unsigned short ping_interval = 10000;
unsigned long last_ping_sent = 0;
unsigned long last_timeout_check = 0;

void setup() {
    Serial.begin(115200);

    // Prepare Ethernet:
    Ethernet.begin(mac);                    // Connected to router
    //Ethernet.begin(mac, ip);              // Directly connected
    Serial.println(Ethernet.localIP());
    Udp.begin(local_port);

    // Prepare RF24:
    radio.begin();
    network.begin(channel, this_node_id);

    Serial.println("Rise and shine...");
}

void loop() {
    network.update();
    // Handle all available radio messages
    while (network.available()) {
        RadioMessage receivedMessage;
        network.read(header, &receivedMessage, sizeof(receivedMessage));
        coAPHandler.handleMessage(receivedMessage);
    }

    // Handle CoAP packet if received any
    unsigned int packet_size = Udp.parsePacket();
    if (packet_size) {
        Udp.read(packet_buffer, MAX_BUFFER);

        DEBUG_PRINT("Received ");
        DEBUG_PRINT(packet_size);
        DEBUG_PRINT(" bytes: ");

        for (int i = 0; i < packet_size; ++i) {
            DEBUG_PRINTHEX(packet_buffer[i]);
            DEBUG_PRINT(" ");
        }
        DEBUG_PRINTLN();

        CoAPMessage message;
        message.deserialize(packet_buffer, packet_size);
        coAPHandler.handleMessage(message);
    }

    // Deletes pending CoAP request if it can't be served in 5s
//    unsigned long now = millis();
//    if (now - last_timeout_check >= coAPHandler.getTimeout()) {
//        coAPHandler.deleteTimedOut();
//        last_timeout_check = now;
//    }
//    if (now - last_ping_sent >= ping_interval) {
//
//        if (Udp.remoteIP() != invalid) {
//            DEBUG_PRINT_TIME();
//            DEBUG_PRINT("PING ");
//            DEBUG_PRINT(Udp.remoteIP());
//            DEBUG_PRINT(":");
//            DEBUG_PRINTLN(Udp.remotePort());
//            coAPHandler.sendPing();
//            last_ping_sent = now;
//        }
//    }
}
