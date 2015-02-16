#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "usage: run_test.sh [packet-size (kb)]"
  exit 1
fi

ps="$1"
EXEC_DIR="/home/danghu/miniature-ryu"
DATA="/home/danghu/miniature-ryu/server/data"
LOG="/home/danghu/miniature-ryu/server/log"
servers=( "node85" "node86" )


function server { 
echo "start server $1"
ssh $1 "nohup $EXEC_DIR/server/async_server.py $DATA/$1.txt \
               >$LOG/$1-$2.log 2>&1 &"
}

for i in "${servers[@]}"
do
  server $i $ps
done


function client {
echo "run client $1"
ssh $1 "nohup $EXEC_DIR/client/async_client.py $2 host86 $ps $DATA/$2.txt &"
}

client "node81" 1
echo "Terminate servers"

for i in "${servers[@]}"
do
  pid=`ssh $i pgrep async_server`
  ssh $i "kill -INT $pid"
done

echo "Hamming Distance"
d=`diff -u $DATA/node85.txt $DATA/node86.txt | grep ^[+-].[0-9] | wc -l`
echo $d
echo "Experiment ended."
