#!/usr/bin/python
'''
    udp socket client
'''

 
import socket, sys, optparse, time, select, cStringIO
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
    parser.add_option('--ps', help=help, type='int', default=4*1024)

    help = "time interval"
    parser.add_option('--time', help=help, type='int', default=10)

    options, args = parser.parse_args()
  
    if len(args) != 1:
        parser.error('Provide server address')
  
    s_addr = args[0]
  
    return options, s_addr

def main():
    options, host = parse_args()
    data = cStringIO.StringIO()
    # create dgram udp socket
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.setblocking(False)
    except socket.error:
        print 'Failed to create socket'
        sys.exit()
     
    total = 0
    Bpms = (options.bw * 1024)  * 128.0 / 10**2  #(Kbps -> B/10ms)
    cid = options.id
    start = datetime.now()
    wait = 0
    pad = '*' * (options.ps - 26) # 64 - len(msg)
    for i in range(1,1000000):
        t = datetime.now()
        tstamp =  '%s' % t.time()
        msg = "%08d,%s,%s" % (i, cid, tstamp+pad)
        try:
            #hosts = ['192.168.3.91', '192.168.4.91',  '192.168.6.91']
            ready = select.select([], [s], [], 5)
            if ready[1]:
                #for host in hosts:
                sent = s.sendto(msg, (host, options.port))
                data.write(msg[:26] + '\n')
                total += sent
                current = datetime.now() 
                diff = current - start
                du = total_seconds(diff)
                if du > 0:
                    rate = total / (du * 100)
                else:
                    rate = 1
                if rate > Bpms:
                  wait = rate / Bpms / 1000
                  time.sleep(wait)
                
                if du >= options.time:
                    print "wait for %f s." % wait
                    print "last msg: %s" % msg[:10]
                    print "sent total: %d, packet-size: %d, duration: %3.2f" % (total, sent, du)
                    print "bandwidth: %3.2f Mbps" % (total / du * 8 / 10**6)
                    break
            else:
              print "socket is busy"
        except socket.error, msg:
            print 'Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
            sys.exit()

    with open('client' + str(cid) + '.csv', 'w+') as f:
      f.write(data.getvalue())

    s.close()
    sys.exit()  

def total_seconds(td):
    return (td.microseconds + (td.seconds + td.days * 24 * 3600) * 10**6) / 10**6

if __name__ == '__main__':
    main()
