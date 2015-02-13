#!/usr/bin/python

import sys

contents = []

def hamming_distance(s1, s2):
    """Return the Hamming distance between (un)equal-length sequences"""
    #if len(s1) != len(s2):
    #    raise ValueError("Undefined for sequences of unequal length")
    sum = 0
    line = 1
    for ch1, ch2 in zip(s1, s2):
      if ch1 != ch2:
        print line
      sum = sum + (ch1 != ch2)
      line += 1
    return sum
    #return sum(ch1 != ch2 for ch1, ch2 in zip(s1, s2))

for arg in sys.argv[1:]:
  with open(arg) as f:
    content = f.readlines()
    contents.append(content)
    

s0 = contents[0]
print 'len of contents %d' % len(contents)
print type(s0).__name__
print 'len of s0 %d' % len(s0)
for s in contents[1:]:
  print 'len of s %d' % len(s)
  d = hamming_distance(s0, s)
  print 'distance %d' % d
  s0 = s
