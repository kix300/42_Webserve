#!/usr/bin/env python3
"""
Script CGI avec formulaire pour tester les requêtes GET et POST
"""

import os
import sys
import urllib.parse
from datetime import datetime

def parse_query_string(query_string):
    """Parse la query string en dictionnaire"""
    params = {}
    if query_string:
        for param in query_string.split('&'):
            if '=' in param:
                key, value = param.split('=', 1)
                params[urllib.parse.unquote(key)] = urllib.parse.unquote(value)
    return params

def parse_post_data(post_data):
    """Parse les données POST"""
    params = {}
    if post_data:
        for param in post_data.split('&'):
            if '=' in param:
                key, value = param.split('=', 1)
                params[urllib.parse.unquote(key)] = urllib.parse.unquote(value)
    return params

def main():
    print("Content-Type: text/html")
    print()
    
    method = os.environ.get('REQUEST_METHOD', 'GET')
    query_string = os.environ.get('QUERY_STRING', '')
    
    print("<!DOCTYPE html>")
    print("<html>")
    print("<head>")
    print("    <title>Formulaire CGI</title>")
    print("    <style>")
    print("        body { font-family: Arial, sans-serif; margin: 40px; }")
    print("        .form-group { margin: 15px 0; }")
    print("        label { display: block; margin-bottom: 5px; font-weight: bold; }")
    print("        input, textarea, select { padding: 8px; width: 300px; }")
    print("        button { padding: 10px 20px; background: #007cba; color: white; border: none; cursor: pointer; }")
    print("        .result { background: #e8f5e8; padding: 15px; margin: 20px 0; border-left: 5px solid #4caf50; }")
    print("        .info { background: #f0f8ff; padding: 10px; margin: 10px 0; border-left: 5px solid #007cba; }")
    print("    </style>")
    print("</head>")
    print("<body>")
    print("    <h1>Formulaire CGI Webserve</h1>")
    
    # Traitement des données reçues
    if method == 'GET' and query_string:
        params = parse_query_string(query_string)
        print("    <div class='result'>")
        print("        <h2>Données GET reçues:</h2>")
        for key, value in params.items():
            print("        <p><strong>{}:</strong> {}</p>".format(key, value))
        print("    </div>")
    
    elif method == 'POST':
        content_length = os.environ.get('CONTENT_LENGTH')
        if content_length:
            try:
                content_length = int(content_length)
                post_data = sys.stdin.read(content_length)
                params = parse_post_data(post_data)
                print("    <div class='result'>")
                print("        <h2>Données POST reçues:</h2>")
                for key, value in params.items():
                    print("        <p><strong>{}:</strong> {}</p>".format(key, value))
                print("    </div>")
            except Exception as e:
                print("        <div class='result'>Erreur: {}</div>".format(str(e)))
    
    # Formulaire
    print("    <form method='POST' action='/cgi/form.py'>")
    print("        <div class='form-group'>")
    print("            <label for='nom'>Nom:</label>")
    print("            <input type='text' id='nom' name='nom' required>")
    print("        </div>")
    print("        <div class='form-group'>")
    print("            <label for='email'>Email:</label>")
    print("            <input type='email' id='email' name='email' required>")
    print("        </div>")
    print("        <div class='form-group'>")
    print("            <label for='age'>Age:</label>")
    print("            <input type='number' id='age' name='age' min='1' max='120'>")
    print("        </div>")
    print("        <div class='form-group'>")
    print("            <label for='pays'>Pays:</label>")
    print("            <select id='pays' name='pays'>")
    print("                <option value='fr'>France</option>")
    print("                <option value='be'>Belgique</option>")
    print("                <option value='ch'>Suisse</option>")
    print("                <option value='ca'>Canada</option>")
    print("            </select>")
    print("        </div>")
    print("        <div class='form-group'>")
    print("            <label for='message'>Message:</label>")
    print("            <textarea id='message' name='message' rows='4'></textarea>")
    print("        </div>")
    print("        <button type='submit'>Envoyer (POST)</button>")
    print("    </form>")

    
    # Informations techniques
    print("    <div class='info'>")
    print("        <h3>Informations de la requete:</h3>")
    print("        <p><strong>Methode:</strong> {}</p>".format(method))
    print("        <p><strong>Query String:</strong> {}</p>".format(query_string or '(vide)'))
    print("        <p><strong>Heure:</strong> {}</p>".format(datetime.now().strftime("%Y-%m-%d %H:%M:%S")))
    print("        <p><strong>Server:</strong> {} : {}</p>".format(
        os.environ.get('SERVER_NAME', 'N/A'), 
        os.environ.get('SERVER_PORT', 'N/A')
    ))
    print("    </div>")
    
    print("    <p><a href='/'>Retour a l accueil</a></p>")
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
