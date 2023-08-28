# Input Parameters
_datafile = ARG1
_outputfile = ARG2
_title = ARG3
_xlabel = ARG4
_ylabel = ARG5
_graph1 = ARG6
_graph2 = ARG7

# Set terminal and output file
set terminal pngcairo enhanced font "arial,10"
set output _outputfile

# Set title and labels
set title _title
set xlabel _xlabel
set ylabel _ylabel

set grid
set autoscale

plot _datafile using 1:2 with linespoints title _graph1, _datafile using 1:3 with linespoints title _graph2