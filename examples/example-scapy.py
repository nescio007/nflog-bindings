#!/usr/bin/env python3

# need root privileges

import struct
import sys
from datetime import datetime

from socket import AF_INET, AF_INET6, inet_ntoa

sys.path.append('python')
sys.path.append('build/python')
import nflog

from scapy.all import *

l = nflog.log()

def cb(payload):
    try:
        print("")
        print("Packet received [%d]" % payload.get_seq())

        try:
            tv = payload.get_timestamp()
            d = datetime.fromtimestamp(tv.tv_sec + (tv.tv_usec / 1000000.))
            print("timestamp: ", d)
        except RuntimeError as e:
            #print(e.args[0])
            pass
        data = payload.get_data()
        pkt = IP(data)
        if pkt:
            pkt.show()
        else:
            print("Error decoding packet")

        sys.stdout.flush()
        return 1
    except KeyboardInterrupt:
        print("interrupted in callback")
        global l
        print("stop the loop")
        l.stop_loop()

print("setting callback")
l.set_callback(cb)

print("open")
l.fast_open(1, AF_INET)
l.set_flags(nflog.CfgSeq)

print("prepare")
l.prepare()

print("loop nflog device until SIGINT")
try:
    l.loop()
except KeyboardInterrupt as e:
	print("loop() was interrupted")


print("unbind")
l.unbind(AF_INET)

print("close")
l.close()

