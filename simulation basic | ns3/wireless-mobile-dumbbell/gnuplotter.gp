# Print the number of passed arguments

# Check for the correct number of arguments
if (ARGC != 5) {
    print "Usage: plot.sh <data_file> <output_file> <title> <xlabel> <ylabel>"
    exit
}

data_file = ARG1
output_file = ARG2
_title = ARG3
_xlabel = ARG4 
_ylabel = ARG5

# Set terminal and output file
set terminal pngcairo enhanced font "Helvetica,14"
set output output_file

# Set plot title, axis labels, and legend
set title _title
set xlabel _xlabel 
set ylabel _ylabel 
set key top left

# Set grid
set grid

# Plot the data with lines and points
plot data_file using 1:2 with linespoints title "data"

