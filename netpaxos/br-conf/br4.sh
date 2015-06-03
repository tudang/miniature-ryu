ovs-vsctl del-br br4

ovs-vsctl add-br br4 -- set bridge br4 datapath_type=pica8
ovs-ofctl del-flows br4
ovs-vsctl add-port br4 ge-1/1/26 vlan_mode=access tag=1 -- set Interface ge-1/1/26 type=pica8
ovs-vsctl add-port br4 ge-1/1/27 vlan_mode=trunk trunks=1,9 -- set Interface ge-1/1/27 type=pica8
ovs-vsctl add-port br4 ge-1/1/28 vlan_mode=trunk trunks=1,9 -- set Interface ge-1/1/28 type=pica8
ovs-vsctl add-port br4 ge-1/1/29 vlan_mode=trunk trunks=1,5 -- set Interface ge-1/1/29 type=pica8
ovs-vsctl add-port br4 ge-1/1/30 vlan_mode=access tag=1 -- set Interface ge-1/1/30 type=pica8
ovs-vsctl add-port br4 ge-1/1/35 vlan_mode=access tag=1 -- set Interface ge-1/1/35 type=pica8
ovs-vsctl add-port br4 ge-1/1/36 vlan_mode=access tag=1 -- set Interface ge-1/1/36 type=pica8
ovs-vsctl add-port br4 ge-1/1/39 vlan_mode=access tag=1 -- set Interface ge-1/1/39 type=pica8
ovs-vsctl add-port br4 ge-1/1/40 vlan_mode=access tag=1 -- set Interface ge-1/1/40 type=pica8
ovs-vsctl add-port br4 ge-1/1/48 vlan_mode=access tag=1 -- set Interface ge-1/1/48 type=pica8
ovs-vsctl set-controller br4 tcp:192.168.3.84:6633
#ovs-ofctl add-flows br4 backbone.flow
