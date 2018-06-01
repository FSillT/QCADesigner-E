set style line 1 lt 1 lw 0.1 ps 0.01
#set terminal pngcairo size 1024,1024
#set output 'output.png' 
set xrange [0:3.4e-11]
set multiplot layout 4, 1
set title 'lambdaz'
#plot "tmp" using 1:17 title 'lz' ls 1, "tmp" using 1:16 title 'PolIn' ls 1 lc 'red', "tmp" using 1:15 title 'Pol' ls 1 lc 'green'
plot "tmp" using 1:2 ls 1 title 'lz' 
unset key
set yrange [-1e-4:1e-4]
set title 'slz'
plot "tmp" using 1:6 ls 1 title 'slz'
set title 'lssz-lz'
set yrange [-1e-2:1e-2]
plot "tmp" using 1:7 ls 1 title 'lssz-lz'
unset key
set title 'CLK'
set yrange [*:*]
plot "tmp" using 1:14 ls 1 title 'clk' 
unset key
#set title 'clk'
#plot "tmp" using 1:14 ls 1
#unset key
#set title 'Eclk'
#plot "tmp" using 1:13 ls 1
unset multiplot
pause -1