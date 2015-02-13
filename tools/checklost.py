#!/usr/bin/python

import sys

def count_lost_and_unorder(seqs):
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
  print 'lost %d over %d messages, %2.1f%%' % (count, final,  p)
  if lost:
     print lost[0:10]
  print 'out of order: %d' % unorder
  if out_of_order:
    print out_of_order[0:10]
  return seqs


with open(sys.argv[1]) as f:
  content = f.readlines()

clients = {}
for l in content:
  num = int(l[0:8])
  clid = l[9]
  seqs = clients.setdefault(clid, [])
  seqs.append(num)

for k,v in clients.items():
  print '-' * 40
  print 'check client %s' % k
  new_seqs = count_lost_and_unorder(v)
  with open("processed-" + sys.argv[1], "a") as f:
    for seq in new_seqs:
      f.write("%s\n" % seq)
