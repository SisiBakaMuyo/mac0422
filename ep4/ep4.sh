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

echo ">>>>>>> Gerando um arquivo texto de: $2 MB..."
bytes=$((1048576 * $2))
base64 /dev/urandom | head -c $bytes > /tmp/arquivo_$2MB.txt
echo >> /tmp/arquivo_$2MB.txt

echo "Subindo o servidor ep4-servidor-inet_processos"
gnome-terminal -- bash -c "/tmp/./ep4-servidor-inet_processos"   # abre uma nova janela de terminal e executa 
PID_ep4_servidor_inet_processos=$(pgrep -f ep4-servidor-inet_processos)

echo ">>>>>>> Fazendo $1 clientes ecoarem um arquivo de: $2 MB..."
for ((i = 1; i < $1 + 1; i++)); do
    /tmp/./ep4-cliente-inet /tmp/arquivo_$2MB.txt; 
done


echo "Esperando os clientes terminarem..."



echo "Verificando os instantes de tempo no journald..."



echo ">>>>>>> $1 clientes encerraram a conexão"


echo ">>>>>>> Tempo para servir os $1 clientes com o ep4-servidor-inet_processos: 00:01"



echo "Enviando um sinal 15 para o servidor ep4-servidor-inet_processos..."
kill -15 $PID_ep4_servidor_inet_processos


echo "Subindo o servidor ep4-servidor-inet_threads"



echo ">>>>>>> Fazendo 10 clientes ecoarem um arquivo de: 5MB..."



echo "Esperando os clientes terminarem..."



echo "Verificando os instantes de tempo no journald..."



echo ">>>>>>> 10 clientes encerraram a conexão"



echo ">>>>>>> Tempo para servir os 10 clientes com o ep4-servidor-inet_threads: 00:01"



echo "Enviando um sinal 15 para o servidor ep4-servidor-inet_threads..."



echo "Subindo o servidor ep4-servidor-inet_muxes"



echo ">>>>>>> Fazendo 10 clientes ecoarem um arquivo de: 5MB..."



echo "Esperando os clientes terminarem..."



echo "Verificando os instantes de tempo no journald..."



echo ">>>>>>> 10 clientes encerraram a conexão"


echo ">>>>>>> Tempo para servir os 10 clientes com o ep4-servidor-inet_muxes: 00:02"



echo "Enviando um sinal 15 para o servidor ep4-servidor-inet_muxes..."



echo "Subindo o servidor ep4-servidor-unix_threads"


echo ">>>>>>> Fazendo 10 clientes ecoarem um arquivo de: 5MB..."


echo "Esperando os clientes terminarem..."


echo "Verificando os instantes de tempo no journald..."


echo ">>>>>>> 10 clientes encerraram a conexão"



echo ">>>>>>> Tempo para servir os 10 clientes com o ep4-servidor-unix_threads: 00:01"



echo "Enviando um sinal 15 para o servidor ep4-servidor-unix_threads..."



echo ">>>>>>> Gerando um arquivo texto de: 10MB..."



echo "Subindo o servidor ep4-servidor-inet_processos"



echo ">>>>>>> Fazendo 10 clientes ecoarem um arquivo de: 10MB..."


echo "Esperando os clientes terminarem..."



echo "Verificando os instantes de tempo no journald..."



echo ">>>>>>> 10 clientes encerraram a conexão"



echo ">>>>>>> Tempo para servir os 10 clientes com o ep4-servidor-inet_processos: 00:03"



echo "Enviando um sinal 15 para o servidor ep4-servidor-inet_processos..."



echo "Subindo o servidor ep4-servidor-inet_threads"



echo ">>>>>>> Fazendo 10 clientes ecoarem um arquivo de: 10MB..."



echo "Esperando os clientes terminarem..."



echo "Verificando os instantes de tempo no journald..."


echo ">>>>>>> 10 clientes encerraram a conexão"


echo ">>>>>>> Tempo para servir os 10 clientes com o ep4-servidor-inet_threads: 00:03"


echo "Enviando um sinal 15 para o servidor ep4-servidor-inet_threads..."



echo "Subindo o servidor ep4-servidor-inet_muxes"



echo ">>>>>>> Fazendo 10 clientes ecoarem um arquivo de: 10MB..."



echo "Esperando os clientes terminarem..."



echo "Verificando os instantes de tempo no journald..."


echo ">>>>>>> 10 clientes encerraram a conexão"



echo ">>>>>>> Tempo para servir os 10 clientes com o ep4-servidor-inet_muxes: 00:03"



echo "Enviando um sinal 15 para o servidor ep4-servidor-inet_muxes..."



echo "Subindo o servidor ep4-servidor-unix_threads"



echo ">>>>>>> Fazendo 10 clientes ecoarem um arquivo de: 10MB..."



echo "Esperando os clientes terminarem..."



echo "Verificando os instantes de tempo no journald..."


echo ">>>>>>> 10 clientes encerraram a conexão"


echo ">>>>>>> Tempo para servir os 10 clientes com o ep4-servidor-unix_threads: 00:01"



echo "Enviando um sinal 15 para o servidor ep4-servidor-unix_threads..."



echo ">>>>>>> Gerando o gráfico de 10 clientes com arquivos de: 5MB 10MB"




