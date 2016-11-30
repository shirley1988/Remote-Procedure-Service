#ifndef _RPC_H_
#define _RPC_H_

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
class RPC {
public:
    // constructor
    RPC(char *host, unsigned short port, int timeout = 5, int maxTry = 3);
    int send(const void *buffer, size_t size);  // RPC send for client
    int recv(void *buffer, size_t size);   // RPC receive for client
    int getPktId();    // return a packet id to initialize a packet

private:
    int sockfd;     // socket id
    struct sockaddr_in sin;   // IP address
    socklen_t slen;     // length of sockaddr_in
    int pktId;  // packet id
    int maxTry; // maximum retry times
};

#endif
