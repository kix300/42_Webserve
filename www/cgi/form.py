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
    print("        body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; background-color: #f5f5f5; }")
    print("        .container { background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin-bottom: 20px; }")
    print("        .card { background: white; border-radius: 8px; padding: 1.5rem; margin-bottom: 2rem; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); transition: transform 0.3s ease; }")
    print("        .btn { display: inline-block; background: #6e8efb; color: white; padding: 0.7rem 1.5rem; text-decoration: none; border-radius: 50px; transition: background 0.3s ease; margin: 5px; }")
    print("        .btn:hover { background: #5a7bf0; }")
    print("        .form-group { margin: 15px 0; }")
    print("        label { display: block; margin-bottom: 5px; font-weight: bold; }")
    print("        input, textarea, select { padding: 8px; width: 300px; border: 1px solid #ddd; border-radius: 4px; }")
    print("        button { padding: 10px 20px; background: #007cba; color: white; border: none; cursor: pointer; border-radius: 4px; }")
    print("        button:hover { background: #005a9f; }")
    print("        .result { background: #e8f5e8; padding: 15px; margin: 20px 0; border-left: 5px solid #4caf50; border-radius: 4px; }")
    print("        .info { background: #f0f8ff; padding: 10px; margin: 10px 0; border-left: 5px solid #007cba; border-radius: 4px; }")
    print("    </style>")
    print("</head>")
    print("<body>")
    print("    <h1>Formulaire CGI Webserve</h1>")
    
    # Section de navigation
    print("    <div class='card'>")
    print("        <h2>Mes Tests</h2>")
    print("        <p>Test des differents sites</p>")
    print("        <a href='/' class='btn'>HOME</a>")
    print("        <a href='/methode/' class='btn'>Test des Methodes</a>")
    print("        <a href='/test3/' class='btn'>Test de redirection serveur</a>")
    print("        <a href='/files/' class='btn'>Test de autoindex</a>")
    print("        <a href='/cgi/timeout.py' class='btn'>Test de timeout CGI</a>")
    print("    </div>")
    
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
    print("    <div class='container'>")
    print("        <h2>Formulaire de test</h2>")
    print("        <form method='POST' action='/cgi/form.py'>")
    print("            <div class='form-group'>")
    print("                <label for='nom'>Nom:</label>")
    print("                <input type='text' id='nom' name='nom' required>")
    print("            </div>")
    print("            <div class='form-group'>")
    print("                <label for='email'>Email:</label>")
    print("                <input type='email' id='email' name='email' required>")
    print("            </div>")
    print("            <div class='form-group'>")
    print("                <label for='age'>Age:</label>")
    print("                <input type='number' id='age' name='age' min='1' max='120'>")
    print("            </div>")
    print("            <div class='form-group'>")
    print("                <label for='pays'>Pays:</label>")
    print("                <select id='pays' name='pays'>")
    print("                    <option value='fr'>France</option>")
    print("                    <option value='be'>Belgique</option>")
    print("                    <option value='ch'>Suisse</option>")
    print("                    <option value='ca'>Canada</option>")
    print("                </select>")
    print("            </div>")
    print("            <div class='form-group'>")
    print("                <label for='message'>Message:</label>")
    print("                <textarea id='message' name='message' rows='4'></textarea>")
    print("            </div>")
    print("            <button type='submit'>Envoyer (POST)</button>")
    print("        </form>")
    print("    </div>")

    
    # Informations techniques
    print("    <div class='container'>")
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
    print("    </div>")
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
