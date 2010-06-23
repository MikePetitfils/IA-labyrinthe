#!/bin/sh

echo "starting up"
./server &
echo "starting client"
./client 127.0.0.1 IHM
echo "stop"
