reset

#default settings: set terminal svg size 600,480 fixed enhanced font 'Arial,12' butt dashlength 1.0
set terminal svg size 800,600 fixed enhanced font 'Arial,18' butt dashlength 1.0

set xlabel offset 0,1
set ylabel offset 2,0
set xtics offset 0,0.3

set xlabel "True Cardinality"
set ylabel "Average relative error of estimate"

set xrange [629:1.03546e+07]

#set key left top
#set key opaque

f(n,k) = sqrt(exp(log(n/(exp(1)*k))*1/k) - 1)

# hll init
set xrange [629:100000]
set yrange [0:10]
set output "plots/hll_init.svg"
plot "out/hll_16"  title "m=2^{4}" with linespoints lw 2,	\
     "out/hll_256"  title "m=2^{8}" with linespoints lw 2, \
     "out/hll_4096"  title "m=2^{12}" with linespoints lw 2, \
     "out/hll_65536"  title "m=2^{16}" with linespoints lw 2
set xrange [629:1.03546e+07]
unset yrange

# rec 1
set output "plots/rec1.svg"
plot f(x,16) title "" with lines lw 1 lc "grey40", \
     f(x,64) title "" with lines lw 1 lc "grey40", \
     f(x,256) title "" with lines lw 1 lc "grey40", \
     f(x,1024) title "" with lines lw 1 lc "grey40", \
     "out/rec_1" title "k=1" with linespoints lw 2, \
     "out/rec_16" title "k=16" with linespoints lw 2, \
     "out/rec_64" title "k=64" with linespoints lw 2, \
     "out/rec_256" title "k=256" with linespoints lw 2, \
     "out/rec_1024" title "k=1024" with linespoints lw 2

# hll lowm
set output "plots/hll_lowm.svg"
plot "out/hll_16"  title "m=2^{4}" with linespoints lw 2, \
     "out/hll_64"  title "m=2^{6}" with linespoints lw 2, \
     "out/hll_256"  title "m=2^{8}" with linespoints lw 2
# hll highm
set output "plots/hll_highm.svg"
set yrange [0:1]
plot "out/hll_256"  title "m=2^{8}" with linespoints lw 2, \
     "out/hll_1024"  title "m=2^{10}" with linespoints lw 2, \
     "out/hll_4096"  title "m=2^{12}" with linespoints lw 2, \
     "out/hll_65536"  title "m=2^{16}" with linespoints lw 2
unset yrange

set logscale x 10
set logscale y 10

set key right bottom
set ylabel offset 4,0

# rec 2
set output "plots/rec2.svg"
plot f(x,16) title "" with lines lw 1 lc "grey40", \
     f(x,64) title "" with lines lw 1 lc "grey40", \
     f(x,256) title "" with lines lw 1 lc "grey40", \
     f(x,1024) title "" with lines lw 1 lc "grey40", \
     "out/rec_1" title "k=1" with linespoints lw 2, \
     "out/rec_16" title "k=16" with linespoints lw 2, \
     "out/rec_64" title "k=64" with linespoints lw 2, \
     "out/rec_256" title "k=256" with linespoints lw 2, \
     "out/rec_1024" title "k=1024" with linespoints lw 2
