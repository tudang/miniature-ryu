#!/usr/bin/env python

import argparse
import subprocess
import sys
import shlex
import os
import time
from threading import Timer
import logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

import re
from collections import OrderedDict

import disagree

  
def client(host, args, client_id):
    cmd = "ssh {0} \"nohup {1}/netpaxos -c -t {2} -n {5} > {1}/{3}/{0}-{4}.dat 2>&1 &\"".format(host, 
                args.path, args.interval, args.output, client_id, int(args.instances/args.rate/2))
    logger.info(cmd)
    ssh = subprocess.Popen(shlex.split(cmd),
                        shell=False,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
    out,err = ssh.communicate()
    if out:
        logger.info(out)
    if err:
        logger.err(err)
    ssh.wait()



def server(host, args, interfaces):
    logger.info("start %s" % host)
    itfs = " ".join(interfaces)
    cmd = "ssh %s \"cd miniature-ryu/netpaxos; sudo ./netpaxos -s %s\"" % (host, itfs)
    with open("%s/%s.dat" % (args.output, host), "w+") as out:
        with open("%s/%s.err" % (args.output, host), "w+") as err:
            proc = subprocess.Popen(shlex.split(cmd),
                        shell=False,
                        stdout=out,
                        stderr=err)
    return proc




def main():
    parser = argparse.ArgumentParser(description='Run NetPaxos experiment.')
    parser.add_argument('instances', type=int, help='number of instances')
    parser.add_argument('interval', type=int, help='time to run the experiment')
    parser.add_argument('rate', type=int, help='number of packets sending in one interval')
    parser.add_argument('output', help='output file')
    parser.add_argument('--sync', dest='sync', default=False, action='store_true', help='sync flag')    
    args = parser.parse_args()

    logger.info('instances    = %d' % args.instances)
    logger.info('interval     = %d' % args.interval)
    logger.info('rate         = %d' % args.rate)
    logger.info('output       = %s' % args.output)
    logger.info('sync         = %r' % args.sync)

    if not os.path.exists(args.output):
        os.makedirs(args.output)

    if args.sync:
        servers = ["node81", "node82", "node90", "node91"]
        logger.info('Sync proposers and learners clock')
        for s in servers:
            sync_clock(s)

    args.path = "/home/danghu/miniature-ryu/netpaxos"

    pipes = [] 
    pipes.append(server("node90", args, ['eth0.5', 'eth1.6', 'eth2.7', 'eth3.8']))
    pipes.append(server("node91", args, ['eth0.9', 'eth1.2', 'eth2.3', 'eth3.4']))
    time.sleep(1)
    for i in range(args.rate):
        client("node81", args, i)
        client("node82", args, i)

    for p in pipes:
        p.wait()

    disagree.difference(args.output + "/node90.dat", args.output + "/node91.dat")

if __name__ == "__main__":
    main()
