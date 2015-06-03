ovs-vsctl del-br br3

ovs-vsctl add-br br3 -- set bridge br3 datapath_type=pica8
ovs-vsctl add-port br3 ge-1/1/19 vlan_mode=access tag=1 -- set Interface ge-1/1/19 type=pica8
ovs-vsctl add-port br3 ge-1/1/20 vlan_mode=access tag=1 -- set Interface ge-1/1/20 type=pica8
ovs-vsctl add-port br3 ge-1/1/21 vlan_mode=access tag=1 -- set Interface ge-1/1/21 type=pica8
ovs-vsctl add-port br3 ge-1/1/22 vlan_mode=access tag=1 -- set Interface ge-1/1/22 type=pica8
ovs-vsctl add-port br3 ge-1/1/23 vlan_mode=trunk trunks=1,8 -- set Interface ge-1/1/23 type=pica8
ovs-vsctl add-port br3 ge-1/1/24 vlan_mode=trunk trunks=1,4 -- set Interface ge-1/1/24 type=pica8
ovs-vsctl add-port br3 ge-1/1/25 vlan_mode=access tag=1 -- set Interface ge-1/1/25 type=pica8
ovs-vsctl add-port br3 ge-1/1/33 vlan_mode=access tag=1 -- set Interface ge-1/1/33 type=pica8
ovs-vsctl add-port br3 ge-1/1/34 vlan_mode=access tag=1 -- set Interface ge-1/1/34 type=pica8
ovs-vsctl add-port br3 ge-1/1/37 vlan_mode=access tag=1 -- set Interface ge-1/1/37 type=pica8
ovs-vsctl add-port br3 ge-1/1/38 vlan_mode=access tag=1 -- set Interface ge-1/1/38 type=pica8
ovs-vsctl set-controller br3 tcp:192.168.3.84:6633
