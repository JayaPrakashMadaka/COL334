set terminal png size 1000, 800
set output "taskA_connection3_new.png"
set title "Congestion Window Calculation"
set xlabel "Time (in seconds)"
set ylabel "Congestion window size"
plot "taskA_connection3_new.cwnd" using 1:2 with lines linecolor rgb "blue"
