#!/usr/bin/env python

import argparse
import subprocess
import shlex
import sys
import os

def check_running(host):
    cmd = "ssh %s sudo pgrep netpaxos" % host
    proc = subprocess.Popen(shlex.split(cmd),
                        shell=False,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)

    out,err = proc.communicate()
    if out:
        print host
        print out
    if err:
        print host
        print err

    proc.wait()
    
def send_term_signal(host):
    print "end %s" % host
    cmd = "ssh %s sudo pkill -TERM netpaxos" % host
    proc = subprocess.Popen(shlex.split(cmd),
                        shell=False,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)

    out,err = proc.communicate()
    if out:
        print out
    if err:
        print err

    proc.wait()


def main():
    parser = argparse.ArgumentParser(description='Check Running NetPaxos.')
    parser.add_argument('-k', default=False, action='store_true', help='kill NetPaxos instances')
    args = parser.parse_args()

    servers = ["node81", "node82", "node83", "node84", "node85", "node86", "node90", "node91"]
    for s in servers:
        check_running(s)
        if args.k:
            send_term_signal(s)


if __name__ == "__main__":
    main()
