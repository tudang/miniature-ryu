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
PS=4
BW=10
verbose=0

OPTIND=1 # Reset is necessary if getopts was used previously in the script.  It is a good idea to make this local in a function.
while getopts "hvp:b:" opt ; do
    case "$opt" in
        h)
            show_help
            exit 0
             ;;
        v)  verbose=$((verbose+1))
             ;;
        p)  PS=$OPTARG
             ;;
        b)  BW=$OPTARG
             ;;
        '?')
            show_help >&2
            exit 1
             ;;
    esac
done
shift "$((OPTIND-1))" # Shift off the options and optional --.

ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth0   -w /tmp/eth0   \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth1.2 -w /tmp/eth1.2 \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth2.3 -w /tmp/eth2.3 \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth3.4 -w /tmp/eth3.4 \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node90 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth0.5 -w /tmp/eth0.5 \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node90 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth1.6 -w /tmp/eth1.6 \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node90 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth2.7 -w /tmp/eth2.7 \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'
ssh node90 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth3.8 -w /tmp/eth3.8 \( udp dst port 8888 \) >> /tmp/nohup.out 2>&1 &"'

echo "./udpclient.py node91 --ps $[$PS*1024] --bw $BW"
./udpclient.py node91 --ps $[$PS*1024] --bw $BW

#ssh node81 ~/miniature-ryu/client/udpclient.py --ps $[$PS*1024] --bw $BW node91

ssh node90 -t "sudo pkill tcpdump"
ssh node91 -t "sudo pkill tcpdump"
awk -F, '{ print $1 }' data.txt > client.txt
#ssh node91 "cd /tmp  ; ./parse_tcpdump.sh -f out-eth0 eth0 ; ./parse_tcpdump.sh -f out-eth1.2 eth1.2 ; ./parse_tcpdump.sh -f out-eth2.3 eth2.3 ; ./parse_tcpdump.sh -f out-eth3.4 eth3.4 "
ssh node91 "cd /tmp  ; ./parse_tcpdump.sh -f out-eth0 eth0 ; ./parse_tcpdump.sh -f out-eth1.2 eth1.2 ; ./parse_tcpdump.sh -f out-eth2.3 eth2.3 ; ./parse_tcpdump.sh -f out-eth3.4 eth3.4 "
ssh node90 "cd /tmp  ; sh parse_tcpdump.sh -f out-eth0.5 eth0.5 ; ./parse_tcpdump.sh -f out-eth1.6 eth1.6 ; ./parse_tcpdump.sh -f out-eth2.7 eth2.7  ; ./parse_tcpdump.sh -f out-eth3.8 eth3.8 "
