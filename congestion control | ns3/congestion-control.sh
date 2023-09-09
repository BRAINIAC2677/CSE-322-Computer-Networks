#! /bin/bash

if [ ! -d "congestion-control-metrics" ]; then
    mkdir "congestion-control-metrics"
fi
rm -rf "congestion-control-metrics/*"

# --------------------------- TcpHighSpeed vs TcpAdaptiveReno ---------------------------
tcpVariant1="ns3::TcpHighSpeed"
tcpVariant2="ns3::TcpAdaptiveReno"

graph1="graph1.png"
graph2="graph2.png"
graph3="graph3.png"
graph4="graph4.png"
graph5="graph5.png"

verbose=false
bottleneckDataRate=50 # 50 Mbps
packetLossExp=-6
metricOutputFile="congestion-control-metrics.csv"
cwndOutputFile1="cwnd1.txt"
cwndOutputFile2="cwnd2.txt"

dataFile="data.txt"
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

throughputIndex1=17
throughputIndex2=18
jainIndex=19

# variable parameter: bottleneckDataRate 1-300 Mbps
echo "bottleneckDataRate vs throughput"
variableParameterIndex=5
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

# variable parameter: bottleneckDataRate 1-300 Mbps
if [ -f "$dataFile" ]; then
    rm "$dataFile"
fi
touch "$dataFile"
echo "botleneckDataRate vs Jain's Index"
variableParameterIndex=5
for i in {1..301..50}; do
    echo "bottleneckDataRate: $i Mbps"
    ./ns3 run "scratch/congestion-control/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$i --packetLossExp=$packetLossExp --tcpVariant1=$tcpVariant1 --tcpVariant2=$tcpVariant2 --metricOutputFile=$metricOutputFile --cwndOutputFile1=$cwndOutputFile1 --cwndOutputFile2=$cwndOutputFile2"
    lastLine=$(tail -n 1 "$metricOutputFile")
    xVal=$(echo "$lastLine" | cut -d ',' -f "$variableParameterIndex")
    yVal=$(echo "$lastLine" | cut -d ',' -f "$jainIndex")
    echo "$xVal $yVal" >> "$dataFile"
done

gnuplot -c jain-plotter.gp "$dataFile" "$graph4" "Jain's Index vs bottleneckDataRate" "bottleneckDataRate" "Jain's Index" "$tcpVariant1 vs $tcpVariant2"

# variable parameter: packetLossExp 1 - 6
echo "packetLossExp vs throughput"
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

# variable parameter: packetLossExp 1 - 6
echo "packetLossExp vs Jain's Index"
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
    yVal=$(echo "$lastLine" | cut -d ',' -f "$jainIndex")
    echo "$xVal $yVal" >> "$dataFile"
done

gnuplot -c jain-plotter.gp "$dataFile" "$graph5" "Jain's Index vs packetLossExp" "packetLossExp" "Jain's Index" "$tcpVariant1 vs $tcpVariant2"

# plotting cwnd
echo "Plotting cwnd"
./ns3 run "scratch/congestion-control/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$bottleneckDataRate --packetLossExp=$packetLossExp --tcpVariant1=$tcpVariant1 --tcpVariant2=$tcpVariant2 --metricOutputFile=$metricOutputFile --cwndOutputFile1=$cwndOutputFile1 --cwndOutputFile2=$cwndOutputFile2"

gnuplot -c cwnd-plotter.gp "$cwndOutputFile1" "$cwndOutputFile2" "$graph3" "Congestion Window vs Time" "Time(s)" "Congestion Window" "$tcpVariant1" "$tcpVariant2"

mv "$graph1" congestion-control-metrics/
mv "$graph2" congestion-control-metrics/
mv "$graph3" "congestion-control-metrics"
mv "$graph4" "congestion-control-metrics"
mv "$graph5" "congestion-control-metrics"

rm "$dataFile"
rm "$cwndOutputFile1"
rm "$cwndOutputFile2"

# --------------------------- TcpHighSpeed vs TcpNewReno ---------------------------
tcpVariant1="ns3::TcpHighSpeed"
tcpVariant2="ns3::TcpNewReno"

graph1="graph6.png"
graph2="graph7.png"
graph3="graph8.png"
graph4="graph9.png"
graph5="graph10.png"

verbose=false
bottleneckDataRate=50 # 50 Mbps
packetLossExp=-6
metricOutputFile="congestion-control-metrics.csv"
cwndOutputFile1="cwnd1.txt"
cwndOutputFile2="cwnd2.txt"

dataFile="data.txt"
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

throughputIndex1=17
throughputIndex2=18
jainIndex=19

# variable parameter: bottleneckDataRate 1-300 Mbps
echo "bottleneckDataRate vs throughput"
variableParameterIndex=5
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

# variable parameter: bottleneckDataRate 1-300 Mbps
if [ -f "$dataFile" ]; then
    rm "$dataFile"
fi
touch "$dataFile"
echo "botleneckDataRate vs Jain's Index"
variableParameterIndex=5
for i in {1..301..50}; do
    echo "bottleneckDataRate: $i Mbps"
    ./ns3 run "scratch/congestion-control/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$i --packetLossExp=$packetLossExp --tcpVariant1=$tcpVariant1 --tcpVariant2=$tcpVariant2 --metricOutputFile=$metricOutputFile --cwndOutputFile1=$cwndOutputFile1 --cwndOutputFile2=$cwndOutputFile2"
    lastLine=$(tail -n 1 "$metricOutputFile")
    xVal=$(echo "$lastLine" | cut -d ',' -f "$variableParameterIndex")
    yVal=$(echo "$lastLine" | cut -d ',' -f "$jainIndex")
    echo "$xVal $yVal" >> "$dataFile"
done

gnuplot -c jain-plotter.gp "$dataFile" "$graph4" "Jain's Index vs bottleneckDataRate" "bottleneckDataRate" "Jain's Index" "$tcpVariant1 vs $tcpVariant2"

# variable parameter: packetLossExp 1 - 6
echo "packetLossExp vs throughput"
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

# variable parameter: packetLossExp 1 - 6
echo "packetLossExp vs Jain's Index"
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
    yVal=$(echo "$lastLine" | cut -d ',' -f "$jainIndex")
    echo "$xVal $yVal" >> "$dataFile"
done

gnuplot -c jain-plotter.gp "$dataFile" "$graph5" "Jain's Index vs packetLossExp" "packetLossExp" "Jain's Index" "$tcpVariant1 vs $tcpVariant2"

# plotting cwnd
echo "Plotting cwnd"
./ns3 run "scratch/congestion-control/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$bottleneckDataRate --packetLossExp=$packetLossExp --tcpVariant1=$tcpVariant1 --tcpVariant2=$tcpVariant2 --metricOutputFile=$metricOutputFile --cwndOutputFile1=$cwndOutputFile1 --cwndOutputFile2=$cwndOutputFile2"

gnuplot -c cwnd-plotter.gp "$cwndOutputFile1" "$cwndOutputFile2" "$graph3" "Congestion Window vs Time" "Time(s)" "Congestion Window" "$tcpVariant1" "$tcpVariant2"

mv "$graph1" congestion-control-metrics/
mv "$graph2" congestion-control-metrics/
mv "$graph3" "congestion-control-metrics"
mv "$graph4" "congestion-control-metrics"
mv "$graph5" "congestion-control-metrics"

rm "$dataFile"
rm "$cwndOutputFile1"
rm "$cwndOutputFile2"

# --------------------------- TcpHighSpeed vs TcpWestwoodPlus---------------------------
tcpVariant1="ns3::TcpHighSpeed"
tcpVariant2="ns3::TcpWestwoodPlus"

graph1="graph11.png"
graph2="graph12.png"
graph3="graph13.png"
graph4="graph14.png"
graph5="graph15.png"

verbose=false
bottleneckDataRate=50 # 50 Mbps
packetLossExp=-6
metricOutputFile="congestion-control-metrics.csv"
cwndOutputFile1="cwnd1.txt"
cwndOutputFile2="cwnd2.txt"

dataFile="data.txt"
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

throughputIndex1=17
throughputIndex2=18
jainIndex=19

# variable parameter: bottleneckDataRate 1-300 Mbps
echo "bottleneckDataRate vs throughput"
variableParameterIndex=5
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

# variable parameter: bottleneckDataRate 1-300 Mbps
if [ -f "$dataFile" ]; then
    rm "$dataFile"
fi
touch "$dataFile"
echo "botleneckDataRate vs Jain's Index"
variableParameterIndex=5
for i in {1..301..50}; do
    echo "bottleneckDataRate: $i Mbps"
    ./ns3 run "scratch/congestion-control/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$i --packetLossExp=$packetLossExp --tcpVariant1=$tcpVariant1 --tcpVariant2=$tcpVariant2 --metricOutputFile=$metricOutputFile --cwndOutputFile1=$cwndOutputFile1 --cwndOutputFile2=$cwndOutputFile2"
    lastLine=$(tail -n 1 "$metricOutputFile")
    xVal=$(echo "$lastLine" | cut -d ',' -f "$variableParameterIndex")
    yVal=$(echo "$lastLine" | cut -d ',' -f "$jainIndex")
    echo "$xVal $yVal" >> "$dataFile"
done

gnuplot -c jain-plotter.gp "$dataFile" "$graph4" "Jain's Index vs bottleneckDataRate" "bottleneckDataRate" "Jain's Index" "$tcpVariant1 vs $tcpVariant2"

# variable parameter: packetLossExp 1 - 6
echo "packetLossExp vs throughput"
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

# variable parameter: packetLossExp 1 - 6
echo "packetLossExp vs Jain's Index"
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
    yVal=$(echo "$lastLine" | cut -d ',' -f "$jainIndex")
    echo "$xVal $yVal" >> "$dataFile"
done

gnuplot -c jain-plotter.gp "$dataFile" "$graph5" "Jain's Index vs packetLossExp" "packetLossExp" "Jain's Index" "$tcpVariant1 vs $tcpVariant2"

# plotting cwnd
echo "Plotting cwnd"
./ns3 run "scratch/congestion-control/congestion-control.cc --verbose=$verbose --bottleneckDataRate=$bottleneckDataRate --packetLossExp=$packetLossExp --tcpVariant1=$tcpVariant1 --tcpVariant2=$tcpVariant2 --metricOutputFile=$metricOutputFile --cwndOutputFile1=$cwndOutputFile1 --cwndOutputFile2=$cwndOutputFile2"

gnuplot -c cwnd-plotter.gp "$cwndOutputFile1" "$cwndOutputFile2" "$graph3" "Congestion Window vs Time" "Time(s)" "Congestion Window" "$tcpVariant1" "$tcpVariant2"

mv "$graph1" congestion-control-metrics/
mv "$graph2" congestion-control-metrics/
mv "$graph3" "congestion-control-metrics"
mv "$graph4" "congestion-control-metrics"
mv "$graph5" "congestion-control-metrics"

rm "$dataFile"
rm "$cwndOutputFile1"
rm "$cwndOutputFile2"

mv "$metricOutputFile" "congestion-control-metrics"