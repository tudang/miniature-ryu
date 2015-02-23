#!/usr/bin/python
'''
    udp socket client
'''

 
import socket, sys, optparse, time
from datetime import datetime
 
def parse_args():
    usage = '''usage: %prog [options] server-address'''
    
    parser = optparse.OptionParser(usage)

    help = "Client ID"
    parser.add_option('--id', help=help, default='1')
  
    help = "The server's listening port"
    parser.add_option('--port', help=help, type='int', default=8888)
  
    help = "The sending throughput"
    parser.add_option('--bw', help=help, type='int', default=1)

    help = "packet size"
    parser.add_option('--ps', help=help, type='int', default=16*1024)

    help = "time interval"
    parser.add_option('--time', help=help, type='int', default=10)

    options, args = parser.parse_args()
  
    if len(args) != 1:
        parser.error('Provide server address')
  
    s_addr = args[0]
  
    return options, s_addr

def main():
    options, host = parse_args()
    # create dgram udp socket
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    except socket.error:
        print 'Failed to create socket'
        sys.exit()
     
    total = 0
    Bpms = options.bw * 128.0 / 10**2  #(Kbps -> B/10ms)
    cid = options.id
    start = datetime.now()
    wait = 0
    pad = '*' * (options.ps - 10) # 64 - len(msg)
    for i in range(0,1000000):
        msg = "%08d,%s" % (i, cid+pad)
        try:
            sent = s.sendto(msg, (host, options.port))
            total += sent
            current = datetime.now() 
            diff = current - start
            du = diff.total_seconds()
            rate = total / (du * 100)
            if rate > Bpms:
              wait = rate / Bpms / 1000
              time.sleep(wait)
            
            if du >= options.time:
                print "wait for %f s." % wait
                print "last msg: %s" % msg[:10]
                print "packet-size: %d, duration: %3.2f" % (sent, du)
                print "bandwidth: %3.2f Mbps" % (total / du * 8 / 2**20)
                s.close()
                sys.exit()  
        except socket.error, msg:
            print 'Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
            sys.exit()

if __name__ == '__main__':
    main()
