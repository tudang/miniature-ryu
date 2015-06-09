#!/bin/sh

DIR="/home/danghu/miniature-ryu/netpaxos"
ssh node81 "nohup $DIR/client -n $1 -t $2 -c 81 > $DIR/node81.log 2>&1 &"
ssh node82 "$DIR/client  -n $1 -t $2 -c 82"
sleep 10
./copy_output.sh
