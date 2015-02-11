#!/usr/bin/python
import sys

class LostMessage:

  def __init__(self, args):
    self.files = args
    self.clients = {}
    self.losts = {}
  
  def intersect(self, a, b):
    """ return the intersection of two lists """
    return (set(a) & set(b))

  def get_lost_seqs(self, seqs):
    count = 0
    unorder = 0
    losts = []
    out_of_orders = []
    final = max(seqs)
    last = 0
    for i in range(0,final+1):
      if ( i not in seqs):
        count = count + 1
        losts.append(i)
        seqs.insert(i,-1)
        continue
      if seqs[i] < last:
        unorder = unorder + 1
        out_of_orders.append(seqs[i])
      else:
        last = seqs[i]  
    
    p = 100 * count / final 
    print 'lost %d over %d messages, %2.1f%%' % (count, final,  p)
    if losts:
       print losts[0:10]
    print 'out of order: %d' % unorder
    if out_of_orders:
      print out_of_orders[0:10]
    return losts

  def parse_data(self, filename):
    with open(filename) as f:
      content = f.readlines()

    clients = {}
    for line in content:
      num = int(line[0:8])
      clid = line[9]
      seqs = clients.setdefault(clid, [])
      seqs.append(num)
    return clients

if __name__=='__main__':
  ob = LostMessage(sys.argv[1:])
  for afile in ob.files:
    data = ob.clients.setdefault(afile, {})
    ob.clients[afile]  = ob.parse_data(afile)
    ob.losts.setdefault(afile, {})
 
  for afile,adict in ob.clients.items():
    print 'process file %s' % afile
    for k,v in adict.items():
      print '-' * 40
      print 'check client %s' % k
      ob.losts[afile].setdefault(k, [])
      ob.losts[afile][k] = ob.get_lost_seqs(v)

  common = ob.intersect(ob.losts[ob.files[0]]['1'],ob.losts[ob.files[1]]['1'])
  print common
