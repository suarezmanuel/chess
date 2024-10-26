set terminal pngcairo size 800,600 enhanced font 'Verdana,10'
set output 'plot_minimax.png'
set xlabel "Depth"
set ylabel "Time (ms)"
set title "Time vs Depth"
set grid
plot "data2.txt" using 1:2 with linespoints lw 2 title "NegMax"
