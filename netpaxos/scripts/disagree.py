#!/usr/bin/env python

import argparse
import subprocess
import sys
import os
import time

def difference(fname1, fname2):
    print '{0} vs {1}'.format(fname1, fname2)
    disagreement = []
    left_indecision = []
    right_indecision = []
    both_indecision = 0
    count = 0
    with open(fname1, 'r') as file1, open(fname2, 'r') as file2:
        for (x,y) in zip(file1, file2):
            count += 1
            x = x.strip()
            y = y.strip()
            if x == y and (not 'Indecision' in x):
                continue
            elif x != y and (not 'Indecision' in x) and (not 'Indecision' in y):
                disagreement.append((x,y))
            elif x != y and (not 'Indecision' in x) and ('Indecision' in y):
                right_indecision.append((x,y))
            elif x != y and ('Indecision' in x) and (not 'Indecision' in y):
                left_indecision.append((x,y))
            else:
                both_indecision += 1

    print "disagreement: {0} indecision: {1} {2} {3}. Count {4}".format(len(disagreement), 
        len(left_indecision), len(right_indecision), both_indecision, count)
    return disagreement
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
    parser.add_argument('--verbose', default=False, action='store_true', help='output file')
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
    differences = difference(f1, f2)
    if (args.verbose):
        for (x,y) in differences:
            print x,y
    print "Finish."

if __name__ == "__main__":
    main()
