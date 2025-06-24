#!/bin/bash

echo "Compilando ep4-servidor-inet_processos"
gcc ep4-clientes+servidores/ep4-servidor-inet_processos.c -o /tmp/ep4-servidor-inet_processos

echo "Compilando ep4-servidor-inet_threads"
gcc ep4-clientes+servidores/ep4-servidor-inet_threads.c -o /tmp/ep4-servidor-inet_threads

echo "Compilando ep4-servidor-inet_muxes"
gcc ep4-clientes+servidores/ep4-servidor-inet_muxes.c -o /tmp/ep4-servidor-inet_muxes

echo "Compilando ep4-servidor-unix_threads"
gcc ep4-clientes+servidores/ep4-servidor-unix_threads.c -o /tmp/ep4-servidor-unix_threads

echo "Compilando ep4-cliente-inet"
gcc ep4-clientes+servidores/ep4-cliente-inet.c -o /tmp/ep4-cliente-inet

echo "Compilando ep4-cliente-unix"
gcc ep4-clientes+servidores/ep4-cliente-unix.c -o /tmp/ep4-cliente-unix

lista_servidores=("ep4-servidor-inet_processos" "ep4-servidor-inet_threads" "ep4-servidor-inet_muxes" "ep4-servidor-unix_threads")
arqc=1

touch /tmp/ep4-resultados-$1.data 

#laco gerador dos arquivos
for arq in "$@"; do
    linha_data=""

    if ((arqc > 1)); then
        linha_data+="$arq "

        echo ">>>>>>> Gerando um arquivo texto de: $arq MB..."
        bytes=$((1048576 * $arq))
        base64 /dev/urandom | head -c $bytes > /tmp/arquivo_MB$arq.txt
        echo >> /tmp/arquivo_MB$arq.txt

        echo "$arq " > /tmp/ep4-resultados-$1.data 

        servc=0
        #laco ativador de servidores
        for servidor in "${lista_servidores[@]}"; do
            echo "Subindo o servidor $servidor" 
            gnome-terminal -- bash -c "/tmp/./$servidor"    
            PID_servidor=$(pgrep -f $servidor)

            #laco criador de clientes
            echo ">>>>>>> Fazendo $1 clientes ecoarem um arquivo de: $arq MB..."
            for ((j = 1; j < $1 + 1; j++)); do
                if (($servc < 3)); then
                    /tmp/./ep4-cliente-inet 127.0.0.1 < /tmp/arquivo_MB$arq.txt &>/dev/null & 
                else
                    /tmp/./ep4-cliente-unix 127.0.0.1 < /tmp/arquivo_MB$arq.txt &>/dev/null &
                fi    
            done
            ((servc++))

            output=0
            echo "Esperando os clientes terminarem..."
            while ((output != $1)); do
                output=$(journalctl -q --since "1 hour ago" | grep $servidor | grep "provavelmente enviou um exit" | wc -l)
            done

            echo "Verificando os instantes de tempo no journald..."
            dia_ini=$(journalctl -q --since "1 hour ago" | grep $servidor | grep "Passou pelo accept :-)" | head -n 1 | cut -c 1-6)
            hora_ini=$(journalctl -q --since "1 hour ago" | grep $servidor | grep "Passou pelo accept :-)" | head -n 1 | cut -c 8-15)
            data_ini=$(date -d "$dia_ini" +%Y-%m-%d)
            inicio="$data_ini $hora_ini"
            
            dia_fim=$(journalctl -q --since "1 hour ago" | grep $servidor | grep "provavelmente enviou um exit" | tail -n 1 | cut -c 1-6)
            hora_fim=$(journalctl -q --since "1 hour ago" | grep $servidor | grep "provavelmente enviou um exit" | tail -n 1 | cut -c 8-15)
            data_fim=$(date -d "$dia_ini" +%Y-%m-%d)
            fim="$data_fim $hora_fim"

            tempo_total=$(dateutils.ddiff "$inicio" "$fim" -f "%0M:%0S")
            linha_data+="$tempo_total "

            echo ">>>>>>> $1 clientes encerraram a conexão"
            echo ">>>>>>> Tempo para servir os $1 clientes com o $servidor: $tempo_total"
            echo "Enviando um sinal 15 para o servidor $servidor..."
            gnome-terminal -- bash -c "kill -15 $PID_servidor"
        done
    fi  
    
    ((arqc++)) 
    echo "$linha_data" >> /tmp/ep4-resultados-$1.data 
done    

arquivos=""
arquivosc=1
for arg in "$@"; do
    if ((arquivosc > 1)); then
        arquivos+=" $arg MB"
    fi  
    ((arquivosc++))  
done

echo ">>>>>>> Gerando o gráfico de $1 clientes com arquivos de: $arquivos"

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



