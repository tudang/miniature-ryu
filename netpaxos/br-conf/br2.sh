ovs-vsctl del-br br2

ovs-vsctl add-br br2 -- set bridge br2 datapath_type=pica8
ovs-vsctl add-port br2 ge-1/1/16 vlan_mode=access tag=1 -- set Interface ge-1/1/16 type=pica8
ovs-vsctl add-port br2 ge-1/1/17 vlan_mode=access tag=1 -- set Interface ge-1/1/17 type=pica8
ovs-vsctl add-port br2 ge-1/1/18 vlan_mode=trunk trunks=1,7 -- set Interface ge-1/1/18 type=pica8
ovs-vsctl add-port br2 ge-1/1/19 vlan_mode=trunk trunks=1,3 -- set Interface ge-1/1/19 type=pica8
ovs-vsctl add-port br2 ge-1/1/20 vlan_mode=access tag=1 -- set Interface ge-1/1/20 type=pica8
ovs-vsctl add-port br2 ge-1/1/21 vlan_mode=access tag=1 -- set Interface ge-1/1/21 type=pica8
ovs-vsctl add-port br2 ge-1/1/35 vlan_mode=access tag=1 -- set Interface ge-1/1/35 type=pica8
ovs-vsctl add-port br2 ge-1/1/36 vlan_mode=access tag=1 -- set Interface ge-1/1/36 type=pica8
ovs-vsctl add-port br2 ge-1/1/39 vlan_mode=access tag=1 -- set Interface ge-1/1/39 type=pica8
ovs-vsctl add-port br2 ge-1/1/40 vlan_mode=access tag=1 -- set Interface ge-1/1/40 type=pica8
ovs-vsctl set-controller br2 tcp:192.168.3.84:6633
