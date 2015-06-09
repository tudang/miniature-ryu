#!/bin/sh

DIR="/home/danghu/miniature-ryu/netpaxos"
LOG="$DIR/log/$1-$2-$(date "+%T")"
mkdir -p $LOG
ssh node80 "nohup iperf -su > $LOG/iperf-server.log 2>&1 &"
ssh node83 "nohup iperf -c 192.168.4.80 -u -b 800m -t 60 > $LOG/iperf-client.log 2>&1 &"
ssh node81 "nohup $DIR/client -n $1 -t $2 -c 81 > $LOG/node81.log 2>&1 &"
ssh node82 "$DIR/client  -n $1 -t $2 -c 82"
ssh node80 "pkill iperf"
ssh node83 "pkill iperf"
#sleep 20
#ssh node90 "cp /tmp/server1.txt $LOG"
#ssh node91 "cp /tmp/server2.txt $LOG"
#diff $LOG/server2.txt $LOG/server1.txt | wc
