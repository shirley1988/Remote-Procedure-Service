#ifndef _SERVICE_H_
#define _SERVICE_H_

#include "rpc.h"

/* Define parameters used by both server and client in Remote Procedure Calls.
 * Two operations are supported, cartesian_distance() and midpoint(). functions
 * not interchangable. distance_slnt() and midpoint_clnt() are client side 
 * functions which marshall and unmarshall the parameters. distance_svc() and 
 * midpoint_svc() are server side funtions which would take Packet from client
 * can return Message to client.
 */

// defind Operation
enum Operation {
    Distance = 0,
    Midpoint = 1
};

// define Point
typedef struct s_Point {
    double x;
    double y;
} Point;

// define Packet sending from client to server
typedef struct s_Packet {
    int id;
    Operation op;
    Point a;
    Point b;
} Packet;

// define Message sending from server to client
typedef struct s_Message {
    int id;
    int rcode;
    double distance;
    Point midpoint;
} Message;

// client side functions
int distance_clnt(RPC* rpc, Point* a, Point* b, double* result);
int midpoint_clnt(RPC* rpc, Point* a, Point* b, Point* result);

// server side functions
int distance_svc(Point *p1, Point *p2, double *result);
int midpoint_svc(Point *p1, Point *p2, Point *result);

#endif
