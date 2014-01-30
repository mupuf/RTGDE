set datafile separator ','
#set term pngcairo size 800,600
set autoscale
set xrange [0:*]
set offset graph 0.02, graph 0.02, graph 0.02, graph 0.02

set style fill pattern 6
set title "Prediction Throughput"

plot './build/pred_average_throughput_5.csv' u 1:6:5 lt rgb "yellow" title "congestion" w filledcu below, \
     '' u 1:2 lw 2 lt rgb "green" title columnhead with lines, \
     '' u 1:3 lw 1.5 lt rgb "blue" title columnhead with lines, \
     '' u 1:4 lw 1.5 lt rgb "#6495ED" title columnhead with lines, \
     '' u 1:5 lw 1.5 lt rgb "#1E90FF" title columnhead with lines, \
     '' u 1:6 lw 2 lt rgb "red" title columnhead with lines

pause -1