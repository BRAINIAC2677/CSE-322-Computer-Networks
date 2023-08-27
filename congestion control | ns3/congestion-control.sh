#! /bin/bash

verbose=false
bottleneckDataRate=$(( 50 * 1024 * 1024 )) # 50 Mbps
packetLossRate=0.000001

./ns3 run "scratch/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$bottleneckDataRate --packetLossRate=$packetLossRate" 