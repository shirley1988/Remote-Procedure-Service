#!/bin/sh

g++ client.cpp rpc.cpp -o client -Werror -Wall -pedantic
g++ server.cpp -o server -Werror -Wall -pedantic
