#!/bin/bash

echo "Gerando arquivo data"

lista_servidores=("ep4-servidor-inet_processos" "ep4-servidor-inet_threads" "ep4-servidor-inet_muxes" "ep4-servidor-unix_threads")
arqc=1

#laco gerador dos arquivos
for arq in "$@"; do
    linha_data=""

    if ((arqc > 1)); then

        linha_data+="$arq " 
        servc=0
        #laco ativador de servidores
        for servidor in "${lista_servidores[@]}"; do
            tempo_total=00:15
            # echo ">>>>>>> Tempo para servir os $1 clientes com o $servidor: $tempo_total"
            linha_data+="$tempo_total " 
        done
    fi  
    ((arqc++))  

    echo "$linha_data" >> /tmp/ep4-resultados-$1.data
done    