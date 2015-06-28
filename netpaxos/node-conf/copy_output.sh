#!/bin/sh
#copy pcap files
scp danghu@node90:/tmp/server1.txt $1
scp danghu@node91:/tmp/server2.txt $1
diff $1/server2.txt $1/server1.txt | wc
