#!/usr/bin/python
'''
    UDP Socket Server Binding to a specific interface
    Tu Dang (huynh.tu.dang@usi.ch)
'''
import threading
import time
import socket
import sys
import optparse
import select
import sys
import signal
import Queue
import logging
from datetime import datetime
logging.basicConfig(filename='/tmp/netpaxos.log',level=logging.DEBUG)
exitFlag = 0
HOST = ''   # Symbolic name meaning all available interfaces
PORT = 8888
BUFSIZE = 65536

class myThread (threading.Thread):
    def __init__(self, threadID, name, dev):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.dev = dev

    def run(self):
        exitFlag = 0
        logging.info("Starting " + self.name)
        sock = self.create_udp_socket()
        recv_total = 0
        q  = Queue.Queue()
        data, addr = sock.recvfrom(BUFSIZE)
        print '[%s] %s connected' % (self.dev, addr)
        start = datetime.now()
        if not data:
            raise IOError
        recv_total += len(data)
        q.put(data)
        sock.setblocking(False)
        while True:
            if exitFlag:
                break
            ready = select.select([sock], [], [], 1)
            if ready[0]:
                data, addr = sock.recvfrom(BUFSIZE)
                if not data:
                    raise IOError
                end = datetime.now()
                recv_total += len(data)
                q.put(data)
                #print '[%s] - %s' % (self.dev, data[:10])
            else:
                logging.info(self.name + " Receive no data after 1 seconds." )
                du = total_seconds(end - start)
                logging.info('%s: %3.2f Mbps' % (self.dev, (recv_total * 8 / du / 2**20)))
                exitFlag = True
        with open("/tmp/" + self.dev, "w+") as f:  
            while not q.empty():
                data = q.get()
                f.write('%s\n' % data[:10]) 
        logging.debug("Exiting " + self.name)

    def create_udp_socket(self):
        try :
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_BINDTODEVICE, self.dev)
            logging.debug('Socket created')
        except socket.error, msg :
            logging.error('Failed to create socket. Error Code : ' + str(msg[0]) + \
               ' Message ' + msg[1])
            sys.exit()
        # Bind socket to local host and port
        try:
            s.bind((HOST, PORT))
        except socket.error , msg:
            logging.error('Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
            sys.exit()
        logging.debug('Socket bind complete')
        return s


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

    if len(args) < 1:
      parser.error("Provide the binding interface")

    ifaces = args

    return options, ifaces

def break_handler(signal, frame):
    global exitFlag
    exitFlag = True
    print "Terminate Server"
    sys.exit()

def main():
    signal.signal(signal.SIGINT, break_handler)
    # Create new threads
    global PORT, BUFSIZE
    options, ifaces = parse_args()
    PORT = options.port
    BUFSIZE = options.bufsize
    
    threads = []
    for i,itf in enumerate(ifaces):
        t = myThread(i, "Thread-" + str(i), itf)
        threads.append(t)
        t.start()

    for t in threads:
        t.join()
    print "Exiting Main Thread"

def total_seconds(td):
    return (td.microseconds + (td.seconds + td.days * 24 * 3600) * 10**6) / 10**6

if __name__ == '__main__':
    main()
