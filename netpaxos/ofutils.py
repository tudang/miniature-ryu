from ryu.controller import ofp_event
from ryu.ofproto import ofproto_v1_3
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet


def send_msg(datapath, priority, match, actions):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    inst = [ofp_parser.OFPInstructionActions(ofp.OFPIT_APPLY_ACTIONS, 
                                            actions)]
    req = ofp_parser.OFPFlowMod(datapath=datapath,priority=priority,
                                match=match, instructions=inst)
    datapath.send_msg(req)


def del_all_groups(datapath):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    req = ofp_parser.OFPGroupMod(datapath, ofp.OFPGC_DELETE,
                                ofp.OFPGT_ALL, ofp.OFPG_ALL)
    datapath.send_msg(req)

def create_bucket(ofp_parser, out_port):
    max_len = 2000
    actions = [ofp_parser.OFPActionOutput(out_port, max_len)]
    weight = 100
    watch_port = 0
    watch_group = 0
    bucket = ofp_parser.OFPBucket(weight, watch_port, watch_group, actions)
    return bucket

def add_group(datapath, group_id=1, *ports):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    buckets = []
    for port in ports:
        buckets.append(create_bucket(ofp_parser, port))
    req = ofp_parser.OFPGroupMod(datapath, ofp.OFPGC_ADD,
                                 ofp.OFPGT_ALL, group_id, buckets)
    datapath.send_msg(req)


def send_flow_group(datapath, in_port, ipsrc, ipdest, group_id):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    priority = 32
    match = ofp_parser.OFPMatch(in_port=in_port, 
                                eth_type=0x0800, 
                                ipv4_src=ipsrc, ipv4_dst=ipdest)
    actions = [ofp_parser.OFPActionGroup(group_id)]
    send_msg(datapath, priority, match, actions)


def send_packet_out(datapath, buffer_id, in_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    actions = [ofp_parser.OFPActionOutput(OFPP_FLOOD, 0)]
    req = ofp_parser.OFPPacketOut(datapath, buffer_id, 
                                    in_port, actions)
    datapath.send_msg(req)

def forward_ports(datapath, in_port, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    priority = 32
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800)
    actions = [ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)


def mod_eth_dst(datapath, priority, in_port, dl_dst, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800)
    actions = [ofp_parser.OFPActionSetField(eth_dst=dl_dst),
                ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)


def mod_eth_dst_vlan(datapath, priority, in_port, dl_dst, vlan, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800)
    actions = [ofp_parser.OFPActionSetField(eth_dst=dl_dst),
               #ofp_parser.OFPActionPushVlan(),
               ofp_parser.OFPActionSetField(vlan_vid=vlan),
               ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)

def mod_vlan(datapath, priority, in_port, ip_dst, dl_dst, vlan, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch(in_port=in_port, ipv4_dst=ip_dst, eth_type=0x0800)
    actions = [ofp_parser.OFPActionSetField(eth_dst=dl_dst),
               #ofp_parser.OFPActionPushVlan(),
               ofp_parser.OFPActionSetField(vlan_vid=vlan),
               ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)

def mod_eth_src_vlan(datapath, priority, in_port, dl_src, vlan, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800)
    actions = [ofp_parser.OFPActionSetField(eth_src=dl_src),
               #ofp_parser.OFPActionPushVlan(),
               ofp_parser.OFPActionSetField(vlan_vid=vlan),
               ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)


def mod_ip_dst(datapath, in_port,ip_dst, dl_dst, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    priority = 32
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800)
    actions = [ofp_parser.OFPActionSetField(eth_dst=dl_dst),
               ofp_parser.OFPActionSetField(ipv4_dst=ip_dst),
               ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)

def mod_eth_src(datapath, in_port, dl_src, ipdst, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    priority = 32
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800,
                                ipv4_dst=ipdst)
    actions = [ofp_parser.OFPActionSetField(eth_src=dl_src),
                ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)


def mod_eth_src_dst(datapath, in_port, dl_src, dl_dst, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    priority = 32
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800)
    actions = [ofp_parser.OFPActionSetField(eth_dst=dl_dst),
               ofp_parser.OFPActionSetField(eth_src=dl_src),
               ofp_parser.OFPActionPopVlan(),
               ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)


def mod_eth_src_dst_ext(datapath, in_port, ip_src, dl_src, dl_dst, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    priority = 32
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800, ipv4_src=ip_src)
    actions = [ofp_parser.OFPActionSetField(eth_dst=dl_dst),
               ofp_parser.OFPActionSetField(eth_src=dl_src),
               ofp_parser.OFPActionPopVlan(),
               ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)


def send_normal_flow(datapath, in_port, ipdst, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    priority = 32
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800, 
                                ip_proto=0x11, ipv4_dst=ipdst)
    actions = [ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)


def send_backbone_flow(datapath, priority, in_port, ipdst, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch(in_port=in_port, eth_type=0x0800, 
                                ipv4_dst=ipdst)
    actions = [ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)


def send_mac_flow(datapath, dl_dst, out_port):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    priority = 16
    match = ofp_parser.OFPMatch(
                                eth_dst=dl_dst)
    actions = [ofp_parser.OFPActionOutput(out_port)]
    send_msg(datapath, priority, match, actions)


def send_flow(datapath, proto, priority, nw_dst, outports):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch(eth_type=proto,ipv4_dst=nw_dst)
    actions = []
    for p in outports:
        actions.append(ofp_parser.OFPActionOutput(p))
    send_msg(datapath, priority, match, actions)


def ip_src_dst(datapath, proto, priority, nw_src, nw_dst, outports):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch(eth_type=proto,ipv4_src= nw_src,
                                    ipv4_dst=nw_dst)
    actions = []
    for p in outports:
        actions.append(ofp_parser.OFPActionOutput(p))
    send_msg(datapath, priority, match, actions)


def ip_src_dst_popvlan(datapath, proto, priority, nw_src, nw_dst, outport):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch(eth_type=proto,ipv4_src= nw_src,
                                    ipv4_dst=nw_dst)
    actions = []
    actions.append(ofp_parser.OFPActionPopVlan())
    actions.append(ofp_parser.OFPActionOutput(outport))
    send_msg(datapath, priority, match, actions)

def send_arp_flow(datapath, priority, nw_dst, outports):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch(eth_type=0x0806, arp_tpa=nw_dst)
    actions = []
    for p in outports:
        actions.append(ofp_parser.OFPActionOutput(p))
    send_msg(datapath, priority, match, actions)

def del_all_flows(datapath):
    ofp = datapath.ofproto
    ofp_parser = datapath.ofproto_parser
    match = ofp_parser.OFPMatch()
    req = ofp_parser.OFPFlowMod(datapath=datapath, command=ofp.OFPFC_DELETE,
                                out_port=ofp.OFPP_ANY,
                                out_group=ofp.OFPG_ANY,
                                match=match)
    datapath.send_msg(req)

