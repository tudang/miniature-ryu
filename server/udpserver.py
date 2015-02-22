#!/usr/bin/python
'''
    UDP Socket Server Binding to a specific interface
    Tu Dang (huynh.tu.dang@usi.ch)
'''
 
import socket, optparse, sys

def parse_args():
    usage = """usage: %prog [options]

run it like this:
python udpserver.py eth0

"""
    parser = optparse.OptionParser(usage)
    help = "The port to listen on. Default to 8888."
    parser.add_option('--port', type='int', help=help)

    help = "The output filename. Default is output.txt"
    parser.add_option('--output', help=help, default='output.txt')

    options, args = parser.parse_args()

    if len(args) != 1:
      parser.error("Provide the binding interface")

    iface = args[0]

    return options, iface

def main():
    options, iface = parse_args()
    HOST = ''   # Symbolic name meaning all available interfaces
    PORT = 8888 # Arbitrary non-privileged port

    # Datagram (udp) socket
    try :
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_BINDTODEVICE,iface + "\0")
        print 'Socket created'
    except socket.error, msg :
        print 'Failed to create socket. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()
     
     
    # Bind socket to local host and port
    try:
        s.bind((HOST, options.port or 8888))
        out = open(options.output, 'w+')
    except socket.error , msg:
        print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()
         
    print 'Socket bind complete'
     
    #now keep talking with the client
    while 1:
        # receive data from client (data, addr)
        d = s.recvfrom(1024)
        data = d[0]
        addr = d[1]
         
        if not data: 
            break
         
        reply = 'OK...' + data
        out.write(data + '\n')
        #s.sendto(reply , addr)
        #print 'Message[' + addr[0] + ':' + str(addr[1]) + '] - ' + data.strip()

    out.close()
    s.close()

if __name__ == '__main__':
    main()
