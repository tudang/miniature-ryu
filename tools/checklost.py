#!/usr/bin/python
import os
import sys

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

ham_dist = {}
for k,v in clients.items():
  print '-' * 40
  print 'check client %s' % k
  new_seqs = count_lost_and_unorder(v)
  ham_dist[k] = new_seqs
  #print ham_dist[k]
  path, filename = os.path.split(sys.argv[1])
  with open(path + "/fix-" + filename, "a") as f:
    for seq in new_seqs:
      f.write("%s\n" % seq)
