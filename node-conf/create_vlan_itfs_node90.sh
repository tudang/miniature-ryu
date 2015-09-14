#!/bin/sh

ip link add link eth0 name eth0.5 type vlan id 5
ip addr add 192.168.4.91/24 brd 192.168.4.255 dev eth0.5
ip link set dev eth0.5 up

ip link add link eth1 name eth1.6 type vlan id 6
ip addr add 192.168.4.91/24 brd 192.168.4.255 dev eth1.6
ip link set dev eth1.6 up

ip link add link eth2 name eth2.7 type vlan id 7
ip addr add 192.168.4.91/24 brd 192.168.4.255 dev eth2.7
ip link set dev eth2.7 up

ip link add link eth3 name eth3.8 type vlan id 8
ip addr add 192.168.4.91/24 brd 192.168.4.255 dev eth3.8
ip link set dev eth3.8 up

# to turn down the device
# ip link set dev eth0.5 down

# to remove the device
#ip link delete eth0.5
