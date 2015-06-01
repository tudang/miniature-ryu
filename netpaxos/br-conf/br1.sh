ovs-vsctl del-br br1
ovs-vsctl add-br br1 -- set bridge br1 datapath_type=pica8
ovs-vsctl add-port br1 ge-1/1/11 vlan_mode=access tag=1 -- set Interface ge-1/1/11 type=pica8
ovs-vsctl add-port br1 ge-1/1/13 vlan_mode=trunk trunks=1,6 -- set Interface ge-1/1/13 type=pica8
ovs-vsctl add-port br1 ge-1/1/14 vlan_mode=trunk trunks=1,2 -- set Interface ge-1/1/14 type=pica8
ovs-vsctl add-port br1 ge-1/1/33 vlan_mode=trunk trunks=1,6 -- set Interface ge-1/1/33 type=pica8
ovs-vsctl add-port br1 ge-1/1/34 vlan_mode=trunk trunks=1,6 -- set Interface ge-1/1/34 type=pica8
ovs-vsctl add-port br1 ge-1/1/37 vlan_mode=trunk trunks=1,2 -- set Interface ge-1/1/37 type=pica8
ovs-vsctl add-port br1 ge-1/1/38 vlan_mode=trunk trunks=1,2 -- set Interface ge-1/1/38 type=pica8
ovs-vsctl set-controller br1 tcp:192.168.3.84:6633
