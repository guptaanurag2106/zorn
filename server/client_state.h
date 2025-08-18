#pragma once
#include <netinet/in.h>
#include <stdint.h>

#include "net_stuff.h"

typedef struct Message {
    char protocol_version[PROTOCOL_LEN + 1];
    uint8_t packet_type;
    uint16_t payload_length;
    char msg[MAX_MESSAGE_SIZE];  // Actually MAX_MESSAGE_SIZE -
                                 // MSG_HEADER_SIZE];
} Message;

typedef struct Client {
    int new_socket_fd;
    struct sockaddr_in client_address;
    Message last_message;
    char *buffer;

    int buffer_offset;  // Current offset in the buffer
} Client;
