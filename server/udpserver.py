#!/usr/bin/python
'''
    UDP Socket Server Binding to a specific interface
    Tu Dang (huynh.tu.dang@usi.ch)
'''
 
import socket, optparse, sys, signal, cStringIO

class UDPServer(object):

    def break_handler(self, signal, frame):
        print "Terminate server"
        with open(self.output, 'w+') as f:
            f.write(self.oc.getvalue())
        print "write to file %s" % self.output
        self.oc.close()
        sys.exit()

    def __init__(self, iface, port,  output):
        self.iface = iface
        self.port = port
        self.output = output
        signal.signal(signal.SIGINT, self.break_handler)
        self.sock = self.create_udp_socket()
        self.oc = cStringIO.StringIO()

    def create_udp_socket(self):
        HOST = ''   # Symbolic name meaning all available interfaces
        try :
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_BINDTODEVICE, self.iface + "\0")
            print 'Socket created'
        except socket.error, msg :
            print 'Failed to create socket. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
            sys.exit()
         
         
        # Bind socket to local host and port
        try:
            s.bind((HOST, self.port))
        except socket.error , msg:
            print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
            sys.exit()
             
        print 'Socket bind complete'
        return s


def parse_args():
    usage = """usage: %prog [options]

run it like this:
sudo python udpserver.py eth1

"""
    parser = optparse.OptionParser(usage)
    help = "The port to listen on. Default to 8888."
    parser.add_option('--port', type='int', help=help, default=8888)

    help = "The output filename. Default is output.txt"
    parser.add_option('--output', help=help, default='output.txt')

    options, args = parser.parse_args()

    if len(args) != 1:
      parser.error("Provide the binding interface")

    iface = args[0]

    return options, iface


def main():
    options, iface = parse_args()
    udpserver = UDPServer(iface, options.port, options.output)
    #now keep talking with the client
    while 1:
        # receive data from client (data, addr)
        data, addr = udpserver.sock.recvfrom(4096)
        if not data: 
            break
        udpserver.oc.write(data.strip()[:10] + '\n')
        #print 'Message[' + addr[0] + ':' + str(addr[1]) + '] - ' + data.strip()[:10]

    udpserver.sock.close()

if __name__ == '__main__':
    main()
