#!/usr/bin/env python3
"""
Script CGI de test simple
"""

import os
import sys
from datetime import datetime

def main():
    print("Content-Type: text/html")
    print()  # Ligne vide obligatoire entre headers et body
    
    print("<!DOCTYPE html>")
    print("<html>")
    print("<head>")
    print("    <title>Test CGI</title>")
    print("    <style>")
    print("        body { font-family: Arial, sans-serif; margin: 40px; }")
    print("        .info { background: #f0f0f0; padding: 10px; margin: 10px 0; }")
    print("        .env-var { margin: 5px 0; }")
    print("    </style>")
    print("</head>")
    print("<body>")
    print("    <h1>🐍 Script CGI Python</h1>")
    print("    <p>Ce script fonctionne ! Date et heure: {}</p>".format(datetime.now().strftime("%Y-%m-%d %H:%M:%S")))
    
    print("    <div class='info'>")
    print("        <h2>Informations de la requête</h2>")
    print("        <div class='env-var'><strong>Méthode:</strong> {}</div>".format(os.environ.get('REQUEST_METHOD', 'N/A')))
    print("        <div class='env-var'><strong>Script:</strong> {}</div>".format(os.environ.get('SCRIPT_NAME', 'N/A')))
    print("        <div class='env-var'><strong>Query String:</strong> {}</div>".format(os.environ.get('QUERY_STRING', 'N/A')))
    print("        <div class='env-var'><strong>Server:</strong> {}</div>".format(os.environ.get('SERVER_NAME', 'N/A')))
    print("        <div class='env-var'><strong>Port:</strong> {}</div>".format(os.environ.get('SERVER_PORT', 'N/A')))
    print("    </div>")
    
    # Afficher les données POST si disponibles
    if os.environ.get('REQUEST_METHOD') == 'POST':
        content_length = os.environ.get('CONTENT_LENGTH')
        if content_length:
            try:
                content_length = int(content_length)
                post_data = sys.stdin.read(content_length)
                print("    <div class='info'>")
                print("        <h2>Données POST reçues</h2>")
                print("        <pre>{}</pre>".format(post_data))
                print("    </div>")
            except (ValueError, Exception) as e:
                print("        <div class='info'>Erreur lecture POST: {}</div>".format(str(e)))
    
    print("    <div class='info'>")
    print("        <h2>Variables d'environnement CGI</h2>")
    for key, value in sorted(os.environ.items()):
        if key.startswith(('HTTP_', 'SERVER_', 'REQUEST_', 'SCRIPT_', 'QUERY_', 'CONTENT_', 'GATEWAY_')):
            print("        <div class='env-var'><strong>{}:</strong> {}</div>".format(key, value))
    print("    </div>")
    
    print("    <p><a href='/'>Retour à l'accueil</a></p>")
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
