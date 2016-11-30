#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "rpc.h"


/**
 * RPC: a class for remote procedure call.
 * It's constructor take the following 4 parameters:
 * @param host: the remote host IP address
 * @param port: the remote host port number
 * @param timeout: the socket timeout in seconds, default is 5 seconds
 * @param maxTry: max times of try if remote call failed, e.g. timeout
 *
 * RPC mainly implements two functions: send and recv, which will send/receive
 * packets to/from the remote server. 
 * Return value is the actual bytes sent/reseived
 *
 * RPC also has a function getPktId for caller to generate packet ids.
 * For sanity check, the id of sent/received packet should be the same
 */


// RPC constructor
RPC::RPC(char *host, unsigned short port, int timeout, int maxTry) {
    this->maxTry = maxTry;
    this->pktId = 0;

    // open socket
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0 ) {
        fprintf(stderr, "get socket error\n");
        exit(1);
    }

    // set socket timeout
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        fprintf(stderr, "error setting socket timeout\n");
        exit(1);
    }

    // set remote server IP address
    struct in_addr server_ip;
    int n = inet_aton(host, &server_ip);
    if (n == 0) {
        fprintf(stderr, "error setting up server_ip\n");
        exit(1);
    }

    memset(&this->sin, 0, sizeof(sin));
    this->sin.sin_family = AF_INET;
    this->sin.sin_addr = server_ip;
    this->sin.sin_port = htons(port);
    this->slen = sizeof(this->sin);
    printf("connect to 0x%08x, %s\n", sin.sin_addr.s_addr, inet_ntoa(sin.sin_addr));

}

// RPC sent() called by client
int RPC::send(const void *buffer, size_t size) {
    int outBytes = 0;

    // try maxTry times in case of failure
    for(int t = 0; t < this->maxTry; t++ ) {
        outBytes = sendto(this->sockfd, buffer, size, 0, (sockaddr*)&this->sin, this->slen);
        if (outBytes >= 0) {
            break;
        } else {
            fprintf(stderr, "sendto failed, retry\n");
        }
    }
    if (outBytes < 0) {
        fprintf(stderr, "max retry reached\n");
        return -1;
    }
    return outBytes;
}

// RPC recv() called by client
int RPC::recv(void *buffer, size_t size) {
    int inBytes = 0;
    
    // try maxTry times in case of failure
    for (int t = 0; t < this->maxTry; t++) {
        inBytes = recvfrom(this->sockfd, buffer, size, 0, (sockaddr *)&this->sin, &this->slen);
        if (inBytes > 0 ) {
            break;
        } else {
            fprintf(stderr, "recvfrom failed, retry\n");
        }
    }
    if (inBytes < 0 ) {
        fprintf(stderr, "max retry reached\n");
        return -1;
    }
    return inBytes;
}

// RPC getPktId(): return id of a packet
int RPC::getPktId() {
    return this->pktId++;
}

