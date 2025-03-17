#!/bin/bash

# Détecter et vider la corbeille selon le système
TRASH_DIR="$HOME/.local/share/Trash/files"
MAC_TRASH="$HOME/.Trash"

if [ -d "$TRASH_DIR" ]; then
    rm -rf "$TRASH_DIR"/*
    echo "Corbeille Linux vidée."
elif [ -d "$MAC_TRASH" ]; then
    rm -rf "$MAC_TRASH"/*
    echo "Corbeille macOS vidée."
else
    echo "Impossible de trouver la corbeille."
fi
