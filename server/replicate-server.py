#!/usr/bin/python
'''
    UDP Socket Server Binding to a specific interface
    Tu Dang (huynh.tu.dang@usi.ch)
'''
 
import socket, select
import optparse, sys, signal
import cStringIO, string
import logging
import Queue

from datetime import datetime

class UDPServer(object):

    def break_handler(self, signal, frame):
        try:
            print "Terminate server"
            with open(self.output, 'w+') as f:
                f.write(self.oc.getvalue())
            print "write to file %s" % self.output
            td = self.end - self.start
            bw = self.total * 8 / total_seconds(td) / 2**20
            lost = self.count_lost()
            print 'lost %f' % lost   
            with open("record.txt", 'a+') as lf:
                lf.write(self.iface + ',' + str(bw) + ',' + str(lost) + '\n')
            print 'received: ', self.total
            print 'total seconds %f' % total_seconds(td)
            print 'bandwidth %3.2f Mbps' % bw
        except ZeroDivisionError:
            print "Do not received anything"
        finally:
            print "Cleanup"
            self.sock.close()
            self.oc.close()
            sys.exit()


    def close(self):
        print "Close server"
        with open(self.output, 'w+') as f:
            f.write(self.oc.getvalue())
        print "write to file %s" % self.output
        td = self.end - self.start
        bw = self.total * 8 / total_seconds(td) / 2**20
        print 'received: ', self.total
        print 'total seconds %f' % total_seconds(td)
        print 'bandwidth %3.2f Mbps' % bw
        lost = self.count_lost()
        print 'lost %f' % lost   
        with open("record.txt", 'a+') as lf:
            lf.write(self.iface + ',' + str(bw) + ',' + str(lost) + '\n')
        self.sock.close()
        self.oc.close()
        sys.exit()

    def count_lost(self):
        current = 1
        lost = 0
        lines = string.split(self.oc.getvalue(), '\n')
        for line in lines:
            try:
                seq = int(line[:8])
                if seq != current:
                    #print seq, current
                    lost += seq - current
                current = seq + 1
            except ValueError:
                current += 1
        print lost
        return float(lost) / current

    def __init__(self, iface, port,  output):
        self.iface = iface
        self.port = port
        self.output = output
        signal.signal(signal.SIGINT, self.break_handler)
        self.sock = self.create_server_socket()
        self.oc = cStringIO.StringIO()
        self.start = datetime.now()
        self.end = self.start
        self.total = 0
        self.sendsock = self.create_socket()
        self.sendsock.setblocking(False)
        self.q = Queue.Queue()

    def create_socket(self):
        HOST = ''   # Symbolic name meaning all available interfaces
        try :
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            #s.setsockopt(socket.SOL_SOCKET, socket.SO_BINDTODEVICE, self.iface + "\0")
            print 'Socket created'
            return s
        except socket.error, msg :
            print 'Failed to create socket. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
            sys.exit()

    def create_server_socket(self):
        HOST = ''   # Symbolic name meaning all available interfaces
        s = self.create_socket()
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # Bind socket to local host and port
        try:
            s.bind((HOST, self.port))
        except socket.error , msg:
            print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
            sys.exit()
        print 'Socket bind complete'
        return s


    def spread_messages(self):
        hosts = ['192.168.3.91', '192.168.4.91','192.168.5.91','192.168.6.91']
        try:
            if not self.q.empty():
                data = self.q.get_nowait()
                #print "[sending]" + data[:10]
                for host in hosts: 
                    sent = self.sendsock.sendto(data, (host, self.port))
        except socket.error, msg:
            print 'Error Code : ' + str(msg[0]) + ' Message ' + msg[1] 
         

    def read_socket(self, BUFSIZE):
        data, addr = self.sock.recvfrom(BUFSIZE)
        #print "Received from %s" % addr[0]
        if not data: 
            raise IOError
        self.end = datetime.now()
        self.q.put(data)
        self.total += len(data)
        self.oc.write(data.strip()[:10] + '\n')
        return len(data)
        #print 'Message[' + addr[0] + ':' + str(addr[1]) + '] - ' + data.strip()[:10]

def parse_args():
    usage = """
            usage: %prog [options]
            run it like this:
            sudo python udpserver.py eth1
            """

    parser = optparse.OptionParser(usage)
    help = "The port to listen on. Default to 8888."
    parser.add_option('--port', type='int', help=help, default=8888)

    help = "buffer size. Default 4kB"
    parser.add_option('--bufsize', type='int', help=help, default=4096)

    help = "The output filename. Default is output.txt"
    parser.add_option('--output', help=help, default='output.txt')

    options, args = parser.parse_args()

    if len(args) != 1:
      parser.error("Provide the binding interface")

    iface = args[0]

    return options, iface


def main():
    options, iface = parse_args()
    BUFSIZE = options.bufsize
    udpserver = UDPServer(iface, options.port, options.output)
    #now keep talking with the client
    data, addr = udpserver.sock.recvfrom(BUFSIZE)
    if data:
        udpserver.q.put(data)
    udpserver.start = datetime.now()
    udpserver.total += len(data)
    udpserver.oc.write(data.strip()[:10] + '\n')
    print "Received from %s" % addr[0]
    udpserver.sock.setblocking(False)
    while 1:
        ready = select.select([udpserver.sock], [udpserver.sendsock], [], 5)
        if ready[0]:
            udpserver.read_socket(BUFSIZE)
        elif ready[1]:
            udpserver.spread_messages()

    udpserver.close()


def total_seconds(td):
    return (td.microseconds + (td.seconds + td.days * 24 * 3600) * 10**6) / 10**6


if __name__ == '__main__':
    main()
