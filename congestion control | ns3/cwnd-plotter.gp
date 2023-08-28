# Input Parameters
_datafile1 = ARG1
_datafile2 = ARG2
_outputfile = ARG3
_title = ARG4
_xlabel = ARG5
_ylabel = ARG6
_graph1 = ARG7
_graph2 = ARG8

# Set terminal and output file
set terminal pngcairo enhanced font "arial,10"
set output _outputfile

# Set title and labels
set title _title
set xlabel _xlabel
set ylabel _ylabel

set grid
set autoscale

plot _datafile1 using 1:2 with linespoints title _graph1, _datafile2 using 1:2 with linespoints title _graph2