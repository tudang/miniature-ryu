#!/usr/bin/env python
import argparse
import subprocess
import shlex
import os
from threading import Timer
import time
import logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def replica(host, path, duration, num_instances, eth, output):
    logger.info("start replica %s" % host)
    cmd = "ssh %s %s/replica -s -t %d -n %d %s" % (host, path, duration, num_instances, eth)
    with open("%s/%s.dat" % (output, host), 'w+') as out: 
        proc = subprocess.Popen(shlex.split(cmd),
                stdout=out,
                stderr=out,
                shell=False)
    return proc


def client(host, path, duration, num_instances, eth):
    logger.info("start client %s" % host)
    cmd = "ssh %s %s/replica -c -t %d -n %d %s" % (host, path, duration, num_instances, eth)
    proc = subprocess.Popen(shlex.split(cmd), shell=False)
    return proc

def send_term_signal(host):
    logger.debug("end %s" % host)
    cmd = "ssh %s pkill -TERM replica" % host
    proc = subprocess.Popen(shlex.split(cmd),
                        shell=False)
    proc.wait()

def kill_all(*hosts):
    for h in hosts:
        send_term_signal(h)

def main():
    parser = argparse.ArgumentParser(description='Run NetPaxos experiment.')
    parser.add_argument('output', help='output directory')
    parser.add_argument('--time', type=int, default=10, help='amout of time in second to run example')
    parser.add_argument('--instance', type=int, default=1000, help='amout of instance to run example')
    parser.add_argument('--verbose', default=False, action='store_true', help='verbose flag')
    args = parser.parse_args()

    if not os.path.exists(args.output):
        os.makedirs(args.output)

    path = os.getcwd() + "/build"
    args.eth = "enp1s0"

    servers = ["node84", "node85", "node86", "node87"]
    clients = ["node81", "node82"]
    # start replicas
    for host in servers:
        replica(host, path, args.time, args.instance, args.eth, args.output)

    time.sleep(5)

    # start client
    for idx in range(10):
        for cli in clients:
            client(cli, path, args.time, args.instance, args.eth)

    
    # kill replicas and client after (args.time) seconds
    t= Timer(args.time, kill_all, clients)
    t.start()
    s= Timer(args.time, kill_all, servers)
    s.start()
   

if __name__ == "__main__":
    main()
