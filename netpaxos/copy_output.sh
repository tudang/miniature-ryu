#!/bin/sh
#copy pcap files
ssh node90 "cp /tmp/server1.txt ~/new-experiment/"
ssh node91 "cp /tmp/server2.txt ~/new-experiment/"
diff ~/new-experiment/server2.txt ~/new-experiment/server1.txt | wc
