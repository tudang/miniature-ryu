#!/bin/sh

DIR="/home/danghu/miniature-ryu/netpaxos"
LOG="$DIR/log/$1-$2-$(date "+%T")"
mkdir -p $LOG
ssh node80 "nohup iperf -su > $LOG/server80.log 2>&1 &"
ssh node84 "nohup iperf -su > $LOG/server84.log 2>&1 &"
ssh node86 "nohup iperf -su > $LOG/server86.log 2>&1 &"
ssh node76 "nohup iperf -su > $LOG/server76.log 2>&1 &"
ssh node71 "nohup iperf -su > $LOG/server71.log 2>&1 &"
ssh node83 "nohup iperf -c 192.168.4.80 -u -b 800m -t 30 > $LOG/client83.log 2>&1 &"
ssh node85 "nohup iperf -c 192.168.4.80 -u -b 800m -t 30 > $LOG/client85.log 2>&1 &"
ssh node88 "nohup iperf -c 192.168.4.80 -u -b 800m -t 30 > $LOG/client88.log 2>&1 &"
ssh node77 "nohup iperf -c 192.168.4.80 -u -b 800m -t 30 > $LOG/client77.log 2>&1 &"
ssh node72 "nohup iperf -c 192.168.4.80 -u -b 800m -t 30 > $LOG/client72.log 2>&1 &"
ssh node81 "nohup $DIR/client -n $1 -t $2 -c 81 > $LOG/node81.log 2>&1 &"
ssh node82 "$DIR/client  -n $1 -t $2 -c 82"
ssh node80 "pkill iperf"
ssh node84 "pkill iperf"
ssh node86 "pkill iperf"
ssh node76 "pkill iperf"
ssh node71 "pkill iperf"
#sleep 20
#ssh node90 "cp /tmp/server1.txt $LOG"
#ssh node91 "cp /tmp/server2.txt $LOG"
#diff $LOG/server2.txt $LOG/server1.txt | wc
