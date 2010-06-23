#!/bin/sh

while true ; do
  echo "starting up"
  ./server &
  echo "starting client"
  ./client 127.0.0.1 IHM
  echo "restarting"
  sleep 1
done
