#!/usr/bin/python
'''
    udp socket client
    Silver Moon
'''

 
import socket   #for sockets
import sys  #for exit
 
# create dgram udp socket
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
except socket.error:
    print 'Failed to create socket'
    sys.exit()
 
host = sys.argv[1]
port = 8888;
x = int(sys.argv[2])
N = x * 1**10
cid='1'
for i in range(1,N):
    msg = str(i) + "," + cid
     
    try :
        #Set the whole string
        s.sendto(msg, (host, port))
         
        # receive data from client (data, addr)
        #d = s.recvfrom(1024)
        #reply = d[0]
        #addr = d[1]
         
        #print 'Server reply : ' + reply
     
    except socket.error, msg:
        print 'Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()
