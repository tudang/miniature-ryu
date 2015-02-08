import sys
import signal
import array
from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller.handler import MAIN_DISPATCHER
from ryu.controller.handler import CONFIG_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3, ether
from ryu.lib.packet import packet, arp, ethernet

def signal_handler(signal, frame):
    print 'You pressed Ctrl+C!'
    sys.exit(0)

class DemoSwitch(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

    def __init__(self, *args, **kwargs):
        super(DemoSwitch, self).__init__(*args, **kwargs)
        signal.signal(signal.SIGINT, signal_handler)
    
    def send_craft_packet():    
        e = ethernet.ethernet(dst='ff:ff:ff:ff:ff:ff',
                              src='08:60:6e:7f:74:e7',
                              ethertype=ether.ETH_TYPE_ARP)
        a = arp.arp(hwtype=1, proto=0x0800, hlen=6, plen=4, opcode=2,
                    src_mac='08:60:6e:7f:74:e7', src_ip='192.0.2.1',
                    dst_mac='00:00:00:00:00:00', dst_ip='129.0.2.2')
        p = packet.Packet()
        p.add_protocol(e)
        p.add_protocol(a)
        p.serialize()
        print repr(p.data) # the on-wire packet
    
    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def packet_in_handler(self, ev):
        pkt = packet.Packet(array.array('B', ev.msg.data))
        

    @set_ev_cls(ofp_event.EventOFPSwitchFeatures, CONFIG_DISPATCHER)
    def switch_features_handler(self, ev):
        msg = ev.msg

        self.logger.debug('OFPSwitchFeatures received: '
                          'datapath_id=0x%016x n_buffers=%d '
                          'n_tables=%d auxiliary_id=%d '
                          'capabilities=0x%08x',
                          msg.datapath_id, msg.n_buffers, msg.n_tables,
                          msg.auxiliary_id, msg.capabilities)
        self.send_get_config_request(msg.datapath)

    def send_get_config_request(self, datapath):
        ofp_parser = datapath.ofproto_parser

        req = ofp_parser.OFPGetConfigRequest(datapath)
        datapath.send_msg(req)

    @set_ev_cls(ofp_event.EventOFPGetConfigReply, MAIN_DISPATCHER)
    def get_config_reply_handler(self, ev):
        msg = ev.msg
        dp = msg.datapath
        ofp = dp.ofproto
        flags = []

        if msg.flags & ofp.OFPC_FRAG_NORMAL:
            flags.append('NORMAL')
        if msg.flags & ofp.OFPC_FRAG_DROP:
            flags.append('DROP')
        if msg.flags & ofp.OFPC_FRAG_REASM:
            flags.append('REASM')
        self.logger.debug('OFPGetConfigReply received: '
                          'flags=%s miss_send_len=%d',
                          ','.join(flags), msg.miss_send_len)


