set datafile separator ','
set term pngcairo size 550,300 enhanced font 'Verdana,9'
set output filename.".png"
set autoscale
#set offset graph 0.02, graph 0.02, graph 0.02, graph 0.02

set style line 11 lc rgb '#606060' lt 1
set border 3 back ls 11
set tics nomirror

set style line 12 lc rgb '#606060' lt 0 lw 1
set grid back ls 12

set style fill pattern 6
set title graph_title

set key below
set xlabel "Time (µs)"
set ylabel "Score of the different models"
# TODO: Add units to metrics

plot filename u 1:2 lw 2 lt rgb "red" title columnhead with lines, \
	'' u 1:3 lw 1.5 lt rgb "blue" title columnhead with lines
