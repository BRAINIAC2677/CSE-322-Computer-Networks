#!/bin/bash

Verbose=false
NumberOfNodes=10
NumberOfFlows=50
PacketSize=1024
NumberOfPacketsPerSecond=100
SpeedOfNodes=5
CoverageArea=1
SimulationTime=1.2
MetricOutputFile="wireless-mobile-dumbbell-metrics.csv"

packetDeliveryRatioIndex=9   
throughputIndex=10
dataFile1="data1.txt"
dataFile2="data2.txt"

# Variable Parameter: Number of Nodes [10, 20, 30, 40, 50]
echo "Variable Parameter: Number of Nodes [10, 20, 30, 40, 50]"
variableParameterIndex=1   
if [ -f "$dataFile1" ]; then
    rm "$dataFile1"
fi
if [ -f "$dataFile2" ]; then
    rm "$dataFile2"
fi
touch "$dataFile1"
touch "$dataFile2"
for i in {10..50..10}
do
    echo "Number of Nodes: $i"
    ./ns3 run "scratch/wireless-mobile-dumbbell.cc --Verbose=$Verbose --NumberOfNodes=$i --NumberOfFlows=$NumberOfFlows --PacketSize=$PacketSize --NumberOfPacketsPerSecond=$NumberOfPacketsPerSecond --SpeedOfNodes=$SpeedOfNodes --CoverageArea=$CoverageArea --SimulationTime=$SimulationTime --MetricOutputFile=$MetricOutputFile"
    lastLine=$(tail -n 1 "$MetricOutputFile")
    xVal=$(echo "$lastLine" | cut -d ',' -f "$variableParameterIndex")
    yVal=$(echo "$lastLine" | cut -d ',' -f "$packetDeliveryRatioIndex")
    echo "$xVal $yVal" >> "$dataFile1"
    yVal=$(echo "$lastLine" | cut -d ',' -f "$throughputIndex")
    echo "$xVal $yVal" >> "$dataFile2"
done
graph1=11.png
graph2=12.png
if [ -f "$graph1" ]; then
    rm "$graph1"
fi
if [ -f "$graph2" ]; then
    rm "$graph2"
fi
gnuplot -c gnuplotter.gp "$dataFile1" "$graph1" "Packet Delivery Ratio vs Number of Nodes" "Number of Nodes" "Packet Delivery Ratio"
gnuplot -c gnuplotter.gp "$dataFile2" "$graph2" "Throughput vs Number of Nodes" "Number of Nodes" "Throughput"

# Variable Parameter: Number of Flows [10, 20, 30, 40, 50]
echo "Variable Parameter: Number of Flows [10, 20, 30, 40, 50]"
variableParameterIndex=2   
if [ -f "$dataFile1" ]; then
    rm "$dataFile1"
fi
if [ -f "$dataFile2" ]; then
    rm "$dataFile2"
fi
touch "$dataFile1"
touch "$dataFile2"
for i in {10..50..10}
do
    echo "Number of Flows: $i"
    ./ns3 run "scratch/wireless-mobile-dumbbell.cc --Verbose=$Verbose --NumberOfNodes=$NumberOfNodes --NumberOfFlows=$i --PacketSize=$PacketSize --NumberOfPacketsPerSecond=$NumberOfPacketsPerSecond --SpeedOfNodes=$SpeedOfNodes --CoverageArea=$CoverageArea --SimulationTime=$SimulationTime --MetricOutputFile=$MetricOutputFile"
    lastLine=$(tail -n 1 "$MetricOutputFile")
    xVal=$(echo "$lastLine" | cut -d ',' -f "$variableParameterIndex")
    yVal=$(echo "$lastLine" | cut -d ',' -f "$packetDeliveryRatioIndex")
    echo "$xVal $yVal" >> "$dataFile1"
    yVal=$(echo "$lastLine" | cut -d ',' -f "$throughputIndex")
    echo "$xVal $yVal" >> "$dataFile2"
done
graph1=21.png
graph2=22.png
if [ -f "$graph1" ]; then
    rm "$graph1"
fi
if [ -f "$graph2" ]; then
    rm "$graph2"
fi
gnuplot -c gnuplotter.gp "$dataFile1" "$graph1" "Packet Delivery Ratio vs Number of Flows" "Number of Flows" "Packet Delivery Ratio"
gnuplot -c gnuplotter.gp "$dataFile2" "$graph2" "Throughput vs Number of Flows" "Number of Flows" "Throughput"

# Variable Parameter: Number of packets per second [100, 200, 300, 400, 500]
echo "Variable Parameter: Number of packets per second [100, 200, 300, 400, 500]"
variableParameterIndex=4   
if [ -f "$dataFile1" ]; then
    rm "$dataFile1"
fi
if [ -f "$dataFile2" ]; then
    rm "$dataFile2"
fi
touch "$dataFile1"
touch "$dataFile2"
for i in {100..500..100}
do
    echo "Number of Packets Per Second: $i"
    ./ns3 run "scratch/wireless-mobile-dumbbell.cc --Verbose=$Verbose --NumberOfNodes=$NumberOfNodes --NumberOfFlows=$NumberOfFlows --PacketSize=$PacketSize --NumberOfPacketsPerSecond=$i --SpeedOfNodes=$SpeedOfNodes --CoverageArea=$CoverageArea --SimulationTime=$SimulationTime --MetricOutputFile=$MetricOutputFile"
    lastLine=$(tail -n 1 "$MetricOutputFile")
    xVal=$(echo "$lastLine" | cut -d ',' -f "$variableParameterIndex")
    yVal=$(echo "$lastLine" | cut -d ',' -f "$packetDeliveryRatioIndex")
    echo "$xVal $yVal" >> "$dataFile1"
    yVal=$(echo "$lastLine" | cut -d ',' -f "$throughputIndex")
    echo "$xVal $yVal" >> "$dataFile2"
done
graph1=31.png
graph2=32.png
if [ -f "$graph1" ]; then
    rm "$graph1"
fi
if [ -f "$graph2" ]; then
    rm "$graph2"
fi
gnuplot -c gnuplotter.gp "$dataFile1" "$graph1" "Packet Delivery Ratio vs Number of Packets Per Second" "Number of Packets Per Second" "Packet Delivery Ratio"
gnuplot -c gnuplotter.gp "$dataFile2" "$graph2" "Throughput vs Number of Packets Per Second" "Number of Packets Per Second" "Throughput"

# Variable Parameter: Speed of Nodes [5, 10, 15, 20, 25] 
echo "Variable Parameter: Speed of Nodes [5, 10, 15, 20, 25]"
variableParameterIndex=5   
if [ -f "$dataFile1" ]; then
    rm "$dataFile1"
fi
if [ -f "$dataFile2" ]; then
    rm "$dataFile2"
fi
touch "$dataFile1"
touch "$dataFile2"
for i in {5..25..5}
do
    echo "Speed of Nodes: $i"
    ./ns3 run "scratch/wireless-mobile-dumbbell.cc --Verbose=$Verbose --NumberOfNodes=$NumberOfNodes --NumberOfFlows=$NumberOfFlows --PacketSize=$PacketSize --NumberOfPacketsPerSecond=$NumberOfPacketsPerSecond --SpeedOfNodes=$i --CoverageArea=$CoverageArea --SimulationTime=$SimulationTime --MetricOutputFile=$MetricOutputFile"
    lastLine=$(tail -n 1 "$MetricOutputFile")
    xVal=$(echo "$lastLine" | cut -d ',' -f "$variableParameterIndex")
    yVal=$(echo "$lastLine" | cut -d ',' -f "$packetDeliveryRatioIndex")
    echo "$xVal $yVal" >> "$dataFile1"
    yVal=$(echo "$lastLine" | cut -d ',' -f "$throughputIndex")
    echo "$xVal $yVal" >> "$dataFile2"
done
graph1=41.png
graph2=42.png
if [ -f "$graph1" ]; then
    rm "$graph1"
fi
if [ -f "$graph2" ]; then
    rm "$graph2"
fi
gnuplot -c gnuplotter.gp "$dataFile1" "$graph1" "Packet Delivery Ratio vs Speed of Nodes" "Speed of Nodes" "Packet Delivery Ratio"
gnuplot -c gnuplotter.gp "$dataFile2" "$graph2" "Throughput vs Speed of Nodes" "Speed of Nodes" "Throughput"

rm "$dataFile1"
rm "$dataFile2"

if [ ! -d "wireless-mobile-dumbbell-metrics" ]; then
    mkdir wireless-mobile-dumbbell-metrics
fi
rm -f wireless-mobile-dumbbell-metrics/*
for file in *.png
do
    if [[ $file == *"11"* ]] || [[ $file == *"12"* ]] || [[ $file == *"21"* ]] || [[ $file == *"22"* ]] || [[ $file == *"31"* ]] || [[ $file == *"32"* ]] || [[ $file == *"41"* ]] || [[ $file == *"42"* ]]; then
        mv "$file" wireless-mobile-dumbbell-metrics
    fi
done
mv "$MetricOutputFile" wireless-mobile-dumbbell-metrics