set datafile separator ','
set term pngcairo size 800,600
set autoscale
set key autotitle columnhead
set xlabel "Time (µs, step=1µs)"
set ylabel "Probability"
set logscale y

set xrange [-100:*]
set output "fsm_pred_pgraph_1_st_IDLE.png"
set title "Density of probability to switch from state IDLE to ACTIVE"
plot "../build/fsm_pred_pgraph_1_st_IDLE.csv" using 1:2 w l

set autoscale
set xrange [-100:*]
set output "fsm_pred_pgraph_1_st_ACTIVE.png"
set title "Density of probability to switch from state ACTIVE to IDLE"
plot "../build/fsm_pred_pgraph_1_st_ACTIVE.csv" using 1:2 w l
