set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'plot_log.png'
set xlabel "Depth"
set ylabel "Nodes"
set title "Node Counts vs Depth"
set grid

# Apply logarithmic scaling to the y-axis
set logscale y

# Format y-axis labels to display as powers of 10
set format y '10^{%L}'

# Plot the data
plot "../records/data.txt" using 1:2 with linespoints lw 2 title "NegMax", \
     "../records/data.txt" using 1:3 with linespoints lw 2 title "Perft"
