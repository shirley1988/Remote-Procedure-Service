#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "service.h"
#include <iostream>
#include "rpc.h"

/* The client take server host and port from command-line as arguments and will read points 
 * from standard input(each line contains four float-point numbers). The client will marshall
 * and unmarshall the arguments and allocate space for results. The client uses methods of RPC
 * to perform network communication. client has timeout to be 5 seconds and maximum retry times
 * to be 3.
 */

int main(int argc, char* argv[]) {
    // validate arguments
    if (argc != 3) {
        perror("usage");
        exit(-1);
    }

    // initialize RPC
    char *host = argv[1];
    unsigned short port = atoi(argv[2]);
    RPC rpc = RPC(host, port);

    // read in two Points from standard input
    Point p1, p2;
    while (1) {
        double x1, y1, x2, y2;
        std::cin >> x1 >> y1 >> x2 >> y2;
        p1.x = x1;
        p1.y = y1;
        p2.x = x2;
        p2.y = y2;
        
        // allocate results' memory
        double distance;
        Point midpoint;
        // call server to perform operations and get result from it
        int rcode1 = distance_clnt(&rpc, &p1, &p2, &distance);
        int rcode2 = midpoint_clnt(&rpc, &p1, &p2, &midpoint);

        if (rcode1 < 0 || rcode2 < 0) {
            // if any return code is negative, there is something wrong with remote server
            // we will ignore the return result
            fprintf(stderr, "remote server error\n");
        } else {
            // print result to standard output
            printf("(%.6f, %.6f), (%.6f, %.6f), %.6f, (%.6f, %.6f)\n", 
                    x1, y1, x2, y2, distance, midpoint.x, midpoint.y);
        }
        
    }
    return 0;
}


/**
 * Client side function to compute distance, It will pack paramters and send
 * to remote server. Then wait for the reponse and unpack parameters.
 */
int distance_clnt(RPC *rpc, Point *p1, Point *p2, double *result) {
    int id = rpc->getPktId();   // get packet id
    // initialize a packet
    Packet pkt;
    pkt.a = *p1;
    pkt.b = *p2;
    pkt.id = id;
    pkt.op = Distance;
    // call RPC send() to send packet to server
    int outBytes = rpc->send(&pkt, sizeof(Packet));
    if (outBytes < 0) {
        return -1;
    }
    // call RPC recv() to receive message from server
    Message msg;
    int inBytes = rpc->recv(&msg, sizeof(Message));
    if (inBytes < 0) {
        return -1;
    }
    // validate id between packet sended and message received
    if (msg.id != id) {
        fprintf(stderr, "received pakcet id is not send packet id\n");
        return -1;
    }
    // validate if operations conducted by server
    if (msg.rcode >= 0) {
        *result = msg.distance;
    }
    return msg.rcode;
}


/**
 * Client side function to compute midpoint, It will pack paramters and send
 * to remote server. Then wait for the reponse and unpack parameters.
 */
int midpoint_clnt(RPC *rpc, Point *p1, Point *p2, Point *result) {
    int id = rpc->getPktId(); // return packet id
    // initiate a packet
    Packet pkt;
    pkt.a = *p1;
    pkt.b = *p2;
    pkt.id = id;
    pkt.op = Midpoint;
    // call RPC send() to send packet to server
    int outBytes = rpc->send(&pkt, sizeof(Packet));
    if (outBytes < 0) {
        return -1;
    }
    // call RPC recv() to receive message from server
    Message msg;
    int inBytes = rpc->recv(&msg, sizeof(Message));
    if (inBytes < 0) {
        return -1;
    }
    // validate id between packet sended and message received
    if (msg.id != id) {
        fprintf(stderr, "received pakcet id is not equal to send packet id\n");
        return -1;
    }
    // check if the operation desired has performed
    if (msg.rcode >= 0) {
        *result = msg.midpoint;
    }
    return msg.rcode;
}
