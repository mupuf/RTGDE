#!/bin/sh

gnuplot plot_usage_graph.plot || exit 1
convert -append fsm_pred_pgraph_1_st_IDLE.png fsm_pred_pgraph_1_st_ACTIVE.png fsm_pred_graph_1.png || exit 1
gwenview fsm_pred_graph_1.png