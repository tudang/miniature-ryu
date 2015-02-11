#!/usr/bin/python

import sys

with open(sys.argv[1]) as f:
  content = f.readlines()

seqs = []
for l in content:
  seqs.append(int(l))

count = 0
unorder = 0
lost = []
out_of_order = []
final = max(seqs)
last = 0
for i in range(0,final+1):
  if ( i not in seqs):
    count = count + 1
    lost.append(i)
    seqs.insert(i,-1)
    continue
  if seqs[i] < last:
    unorder = unorder + 1
    out_of_order.append(seqs[i])
  else:
    last = seqs[i]  

p = 100 * count / final 
print 'lost %d messages, %d%%' % (count, p)
print lost[0:10]
print 'out of order: %d' % unorder
print out_of_order[0:10]
