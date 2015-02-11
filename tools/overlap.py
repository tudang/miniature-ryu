#!/usr/bin/python

import sys

def parse_data(afile):
  seqs = []
  with open(afile) as f:
    content = f.readlines()
  
  for line in content:
    num = int(line)
    seqs.append(num)
  
  return seqs


def intersect(a, b):
  return (set(a) & set(b))

def union(a, b):
  return (set(a) | set(b))

list_losts = []
for fn in sys.argv[1:]:
  a = parse_data(fn)
  print '%s: %d' % (fn, len(a))
  list_losts.append(a)

tmp = list_losts[0]
uni = list_losts[0]

for el in list_losts:
  tmp = intersect(tmp, el)
  uni = union(uni, el)
print 'intersect of losts: %d' % len(tmp)
print 'union of losts: %d' % len(uni)
