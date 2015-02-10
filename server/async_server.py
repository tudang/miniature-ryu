import asyncore, socket

class AsyncoreServerUDP(asyncore.dispatcher):
   def __init__(self):
      asyncore.dispatcher.__init__(self)

      # Bind to port 5005 on all interfaces
      self.create_socket(socket.AF_INET, socket.SOCK_DGRAM)
      self.bind(('', 5005))

   # Even though UDP is connectionless this is called when it binds to a port
   def handle_connect(self):
      print "Server Started..."

   # This is called everytime there is something to read
   def handle_read(self):
      data, addr = self.recvfrom(2048)
      ip,port = addr
      print str(addr)+" >> "+data
      with open(str(port) + ".txt", "a") as myfile:
         myfile.write(data + "\n")

   # This is called all the time and causes errors if you leave it out.
   def handle_write(self):
      pass

AsyncoreServerUDP()
asyncore.loop()
