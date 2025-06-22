#!/bin/bash

echo "Verificando os instantes de tempo no journald..."
dia_ini=$(journalctl -q --since "3 hour ago" | grep ep4-servidor-inet_processos | grep "Passou pelo accept :-)" | head -n 1 | cut -c 1-6)
hora_ini=$(journalctl -q --since "3 hour ago" | grep ep4-servidor-inet_processos | grep "Passou pelo accept :-)" | head -n 1 | cut -c 8-15)
data_ini=$(date -d "$dia_ini" +%Y-%m-%d)
inicio="$data_ini $hora_ini"
echo "Tempo inicial dos clientes: $inicio"
dia_fim=$(journalctl -q --since "3 hour ago" | grep ep4-servidor-inet_processos | grep "provavelmente enviou um exit" | tail -n 1 | cut -c 1-6)
hora_fim=$(journalctl -q --since "3 hour ago" | grep ep4-servidor-inet_processos | grep "provavelmente enviou um exit" | tail -n 1 | cut -c 8-15)
data_fim=$(date -d "$dia_ini" +%Y-%m-%d)
fim="$data_fim $hora_fim"
echo "Tempo final dos clientes: $fim"
tempo_total=$(dateutils.ddiff "$inicio" "$fim" -f "%0M:%0S")

echo ">>>>>>> $1 clientes encerraram a conexão"
echo ">>>>>>> Tempo para servir os $1 clientes com o ep4-servidor-inet_processos: $tempo_total"

