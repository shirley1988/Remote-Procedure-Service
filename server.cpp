#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include "service.h"
#include <math.h>
#include <iostream>

using namespace std;

/* The server takes the port number as an argument. The server will receive UDP packets
 * on the specific port and sends UDP replies to the sender. To simulate failures, if 
 * either of the two Points in packet received is in the left half-plane, server will
 * ignore the request(so the client will timeout). if Points are in the lower right 
 * quardrant, server will return -1 to indicate error.
 */

int main(int argc, char* argv[]) {
    // validate arguments
    if (argc < 2) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(-1);
    }
    // get socket id
    int serverfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("server socket created\n");
    // initiate a server port
    short port = atoi(argv[1]);
    struct sockaddr_in serversin;
    serversin.sin_family = AF_INET;
    serversin.sin_addr.s_addr = INADDR_ANY;
    serversin.sin_port = htons(port);
    // bind the port to socket
    if ( bind(serverfd, (struct sockaddr*)&serversin, sizeof(serversin)) < 0 ) {
        fprintf(stderr, "bind failed");
        exit(-1);
    }
    printf("binding succeed\n");
    
    // keep client IP address
    struct sockaddr_in clientaddr;
    socklen_t clen = sizeof (clientaddr);
    while (true) {
        Packet recvPacket;
        int inBytes = 0;
        // receive packet from client side and check if recvfrom succeeds
        if ( (inBytes = recvfrom(serverfd, (void *)&recvPacket, sizeof(Packet), 0,
                                 (struct sockaddr*)&clientaddr, (socklen_t*)&clen)) < 0 ) {
            fprintf(stderr, "error recvfrom\n");
            exit(-1);
        }
        // check if packet received is correct format
        if (inBytes != sizeof(Packet)) {
            fprintf(stderr, "data lost in packet, ignoring\n");
            continue;
        }
        // get two points from packet
        Point p1 = recvPacket.a;
        Point p2 = recvPacket.b;
        printf("Serevr received pakcage %d\n", recvPacket.id);

        // ignore requsts if any point is in left plane
        if ( p1.x < 0 || p2.x < 0 ) {
            fprintf(stderr, "Ignoring request because one point is in left plane\n");
            continue;
        }
        // initialize a message to be returned to client
        Message rmessage;
        rmessage.id = recvPacket.id;
        if (recvPacket.op == Distance) {   // perform distance operation
            fprintf(stdout, "Computing distance\n");
            rmessage.rcode = distance_svc(&p1, &p2, &rmessage.distance);
        } else if (recvPacket.op == Midpoint ) {  // perform midpoint operation
            fprintf(stdout, "Computing midpoint\n");
            rmessage.rcode = midpoint_svc(&p1, &p2, &rmessage.midpoint);
        } else {           // error happends if neither of two operations received from client
            fprintf(stderr, "Unsupported operation\n");
            rmessage.rcode = -1;
        }
        // send message to the client accoding to IP address
        if ( sendto(serverfd, &rmessage, sizeof(Message), 0, (struct sockaddr*)&clientaddr, clen) < 0 ) {
            fprintf(stderr, "error sendto\n");
            exit(-1);
        }
    }
    exit(0);
}

// distance_svc(): check if Points are in fourth quardrant and perfrom distance calculation
int distance_svc(Point* p1, Point* p2, double* result) {
    // return error for points in forth quadrant
    if ( (p1->x > 0 && p1->y < 0) || (p2->x > 0 && p2->y < 0) ) {
        fprintf(stderr, "return error because on point is in lower right (4th) quadrant\n");
        return -1;
    }
    double dx = p1->x - p2->x;
    double dy = p1->y - p2->y;
    *result = sqrt( dx * dx + dy * dy );
    return 0;
}

// midpoint_svc(): check if Points are in fourth quardrant and perform midpoint calculation
int midpoint_svc(Point* p1, Point* p2, Point* result) {
    // return error for points in forth quadrant
    if ( (p1->x > 0 && p1->y < 0) || (p2->x > 0 && p2->y < 0) ) {
        fprintf(stderr, "return error because on point is in lower right (4th) quadrant\n");
        return -1;
    }
    result->x = (p1->x + p2->x)/2;
    result->y = (p1->y + p2->y)/2;
    return 0;
}
