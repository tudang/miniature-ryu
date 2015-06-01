#!/bin/sh
ip link add link eth0 name eth0.9 type vlan id 9
ip addr add 192.168.4.91/24 brd 192.168.4.255 dev eth0.9
ip link set dev eth0.9 up

ip link add link eth1 name eth1.2 type vlan id 2
ip addr add 192.168.4.91/24 brd 192.168.4.255 dev eth1.2
ip link set dev eth1.2 up

ip link add link eth2 name eth2.3 type vlan id 3
ip addr add 192.168.4.91/24 brd 192.168.4.255 dev eth2.3
ip link set dev eth2.3 up

ip link add link eth3 name eth3.4 type vlan id 4
ip addr add 192.168.4.91/24 brd 192.168.4.255 dev eth3.4
ip link set dev eth3.4 up

# to turn down the device
# ip link set dev eth0.5 down

# to remove the device
#ip link delete eth0.5
