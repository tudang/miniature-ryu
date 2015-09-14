#!/usr/bin/env python
import argparse
import subprocess
import shlex
import os
from threading import Timer
import time

def replica(host, path, duration, num_instances, eth, output):
    cmd = "ssh %s %s/replica -s -t %d -n %d %s" % (host, path, duration, num_instances, eth)
    with open("%s/%s.dat" % (output, host), 'w+') as out: 
        proc = subprocess.Popen(shlex.split(cmd),
                stdout=out,
                stderr=out,
                shell=False)
    return proc


def client(path, rep_id, duration, num_instances, eth):
    cmd = "%s/replica -c -t %d -n %d %s" % (path, duration, num_instances, eth)
    proc = subprocess.Popen(shlex.split(cmd), shell=False)
    return proc

def send_term_signal(host):
    #logger.info("end %s" % host)
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
    pipes = []

    servers = ["node80", "node87"]
    # start replicas
    for host in servers:
        pipes.append(replica(host, path, args.time, args.instance, args.eth, args.output))

    time.sleep(3)

    # start client
    for idx in range(1):
        pipes.append(client(path, idx, args.time, args.instance, args.eth))

    
    # kill replicas and client after (args.time) seconds
    t= Timer(args.time, kill_all, servers)
    t.start()
   

if __name__ == "__main__":
    main()
