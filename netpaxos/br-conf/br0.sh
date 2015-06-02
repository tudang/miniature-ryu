# ovs script to configure network on sw221
ovs-vsctl del-br br0
ovs-vsctl add-br br0 -- set bridge br0 datapath_type=pica8
ovs-ofctl del-flows br0
ovs-vsctl add-port br0 ge-1/1/1 vlan_mode=access tag=1 -- set Interface ge-1/1/1 type=pica8
ovs-vsctl add-port br0 ge-1/1/2 vlan_mode=access tag=1 -- set Interface ge-1/1/2 type=pica8
ovs-vsctl add-port br0 ge-1/1/3 vlan_mode=access tag=1 -- set Interface ge-1/1/3 type=pica8
ovs-vsctl add-port br0 ge-1/1/4 vlan_mode=access tag=1 -- set Interface ge-1/1/4 type=pica8
ovs-vsctl add-port br0 ge-1/1/5 vlan_mode=access tag=1 -- set Interface ge-1/1/5 type=pica8
ovs-vsctl add-port br0 ge-1/1/6 vlan_mode=access tag=1 -- set Interface ge-1/1/6 type=pica8
ovs-vsctl add-port br0 ge-1/1/7 vlan_mode=access tag=1 -- set Interface ge-1/1/7 type=pica8
ovs-vsctl add-port br0 ge-1/1/8 vlan_mode=access tag=1 -- set Interface ge-1/1/8 type=pica8
ovs-vsctl set-controller br0 tcp:192.168.3.84:6633
