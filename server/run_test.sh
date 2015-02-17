#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "usage: run_test.sh [packet-size (kb)]"
  exit 1
fi

ps="$1"
EXEC_DIR="/home/danghu/miniature-ryu"
DATA="/home/danghu/miniature-ryu/server/data"
LOG="/home/danghu/miniature-ryu/server/log"
servers=( "node86" "node88" )
clients=( "node81" "node82" )


function server { 
echo "start server $1"
ssh $1 "nohup $EXEC_DIR/server/async_server.py $DATA/$1.txt \
               >$LOG/$1.log 2>&1 &"
}

function client {
echo "run client $1"
ssh $1 "nohup $EXEC_DIR/client/async_client.py $2 host86 $ps \
                $DATA/$1.txt >$LOG/$1.log 2>&1 &"
}

#clean data and log
rm $LOG/* $DATA/*

for i in "${servers[@]}"
do
  server $i $ps
done

cid=1
for i in "${clients[@]}"
do
  client $i $cid
  cid=$[cid+1]
done

sleep 15
echo "Terminate servers"

for i in "${servers[@]}"
do
  pid=`ssh $i pgrep async_server`
  ssh $i "kill -INT $pid"
done

echo "Hamming Distance"
set -- "node86" "node88" 
for a; do 
  shift
  for b; do
    d=`diff -u $DATA/$a.txt $DATA/$b.txt | grep ^[+-].[0-9] | wc -l`
    printf "%s - %s: %d\n" "$a" "$b" $d
  done
done
#d=`diff -u $DATA/node86-$ps.txt $DATA/node70-$ps.txt | grep ^[+-].[0-9] | wc -l`
#echo $d
#d=`diff -u $DATA/node70-$ps.txt $DATA/node88-$ps.txt | grep ^[+-].[0-9] | wc -l`
#echo $d
echo "Experiment ended."

wc data/* 
tail log/*
