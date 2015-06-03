#!/bin/bash

for x in 71 72 76 77 78 80 81 82 83 84 85 86 88
{
    ssh node$x -t "sudo ifconfig enp2s0 192.168.4.$x"
}

for x in 71 72 76 77 78 80 83 84 85 86 88
{
    ssh node$x "/usr/sbin/ifconfig enp2s0" 
}
#ssh node71 -t 'sudo ifconfig enp2s0 192.168.4.
