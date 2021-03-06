reset
set terminal postscript color eps 23
set size ratio 0.6
set pointsize 2
set xlabel "time";
set ylabel "correlation";
set nokey;
# set key Left above reverse horizontal nobox spacing 0.9

# set output "problem_twitter.eps"
set output "DEBUG_coeff.ps"


set style line 1 lc rgb "#FF0000" lt 1 lw 4
set style line 2 lc rgb "#0000FF" lt 2 lw 3
set style line 3 lc rgb "orange" lt 3 lw 3
set style line 4 lc rgb "green" lt 4 lw 3
set style line 5 lc rgb "yellow" lt 1 lw 3
set style line 6 lc rgb "black" lt 2 lw 3

set style data points

plot "DEBUG_corref_PSC.txt" using 2 with lines

