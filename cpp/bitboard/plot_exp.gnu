set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'plot_exp.png'
set xlabel "Depth"
set ylabel "Nodes"
set title "Node Counts vs Depth"
set grid
plot "data.txt" using 1:2 with linespoints lw 2 title "NegMax", \
     "data.txt" using 1:3 with linespoints lw 2 title "Perft"
