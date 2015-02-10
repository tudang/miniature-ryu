#!/usr/bin/python

import socket
import asyncore 
import random 
import sys
import os
import string
import signal
import time

N=50000
t=20

def break_handler(signal, frame):
  print 'Terminated'
  sys.exit(0)

class AsyncoreClientUDP(asyncore.dispatcher):

   def __init__(self, server, port):
      self.server = server
      self.port = port
      self.buffer = ""
      signal.signal(signal.SIGINT, break_handler)

      # Network Connection Magic!
      asyncore.dispatcher.__init__(self)
      self.create_socket(socket.AF_INET, socket.SOCK_DGRAM)
      self.bind( ('', 0) ) # bind to all interfaces and a "random" free port.
      print "Connecting..."

   # Once a "connection" is made do this stuff.
   def handle_connect(self):
      print "Connected"
   
   # If a "connection" is closed do this stuff.
   def handle_close(self):
      self.close()

   # If a message has arrived, process it.
   def handle_read(self):
      data, addr = self.recv(2048)
      print data

   # Actually sends the message if there was something in the buffer.
   def handle_write(self):
      if self.buffer != "":
         #print self.buffer
         sent = self.sendto(self.buffer, (self.server, self.port))
         self.buffer = self.buffer[sent:]


def id_generator(size=6, chars=string.ascii_uppercase + string.digits):
   return ''.join(random.choice(chars) for _ in range(size))

connection = AsyncoreClientUDP(sys.argv[1], 5005) # create the "connection"
x = id_generator(int(sys.argv[2])-44)
start_time = time.time()
print 'start time: %s' %time.ctime(start_time)
i = 0
while((time.time() - start_time) < t):
   asyncore.loop(count = 10) # Check for upto 10 packets this call?
   seq = '%05d' % i
   data = seq + "--" +  x
   #print sys.getsizeof(data)
   connection.buffer += data 
   with open(sys.argv[3], "a") as cdata:
       cdata.write(seq + "\n")
   i = i + 1
   
