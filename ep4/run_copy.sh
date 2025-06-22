#!/bin/bash

echo ">>>>>>> Gerando um arquivo texto de: $1 MB..."
dd if=/dev/urandom of=arquivo_$1mb.txt bs=1M count=$1

