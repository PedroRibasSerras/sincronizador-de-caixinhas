#!/bin/bash

# Check for root
if [ "$EUID" -ne 0 ]; then
    echo "ERRO: POR FAVOR DÊ PERMISSAO DE ADMIN UTILIZANDO O 'sudo' (SEM ASPAS) NA FRENTE DO COMANDO."
    exit
fi

sudo apt install youtube-dl vlc libvlc-dev