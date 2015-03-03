#!/bin/sh
show_help() {
cat << EOF
Usage: ${0##*/} [-hv] [-p Packet-size] [-b throughput] 
    -h          display this help and exit
    -p Packet-size   Packet-size to send
    -b Throughput    The client throughput target
    -v          verbose mode. Can be used multiple times for increased
                verbosity.
EOF
}

# Initialize our own variables:
PS=4
BW=10
verbose=0

OPTIND=1 # Reset is necessary if getopts was used previously in the script.  It is a good idea to make this local in a function.
while getopts "hvpb:" opt; do
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
ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth1.2 -w /tmp/eth1.2 \( udp dst port 8888 \) > /dev/null 2>&1 &"'
ssh node91 -t 'sudo nohup bash -c "tcpdump -nnts 1514 -i eth2.3 -w /tmp/eth2.3 \( udp dst port 8888 \) > /dev/null 2>&1 &"'
./udpclient.py node91 --ps $[$PS*1024] --bw $BW
ssh node91 -t "sudo pkill tcpdump"
ssh node91 "cd /tmp ; ./parse_tcpdump.sh -f out-eth2.3 eth2.3; ./parse_tcpdump.sh -f out-eth1.2 eth1.2"
