#!/bin/bash

echo ">>>>>>> Gerando um arquivo texto de: $1 MB..."
bytes=$((1048576 * $1))
base64 /dev/urandom | head -c $bytes > arquivo_$1MB.txt
echo >> arquivo_$1MB.txt


