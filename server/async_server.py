#!/usr/bin/python

import asyncore, socket, sys, time, signal

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
     for f in self.fd:
        f.close()
     sys.exit(0)

   def __init__(self):
      asyncore.dispatcher.__init__(self)
      signal.signal(signal.SIGINT, self.break_handler)
      self.fd = []
      for fn in sys.argv:
        self.fd.append( open( fn + ".txt", "a"))

      # Bind to port %PORT on all interfaces
      self.create_socket(socket.AF_INET, socket.SOCK_DGRAM)
      self.bind(('', PORT))

   # Even though UDP is connectionless this is called when it binds to a port
   def handle_connect(self):
      print "Server Started..."

   # This is called everytime there is something to read
   def handle_read(self):
      data, addr = self.recvfrom(BUF_SIZE)
      ip,port = addr
      #print str(addr)+" >> "+data
      #print sys.getsizeof(data)
      size = len(self.fd)
      self.fd[port%size].write(data[0:5] + "\n")
    
   def handle_close(self):
      print 'close server'
      self.close()

   # This is called all the time and causes errors if you leave it out.
   def handle_write(self):
      pass

AsyncoreServerUDP()
asyncore.loop()
