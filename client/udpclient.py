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
  
    help = "The sending speed"
    parser.add_option('--ss', help=help, type='int', default=50)

    help = "packet size"
    parser.add_option('--ps', help=help, type='int', default=1470)

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
     
    total = 0L
    cid = options.id
    start = datetime.now()
    wait = 0
    pad = '*' * (options.ps - 26) # 64 - len(msg)
    N = options.ss
    npacket = 10**5 / N
    delta = 30
    count = 0
    begin = datetime.now()
    while (True):
        #for j in range(1,N):
        t = datetime.now()
        dur = total_seconds(t - begin)
        if (dur >= delta): 
            break
        tstamp =  '%s' % t.time()
        msg = "%08d,%s,%s" % (count, cid, tstamp+pad)
        count += 1
        try:
            ready = select.select([], [s], [], 0.001)
            if ready[1]:
                sent = s.sendto(msg, (host, options.port))
                data.write(msg[:26] + '\n')
                total += sent
            else:
                print "socket is busy"
        except socket.error, msg:
            print 'Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
            sys.exit()
        time.sleep(0.001)
    du = total_seconds(datetime.now() - start)
    print "sent total: %d,  duration: %3.2f" % (total, du)
    print "bandwidth: %3.2f Mbps" % (total / du * 8 / 10**6)

    with open('client' + str(cid) + '.csv', 'w+') as f:
      f.write(data.getvalue())

    s.close()
    sys.exit()  

def total_seconds(td):
    return (td.microseconds + (td.seconds + td.days * 24 * 3600) * 10**6) / 10**6

if __name__ == '__main__':
    main()
