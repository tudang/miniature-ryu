#!/bin/sh
#sync clocks
sudo ntpdate -u gateway &
#ssh node82 -t "sudo ntpdate -u gateway"
ssh node81 -t "sudo ntpdate -u gateway"
ssh node90 -t "sudo ntpdate -u gateway"
ssh node91 -t "sudo ntpdate -u gateway"

DIR="/home/danghu/miniature-ryu/netpaxos"
ssh node81 "nohup $DIR/client host91 8888 $1 81 > $DIR/node81.log 2>&1 &"
ssh node82 "$DIR/client host91 8888 $1 82"
sleep 10
./copy_output.sh
