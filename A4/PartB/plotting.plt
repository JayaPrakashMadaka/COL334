set terminal png size 1000, 500
set output "PartB(b).png"
set yrange [0:2]
set title "Convergence of Routers"
set xlabel "Time (in seconds)"
set ylabel "Convergence of routers"
plot "points.txt" using 1:2 with lines linecolor rgb "blue", "points.txt" using 1:2 with points lw 3
