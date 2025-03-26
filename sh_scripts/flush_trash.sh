#!/bin/sh

# Détecter l'OS et choisir la bonne corbeille
if [ -d "$HOME/.local/share/Trash/files" ]; then
    rm -rf "$HOME/.local/share/Trash/files"/*
    echo "Corbeille vidée sous Linux."
elif [ -d "$HOME/.Trash" ]; then
    rm -rf "$HOME/.Trash"/*
    echo "Corbeille vidée sous macOS."
else
    echo "Emplacement de la corbeille non trouvé."
fi
