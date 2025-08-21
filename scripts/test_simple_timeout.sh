#!/bin/bash

# Test simple de timeout de connexion
echo "=== Test Timeout - Connexion sans requête ==="

# Se connecter et ne rien envoyer pendant 15 secondes
(echo -n ""; sleep 15) | nc -w 20 localhost 8080 &
PID=$!

echo "PID du test: $PID"
echo "Attendez... le serveur devrait fermer la connexion en moins de 30s"

# Attendre que le processus se termine
wait $PID
RESULT=$?

if [ $RESULT -eq 0 ]; then
    echo "✓ Le serveur a fermé la connexion (timeout fonctionnel)"
else
    echo "✗ Connexion fermée avec erreur ou timeout du client"
fi

echo ""
echo "=== Test Timeout - Requête incomplète ==="

# Envoyer seulement le début d'une requête
(echo -e "GET / HTTP/1.1\r\nHost: localhost"; sleep 15) | nc -w 20 localhost 8080 &
PID=$!

echo "PID du test: $PID"
echo "Attendez... le serveur devrait fermer la connexion en moins de 10s (REQUEST_TIMEOUT)"

wait $PID
RESULT=$?

if [ $RESULT -eq 0 ]; then
    echo "✓ Le serveur a fermé la connexion incomplète (timeout fonctionnel)"
else
    echo "✗ Connexion fermée avec erreur ou timeout du client"
fi
