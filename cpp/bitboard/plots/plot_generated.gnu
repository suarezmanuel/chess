set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'plots/plot_generated.png'
set xlabel 'Depth'
set ylabel 'Nodes'
set title 'Negamax and Perft Performance'
set grid
set logscale y
set format y '10^{%L}'
plot "./records/data_Perft.txt" using 1:2 with linespoints lw 2 lc rgb "#556B2F" title "Perft"
