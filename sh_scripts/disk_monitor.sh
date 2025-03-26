#!/bin/bash

THRESHOLD=85  # Seuil d'alerte en pourcentage

# Déterminer la partition à surveiller
if mount | grep -q " on /home "; then
    TARGET="/home"
else
    TARGET="/"
fi

# Vérifier l'espace disque
USAGE=$(df -h "$TARGET" | awk 'NR==2 {print $5}' | sed 's/%//')

if [ "$USAGE" -gt "$THRESHOLD" ]; then
    echo "Alerte : L'espace disque de $TARGET est utilisé à ${USAGE}% !"
fi
