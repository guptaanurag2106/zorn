#pragma once

#define SERVER_NAME_LEN_MAX 255
#define MAX_PAYLOAD_SIZE 1024  // bytes
#define MAX_CONNECT_RETRIES 3
#define MAX_PROTOCOL_LEN 5
#define PROTOCOL_V_STRINGIFY_HELPER(x, y) #x "." #y
#define PROTOCOL_V_STRINGIFY(x, y) PROTOCOL_V_STRINGIFY_HELPER(x, y)
#define PROTOCOL_V PROTOCOL_V_STRINGIFY(PROTOCOL_MAJORV, PROTOCOL_MINORV)

enum PACKET_TYPES { HELLO, WELCOME, REJECT, GOODBYE, PING, PONG };

static inline char *get_packet_type_string(enum PACKET_TYPES type) {
    switch (type) {
        case HELLO:
            return "HELLO";
        case WELCOME:
            return "WELCOME";
        case REJECT:
            return "REJECT";
        case GOODBYE:
            return "GOODBYE";
        case PING:
            return "PING";
        case PONG:
            return "PONG";
        default:
            return "UNKOWN";
    }
}
