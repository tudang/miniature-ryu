#!/usr/bin/python
'''
    udp socket client
'''

 
import socket   
import sys, optparse 
 
def parse_args():
    usage = '''usage: %prog [options] server-address'''
    
    parser = optparse.OptionParser(usage)
  
    help = "The sending throughput"
    parser.add_option('--bw', help=help, default=1)

    help = "Client ID"
    parser.add_option('--id', help=help, default='1')
  
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
     
    port = 8888;
    N = options.bw * 2**10
    cid = options.id
    for i in range(1,N):
        msg = str(i) + "," + cid
         
        try :
            sent = s.sendto(msg, (host, port))
            print sent 
        except socket.error, msg:
            print 'Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
            sys.exit()

if __name__ == '__main__':
    main()
