import logging
import struct
import sys
import signal
import array
from ryu import utils
from ryu.base import app_manager
from ryu.controller import mac_to_port
from ryu.controller import ofp_event
from ryu.controller.handler import CONFIG_DISPATCHER
from ryu.controller.handler import MAIN_DISPATCHER
from ryu.controller.handler import HANDSHAKE_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3
from ryu.lib.mac import haddr_to_bin
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.app import simple_switch_13
import ofutils

def signal_handler(signal, frame):
    print 'You pressed Ctrl+C!'
    sys.exit(0)

class SimpleSwitch(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

    def __init__(self, *args, **kwargs):
        super(SimpleSwitch, self).__init__(*args, **kwargs)
        signal.signal(signal.SIGINT, signal_handler)


    @set_ev_cls(ofp_event.EventOFPSwitchFeatures, CONFIG_DISPATCHER)
    def switch_features_handler(self, ev):
        msg = ev.msg

        self.logger.debug('OFPSwitchFeatures received: '
                          'datapath_id=0x%016x n_buffers=%d '
                          'n_tables=%d auxiliary_id=%d '
                          'capabilities=0x%08x',
                            msg.datapath_id, msg.n_buffers, msg.n_tables,
                            msg.auxiliary_id, msg.capabilities)
        dp = msg.datapath
        ofutils.del_all_groups(dp)
        if (msg.datapath_id == 0x678cc454444f2bd8):
                print "br0 connected"
                ofutils.add_group(dp, 1, 17, 19, 21, 23, 25, 27, 29, 31)
                ofutils.send_flow_group(dp, 1, '192.168.4.81', 
                  '192.168.4.91', 1)
                ofutils.send_flow_group(dp, 2, '192.168.4.82', 
                  '192.168.4.91', 1)
                ofutils.ip_src_dst_popvlan(dp, 0x0800, 4, '192.168.4.91', 
                                  '192.168.4.81', 1) 
                ofutils.ip_src_dst_popvlan(dp, 0x0800, 4, '192.168.4.91', 
                                  '192.168.4.82', 2) 
                ofutils.forward_ports(dp, 17, 18)
                ofutils.forward_ports(dp, 19, 20)
                ofutils.mod_eth_dst_vlan(dp, 5, 18, 'D4:AE:52:EA:4C:23', 9, 13)
                ofutils.mod_eth_dst_vlan(dp, 5, 20, 'D4:AE:52:EA:49:E3', 5, 9)

                # send to eth1.6(node90)
                ofutils.forward_ports(dp, 21, 22)
                ofutils.mod_eth_dst_vlan(dp, 5, 22, 'D4:AE:52:EA:49:E5', 6, 10)
                # send to eth1.2(node91)
                ofutils.forward_ports(dp, 23, 24)
                ofutils.mod_eth_dst_vlan(dp, 5, 24, 'D4:AE:52:EA:4C:25', 2, 14)
                # send to eth2.7(node90)
                ofutils.forward_ports(dp, 25, 26)
                ofutils.mod_eth_dst_vlan(dp, 5, 26, 'D4:AE:52:EA:49:E7', 7, 11)
                # send to eth2.3(node91)
                ofutils.forward_ports(dp, 27, 28)
                ofutils.mod_eth_dst_vlan(dp, 5, 28, 'D4:AE:52:EA:4C:27', 3, 15)
                # send to eth3.8(node90)
                ofutils.forward_ports(dp, 29, 30)
                ofutils.mod_eth_dst_vlan(dp, 5, 30, 'D4:AE:52:EA:49:E9', 8, 12)
                # send to eth3.4(node91)
                ofutils.forward_ports(dp, 31, 32)
                ofutils.mod_eth_dst_vlan(dp, 5, 32, 'D4:AE:52:EA:4C:29', 4, 16)
                #outbound traffic
                ofutils.ip_src_dst(dp, 0x0800, 3, '192.168.3.90', 
                                  ('192.168.3.0', '255.255.255.0'),[48]) 
                ofutils.ip_src_dst(dp, 0x0800, 3, '192.168.3.91', 
                                  ('192.168.3.0', '255.255.255.0'),[48]) 
                ofutils.ip_src_dst(dp, 0x0800, 4, '192.168.4.91', 
                                  '192.168.4.82', [2]) 
                ofutils.ip_src_dst(dp, 0x0800, 4, '192.168.4.91', 
                                  '192.168.4.81', [1]) 
                #inbound traffic
                ofutils.send_flow(dp, 0x0800, 3, '192.168.3.91', [13])
                ofutils.send_flow(dp, 0x0800, 3, '192.168.3.90', [9])
                ofutils.send_arp_flow(dp, 4, 
                  ('192.168.3.0', '255.255.255.0'), [9,13,48]) # port 28 to br0
                ofutils.send_arp_flow(dp, 4, 
                  ('192.168.4.0', '255.255.255.0'), [1,2,9,13]) # port 28 to br0
        

        elif (msg.datapath_id == 0xca13c454444f2bab):
                print "br1 connected"
                ofutils.add_group(dp, 1, 33, 37)
                ofutils.send_flow_group(dp, 11, '192.168.4.81', 
                  '192.168.4.91', 1)
                ofutils.send_flow_group(dp, 11, '192.168.4.82', 
                  '192.168.4.91', 1)
                # send to eth1.6(node90)
                ofutils.forward_ports(dp, 33, 34)
                ofutils.mod_eth_dst_vlan(dp, 5, 34, 'D4:AE:52:EA:49:E5', 6, 13)
                # send to eth1.2(node91)
                ofutils.forward_ports(dp, 37, 38)
                ofutils.mod_eth_dst_vlan(dp, 5, 38, 'D4:AE:52:EA:4C:25', 2, 14)

                ofutils.send_normal_flow(dp, 9, '192.168.4.85', 10)
                ofutils.send_normal_flow(dp, 10, '192.168.4.84', 9)

        elif (msg.datapath_id == 0xa182c454444f2bab):
                print "br2 connected"
                ofutils.add_group(dp, 1, 35, 39)
                ofutils.send_flow_group(dp, 16, '192.168.4.81', 
                  '192.168.4.91', 1)
                ofutils.send_flow_group(dp, 16, '192.168.4.82', 
                  '192.168.4.91', 1)
                # send to eth2.7(node90)
                ofutils.forward_ports(dp, 35, 36)
                ofutils.mod_eth_dst_vlan(dp, 5, 36, 'D4:AE:52:EA:49:E7', 7, 18)
                # send to eth2.3(node91)
                ofutils.forward_ports(dp, 39, 40)
                ofutils.mod_eth_dst_vlan(dp, 5, 40, 'D4:AE:52:EA:4C:27', 3, 19)

                ofutils.send_normal_flow(dp, 20, '192.168.4.86', 21)
                ofutils.send_normal_flow(dp, 21, '192.168.4.88', 20)

        elif (msg.datapath_id == 0x3079c454444f2bc4):
                print "br3 connected"
                ofutils.add_group(dp, 1, 33, 37)
                ofutils.send_flow_group(dp, 21, '192.168.4.81', 
                  '192.168.4.91', 1)
                ofutils.send_flow_group(dp, 21, '192.168.4.82', 
                  '192.168.4.91', 1)
                # send to eth3.8(node90)
                ofutils.forward_ports(dp, 33, 34)
                ofutils.mod_eth_dst_vlan(dp, 5, 34, 'D4:AE:52:EA:49:E9', 8, 23)
                # send to eth3.4(node91)
                ofutils.forward_ports(dp, 37, 38)
                ofutils.mod_eth_dst_vlan(dp, 5, 38, 'D4:AE:52:EA:4C:29', 4, 24)

                ofutils.send_normal_flow(dp, 20, '192.168.4.77', 19)
                ofutils.send_normal_flow(dp, 19, '192.168.4.76', 20)

        elif (msg.datapath_id == 0x59abc454444f2bc4):
                print "br4 connected"
                ofutils.add_group(dp, 1, 35, 39)
                ofutils.send_flow_group(dp, 28, '192.168.4.81', 
                  '192.168.4.91', 1)
                ofutils.send_flow_group(dp, 28, '192.168.4.82', 
                  '192.168.4.91', 1)
                # output to interface eth0.5(node90)
                ofutils.forward_ports(dp, 35, 36)
                ofutils.forward_ports(dp, 39, 40)
                ofutils.mod_eth_dst_vlan(dp, 5, 36, 'D4:AE:52:EA:4C:23', 9, 27)
                ofutils.mod_eth_dst_vlan(dp, 5, 40, 'D4:AE:52:EA:49:E3', 5, 29)
                ofutils.send_arp_flow(dp, 4, 
                  ('192.168.3.0', '255.255.255.0'), [27, 29, 48]) # port 28 to br0

                #outbound traffic
                ofutils.ip_src_dst(dp, 0x0800, 3, '192.168.3.90', 
                                  ('192.168.3.0', '255.255.255.0'),[48]) 
                ofutils.ip_src_dst(dp, 0x0800, 3, '192.168.3.91', 
                                  ('192.168.3.0', '255.255.255.0'),[48]) 
                ofutils.ip_src_dst(dp, 0x0800, 4, '192.168.4.91', 
                                  '192.168.4.82', [28]) 
                ofutils.ip_src_dst(dp, 0x0800, 4, '192.168.4.91', 
                                  '192.168.4.81', [28]) 
                #inbound traffic
                ofutils.send_flow(dp, 0x0800, 3, '192.168.3.91', [27])
                ofutils.send_flow(dp, 0x0800, 3, '192.168.3.90', [29])
        
                ofutils.ip_src_dst(dp, 0x0800, 4,  
                                  '192.168.3.90', 
                                  '192.168.3.91',[27]) 

                ofutils.send_normal_flow(dp, 25, '192.168.4.72', 26)
                ofutils.send_normal_flow(dp, 26, '192.168.4.71', 25)

    @set_ev_cls(ofp_event.EventOFPErrorMsg,
            [HANDSHAKE_DISPATCHER, CONFIG_DISPATCHER, MAIN_DISPATCHER])
    def error_msg_handler(self, ev):
        msg = ev.msg

        self.logger.debug('OFPErrorMsg received: type=0x%02x code=0x%02x '
                          'message=%s',
                          msg.type, msg.code, utils.hex_array(msg.data))
