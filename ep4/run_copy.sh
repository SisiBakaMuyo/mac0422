#!/bin/bash

echo ">>>>>>> Gerando o gráfico de $1 clientes com arquivos de: ..."

touch /tmp/ep4-resultados-$1.gpi 

echo "set ydata time
set timefmt \"%M:%S\"
set format y \"%M:%S\"
set xlabel 'Dados transferidos por cliente (MB)'
set ylabel 'Tempo para atender 100 clientes concorrentes'
set terminal pdfcairo
set output \"ep4-resultados-100.pdf\"
set grid
set key top left
plot \"/tmp/ep4-resultados-$1.data\" using 1:4 with linespoints title \"Sockets da Internet: Mux de E/S\", \"/tmp/ep4-resultados-$1.data\" using 1:3 with linespoints title \"Sockets da Internet: Threads\", \"/tmp/ep4-resultados-$1.data\" using 1:2 with linespoints title \"Sockets da Internet: Processos\", \"/tmp/ep4-resultados-$1.data\" using 1:5 with linespoints title \"Sockets Unix: Threads\"
set output" > /tmp/ep4-resultados-$1.gpi

echo "load \"/tmp/ep4-resultados-100.gpi\"" | gnuplot

rm /tmp/ep4-resultados-$1.gpi
rm /tmp/ep4-resultados-$1.data
echo 0

