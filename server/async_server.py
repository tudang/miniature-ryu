#!/usr/bin/python

import asyncore, socket, sys
import time, signal
import datetime

PORT = 5005
BUF_SIZE = 63*1024

time.time()
print "-" * 40
print "Server listening on UDP port %d" % PORT
print "Receive Buffer Size: %d" % BUF_SIZE
print "-" * 40



class AsyncoreServerUDP(asyncore.dispatcher):

   def break_handler(self, signal, frame):
      print 'Terminated'
      self.fd.write(self.data)
      self.fd.close()
      print 'duration %f' % self.duration
      print 'avg rcv rate: %4.2f Mbits/second' % (self.total_bytes * 8 / self.duration / 2**20)
      sys.exit(0)

   def __init__(self):
      asyncore.dispatcher.__init__(self)
      signal.signal(signal.SIGINT, self.break_handler)
      self.fd = open(sys.argv[1], "w+")
      self.total_bytes = 0.0
      self.read_1st = True
      self.start = datetime.datetime.now()
      self.duration = 0.0
      self.data = ""
      # Bind to port %PORT on all interfaces
      self.create_socket(socket.AF_INET, socket.SOCK_DGRAM)
      self.bind(('', PORT))

   # Even though UDP is connectionless this is called when it binds to a port
   def handle_connect(self):
      print "Server Started..."

   # This is called everytime there is something to read
   def handle_read(self):
      if self.read_1st:
        self.start = datetime.datetime.now()
        self.read_1st = False

      data, addr = self.recvfrom(BUF_SIZE)
      end = datetime.datetime.now()
      self.total_bytes = self.total_bytes + len(data)
      self.duration = (end - self.start).total_seconds()
      ip,port = addr
      #print 'received at rate %f' % rate
      #print 'duration %f' % self.duration
      #print str(addr)+" >> "+data
      #print sys.getsizeof(data)
      self.data += str(data[0:8] + "," + data[9]  + "\n")
    
   def handle_close(self):
      print 'close server'
      self.close()

   # This is called all the time and causes errors if you leave it out.
   def handle_write(self):
      pass

AsyncoreServerUDP()
asyncore.loop()
