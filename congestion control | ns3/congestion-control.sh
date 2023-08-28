#! /bin/bash

verbose=false
bottleneckDataRate=50 # 50 Mbps
packetLossExp=-6
tcpVariant1="ns3::TcpNewReno"
tcpVariant2="ns3::TcpHighSpeed"
metricOutputFile="congestion-control-metrics.csv"
cwndOutputFile1="cwnd1.txt"
cwndOutputFile2="cwnd2.txt"

dataFile="data.txt"
graph1="graph1.png"
graph2="graph2.png"
graph3="graph3.png"
if [ -f "$dataFile" ]; then
    rm "$dataFile"
fi
if [ -f "$cwndOutputFile1" ]; then
    rm "$cwndOutputFile1"
fi
if [ -f "$cwndOutputFile2" ]; then
    rm "$cwndOutputFile2"
fi
touch "$dataFile"
touch "$cwndOutputFile1"
touch "$cwndOutputFile2"

# variable parameter: bottleneckDataRate 1-300 Mbps
echo "Variable Parameter: bottleneckDataRate 1-300 Mbps"
variableParameterIndex=5
throughputIndex1=17
throughputIndex2=18
for i in {1..301..50}; do
    echo "bottleneckDataRate: $i Mbps"
    ./ns3 run "scratch/congestion-control/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$i --packetLossExp=$packetLossExp --tcpVariant1=$tcpVariant1 --tcpVariant2=$tcpVariant2 --metricOutputFile=$metricOutputFile --cwndOutputFile1=$cwndOutputFile1 --cwndOutputFile2=$cwndOutputFile2"
    lastLine=$(tail -n 1 "$metricOutputFile")
    xVal=$(echo "$lastLine" | cut -d ',' -f "$variableParameterIndex")
    yVal1=$(echo "$lastLine" | cut -d ',' -f "$throughputIndex1")
    yVal2=$(echo "$lastLine" | cut -d ',' -f "$throughputIndex2")
    echo "$xVal $yVal1 $yVal2" >> "$dataFile"
done

gnuplot -c throughput-plotter.gp "$dataFile" "$graph1" "Throughput vs bottleneckDataRate" "bottleneckDataRate" "Throughput(kbps)" "$tcpVariant1" "$tcpVariant2"

# variable parameter: packetLossExp 1 - 6
echo "Variable Parameter: packetLossExp 1 - 6"
variableParameterIndex=6
if [ -f "$dataFile" ]; then
    rm "$dataFile"
fi
touch "$dataFile"
for i in {-6..-1}; do
    echo "packetLossExp: $i"
    ./ns3 run "scratch/congestion-control/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$bottleneckDataRate --packetLossExp=$i --tcpVariant1=$tcpVariant1 --tcpVariant2=$tcpVariant2 --metricOutputFile=$metricOutputFile --cwndOutputFile1=$cwndOutputFile1 --cwndOutputFile2=$cwndOutputFile2" 
    lastLine=$(tail -n 1 "$metricOutputFile")
    xVal=$(echo "$lastLine" | cut -d ',' -f "$variableParameterIndex")
    yVal1=$(echo "$lastLine" | cut -d ',' -f "$throughputIndex1")
    yVal2=$(echo "$lastLine" | cut -d ',' -f "$throughputIndex2")
    echo "$xVal $yVal1 $yVal2" >> "$dataFile"
done

gnuplot -c throughput-plotter.gp "$dataFile" "$graph2" "Throughput vs packetLossExp" "packetLossExp" "Throughput(kbps)" "$tcpVariant1" "$tcpVariant2"

# plotting cwnd
echo "Plotting cwnd"
./ns3 run "scratch/congestion-control/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$bottleneckDataRate --packetLossExp=$packetLossExp --tcpVariant1=$tcpVariant1 --tcpVariant2=$tcpVariant2 --metricOutputFile=$metricOutputFile --cwndOutputFile1=$cwndOutputFile1 --cwndOutputFile2=$cwndOutputFile2"

gnuplot -c cwnd-plotter.gp "$cwndOutputFile1" "$cwndOutputFile2" "$graph3" "Congestion Window vs Time" "Time(s)" "Congestion Window" "$tcpVariant1" "$tcpVariant2"

if [ ! -d "congestion-control-metrics" ]; then
    mkdir "congestion-control-metrics"
fi
rm -f congestion-control-metrics/*
mv "$metricOutputFile" congestion-control-metrics/
mv "$graph1" congestion-control-metrics/
mv "$graph2" congestion-control-metrics/
mv "$graph3" "congestion-control-metrics"

rm "$dataFile"
rm "$cwndOutputFile1"
rm "$cwndOutputFile2"