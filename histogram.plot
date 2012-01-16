clear
reset
set key off
set border 3
set auto
 
set xrange[0:30]
set xtics 1
 
# Make some suitable labels.
set title "Collisions"
set xlabel "Number of collisions"
set ylabel "Count"
 
set terminal png enhanced font arial 14 size 800, 600
ft="png"
# Set the output-file name.
set output "collisions.".ft
 
set style histogram clustered gap 1
set style fill solid border -1
 
binwidth=1
set boxwidth binwidth
bin(x,width)=width*floor(x/width) + binwidth/2.0
 
plot 'report.txt' using (bin($1,binwidth)):(1.0) smooth freq with boxes
