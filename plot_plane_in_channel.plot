reset
set terminal postscript color eps 23
set size ratio 1
set pointsize 2
set xlabel "real";
set ylabel "img";
set nokey;
# set key Left above reverse horizontal nobox spacing 0.9

# set output "problem_twitter.eps"
set output "plane.ps"


set style line 1 lc rgb "#FF0000" lt 1 lw 4
set style line 2 lc rgb "#0000FF" lt 2 lw 3
set style line 3 lc rgb "orange" lt 3 lw 3
set style line 4 lc rgb "green" lt 4 lw 3
set style line 5 lc rgb "yellow" lt 1 lw 3
set style line 6 lc rgb "black" lt 2 lw 3

set style data points

plot "h" using 1:2

########################################################

reset
set terminal postscript color eps 23
set size ratio 1
set pointsize 2
set xlabel "real";
set ylabel "img";
set nokey;
# set key Left above reverse horizontal nobox spacing 0.9

# set output "problem_twitter.eps"
set output "plane_rotated.ps"


set style line 1 lc rgb "#FF0000" lt 1 lw 4
set style line 2 lc rgb "#0000FF" lt 2 lw 3
set style line 3 lc rgb "orange" lt 3 lw 3
set style line 4 lc rgb "green" lt 4 lw 3
set style line 5 lc rgb "yellow" lt 1 lw 3
set style line 6 lc rgb "black" lt 2 lw 3

set style data points

plot "h" using 3:4