#!/bin/bash

NumberOfNodes=50
NumberOfFlows=100
PacketSize=1024
NumberOfPacketsPerSecond=100
SpeedOfNodes=5
CoverageArea=1
Verbose=true

./ns3 run "scratch/wireless-static.cc --NumberOfNodes=$NumberOfNodes --NumberOfFlows=$NumberOfFlows --PacketSize=$PacketSize
--NumberOfPacketsPerSecond=$NumberOfPacketsPerSecond --SpeedOfNodes=$SpeedOfNodes --CoverageArea=$CoverageArea" 