#!/bin/sh

ip addr add 192.168.4.90/24 brd 192.168.4.255 dev eth1
ip link set dev eth1 up

ip addr add 192.168.5.90/24 brd 192.168.5.255 dev eth2
ip link set dev eth2 up

ip addr add 192.168.6.90/24 brd 192.168.6.255 dev eth3
ip link set dev eth3 up

# to turn down the device
# ip link set dev eth0.5 down

# to remove the device
#ip link delete eth0.5
