#!/usr/bin/python

import socket
import asyncore 
import random 
import sys
import os
import string
import signal
import time

N=10000
t=10


class AsyncoreClientUDP(asyncore.dispatcher):
   
  def break_handler(self, signal, frame):
    print 'Terminated'
    self.handle_close()
    sys.exit(0)
    
  def __init__(self, server, port):
    self.server = server
    self.port = port
    self.buffer = ""
    self.total_sent = 0
    signal.signal(signal.SIGINT, self.break_handler)

    # Network Connection Magic!
    asyncore.dispatcher.__init__(self)
    self.create_socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.bind( ('', 0) ) # bind to all interfaces and a "random" free port.
    print "Connecting..."
    self.connect((server,port))

   # Once a "connection" is made do this stuff.
  def handle_connect(self):
    print "Connected"
 
   # If a "connection" is closed do this stuff.
  def handle_close(self):
    print "Closed"
    #self.close()

   # If a message has arrived, process it.
  def handle_read(self):
    data, addr = self.recv(2048)
    print data

   # Actually sends the message if there was something in the buffer.
  def handle_write(self):
    if self.buffer != "":
      sent = self.sendto(self.buffer, (self.server, self.port))
      self.total_sent = self.total_sent + sent
      self.buffer = self.buffer[sent:]


def id_generator(size=6, chars=string.ascii_uppercase + string.digits):
   return ''.join(random.choice(chars) for _ in range(size))

connection = AsyncoreClientUDP(sys.argv[2], 5005) 
sizekb = int(sys.argv[3]) * 1024
x = id_generator(sizekb - 11)
start_time = time.time()
print 'start time: %s' %time.ctime(start_time)
i = 0
total = 0
client_id = sys.argv[1]
seqs = []
while((time.time() - start_time) < t):
   asyncore.loop(count = 10) # Check for upto 10 packets this call?
   seq = '%08d' % i
   data = seq + "," + client_id + "," +  x
   total = total + sys.getsizeof(data)
   connection.buffer += data 
   seqs.append(seq)
   i = i + 1

# Flush the write buffer
while(connection.buffer != ''):
  connection.handle_write()

# Calculate the sending rate of client
end_time = time.time() 
rate = total * 8 / (end_time - start_time) / 2**20
print 'end time: %s' %time.ctime(end_time)
print 'average sending rate: %4.2f Mbits/second' % rate 
print 'number of messages: %d' % len(seqs)
with open(sys.argv[4], "w+") as cdata:
   for s in seqs:     
      cdata.write(s + "\n")
