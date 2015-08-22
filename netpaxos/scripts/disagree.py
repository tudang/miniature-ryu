#!/usr/bin/env python

import argparse
import subprocess
import sys
import os
import time

def difference(fname1, fname2):
    print '{0} vs {1}'.format(fname1, fname2)
    disagrement = 0
    indecision = 0
    with open(fname1, 'r') as file1, open(fname2, 'r') as file2:
        for (x,y) in zip(file1, file2):
            x = x.strip()
            y = y.strip()
            if x == y and x != 'Indecision':
                continue
            elif x != y and y != 'Indecision' and x != 'Indecision':
                disagrement += 1     
            else:
                indecision += 1
    print "disagrement: {0} indecision: {1}".format(disagrement, indecision)
    #bad_words = ['Indesicion', '\n']
    #for line in diff:
    #    if not any(bad_word in line for bad_word in bad_words ):
    #        disagreement.append(line)
    #    else:
    #        indecision.append(line)
    #print "{0} {1} {2} {3}\n".format(len(set1), len(set2), len(disagreement), len(indecision))

def main():
    parser = argparse.ArgumentParser(description='Run NetPaxos experiment.')
    parser.add_argument('input', default='output', help='output file')
    args = parser.parse_args()
    print "Finding different"
    #servers = ['node83', 'node84', 'node85', 'node86']
    #for i in range(len(servers) - 1):
    #    j = len(servers) - 1
    #    while j > i:
    #        f1 = args.input + "/" + servers[i] + ".dat"
    #        f2 = args.input + "/" + servers[j] + ".dat"
    #        different(f1, f2)
    #        j = j - 1
        
    f1 = args.input + "/" + "node90.dat"
    f2 = args.input + "/" + "node91.dat"
    difference(f1, f2)
    print "Finish."

if __name__ == "__main__":
    main()
