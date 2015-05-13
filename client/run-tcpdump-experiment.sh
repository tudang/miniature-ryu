#!/bin/sh
show_help() {
cat << EOF
Usage: ${0##*/} [-hv] [-p Packet-size] [-b throughput] 
    -h          display this help and exit
    -p Packet-size   Packet-size in kB
    -b Throughput    The client throughput target in Mbps
    -v          verbose mode. Can be used multiple times for increased
                verbosity.
EOF
}

# Initialize our own variables:
dir="pcap"
PS=4
BW=10
N=10
verbose=0
save=false
OPTIND=1 # Reset is necessary if getopts was used previously in the script.  It is a good idea to make this local in a function.
while getopts "hvsn:" opt ; do
    case "$opt" in
        h)
            show_help
            exit 0
             ;;
        s)
            save=true
             ;;
        v)  verbose=$((verbose+1))
             ;;
        n)  N=$OPTARG
             ;;
        '?')
            show_help >&2
            exit 1
             ;;
    esac
done
shift "$((OPTIND-1))" # Shift off the options and optional --.
#sync clocks
sudo ntpdate gateway &
ssh node81 -t "sudo ntpdate gateway"
ssh node90 -t "sudo ntpdate gateway"
ssh node91 -t "sudo ntpdate gateway"

ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth0.1   -w /tmp/eth0.1.pcap \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth1.2 -w /tmp/eth1.2.pcap \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth2.3 -w /tmp/eth2.3.pcap \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth3.4 -w /tmp/eth3.4.pcap \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node90 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth0.5 -w /tmp/eth0.5.pcap \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node90 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth1.6 -w /tmp/eth1.6.pcap \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node90 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth2.7 -w /tmp/eth2.7.pcap \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node90 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth3.8 -w /tmp/eth3.8.pcap \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
echo "./client host91 8888 $N"
ssh node81 'nohup bash -c "miniature-ryu/client/client host91 8888 '$N' > miniature-ryu/client/node81.log 2>&1 &"'
./client host91 8888 $N

sleep 5

ssh node90 -t "sudo pkill tcpdump"
ssh node91 -t "sudo pkill tcpdump"
#awk -F, '{ print $1 }' data.txt > client.txt
#ssh node91 "cd /tmp  ; ./parse_tcpdump.sh -f out-eth0 eth0 ; ./parse_tcpdump.sh -f out-eth1.2 eth1.2 ; ./parse_tcpdump.sh -f out-eth2.3 eth2.3 ; ./parse_tcpdump.sh -f out-eth3.4 eth3.4 "
#ssh node91 "cd /tmp  ; ./parse_tcpdump.sh -f out-eth0.1 eth0.1.pcap ; ./parse_tcpdump.sh -f out-eth1.2 eth1.2.pcap ; ./parse_tcpdump.sh -f out-eth2.3 eth2.3.pcap ; ./parse_tcpdump.sh -f out-eth3.4 eth3.4.pcap "
#ssh node90 "cd /tmp  ; sh parse_tcpdump.sh -f out-eth0.5 eth0.5.pcap ; ./parse_tcpdump.sh -f out-eth1.6 eth1.6.pcap ; ./parse_tcpdump.sh -f out-eth2.7 eth2.7.pcap  ; ./parse_tcpdump.sh -f out-eth3.8 eth3.8.pcap "

#copy pcap files
if [ "$save" = true ]; then
  mkdir -p /home/danghu/$dir
  ssh node90 "cp /tmp/*.pcap ~/$dir/"
  ssh node91 "cp /tmp/*.pcap ~/$dir/"
  ssh node81 "cp client1.csv ~/$dir/ ; rm client1.csv"
  cp client2.csv ~/$dir/
fi
