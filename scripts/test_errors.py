# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_webserve.py                                   :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/07 20:22:16 by kduroux           #+#    #+#              #
#    Updated: 2025/08/19 00:00:00 by kduroux          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import requests
import json
import sys
import time

def test_webserve_functionality(base_url):
    """Test des fonctionnalités spécifiques du serveur Webserve"""
    results = {
        "success": [],
        "errors": [],
        "warnings": []
    }

    print(f"\n🚀 Tests du serveur Webserve: {base_url}\n")

    # Test 1: Page d'accueil
    print("=== Test de base ===")
    try:
        response = requests.get(f"{base_url}/")
        if response.status_code == 200:
            results["success"].append("✓ Page d'accueil accessible (200 OK)")
        else:
            results["errors"].append(f"✗ Page d'accueil: code {response.status_code}")
    except Exception as e:
        results["errors"].append(f"✗ Page d'accueil: Exception - {str(e)}")

    # Test 2: Fichiers statiques
    print("=== Test fichiers statiques ===")
    static_files = ["/files/test1.txt", "/files/test2.txt", "/files/subdir/subfile.txt"]
    for file_path in static_files:
        try:
            response = requests.get(f"{base_url}{file_path}")
            if response.status_code == 200:
                results["success"].append(f"✓ Fichier statique {file_path} accessible")
            else:
                results["warnings"].append(f"⚠ Fichier statique {file_path}: code {response.status_code}")
        except Exception as e:
            results["errors"].append(f"✗ Fichier statique {file_path}: Exception - {str(e)}")

    # Test 3: Directory listing (autoindex)
    print("=== Test directory listing ===")
    try:
        response = requests.get(f"{base_url}/files/")
        if response.status_code == 200 and "Index of" in response.text:
            results["success"].append("✓ Directory listing activé pour /files/")
        elif response.status_code == 200:
            results["warnings"].append("⚠ /files/ accessible mais pas de listing visible")
        else:
            results["warnings"].append(f"⚠ Directory listing /files/: code {response.status_code}")
    except Exception as e:
        results["errors"].append(f"✗ Directory listing: Exception - {str(e)}")

    # Test 4: Méthodes HTTP
    print("=== Test méthodes HTTP ===")
    
    # GET sur différentes locations
    locations = ["/kix/", "/form/", "/methode/", "/upload/"]
    for location in locations:
        try:
            response = requests.get(f"{base_url}{location}")
            if response.status_code in [200, 301, 302]:
                results["success"].append(f"✓ GET {location}: code {response.status_code}")
            else:
                results["warnings"].append(f"⚠ GET {location}: code {response.status_code}")
        except Exception as e:
            results["errors"].append(f"✗ GET {location}: Exception - {str(e)}")

    # POST sur locations autorisées
    post_data = {"test": "data", "field": "value"}
    post_locations = ["/form/", "/methode/", "/upload/"]
    for location in post_locations:
        try:
            response = requests.post(f"{base_url}{location}", data=post_data)
            if response.status_code in [200, 201, 302]:
                results["success"].append(f"✓ POST {location}: code {response.status_code}")
            else:
                results["warnings"].append(f"⚠ POST {location}: code {response.status_code}")
        except Exception as e:
            results["errors"].append(f"✗ POST {location}: Exception - {str(e)}")

    # DELETE sur location autorisée
    try:
        response = requests.delete(f"{base_url}/methode/")
        if response.status_code in [200, 204, 405]:  # 405 si DELETE pas implémenté
            results["success"].append(f"✓ DELETE /methode/: code {response.status_code}")
        else:
            results["warnings"].append(f"⚠ DELETE /methode/: code {response.status_code}")
    except Exception as e:
        results["errors"].append(f"✗ DELETE /methode/: Exception - {str(e)}")

    return results

def test_http_errors(base_url):
    """Test des codes d'erreur HTTP spécifiques à Webserve"""
    results = {
        "success": [],
        "errors": [],
        "warnings": []
    }

    print("\n=== Tests des erreurs HTTP ===")
    
    # Test 404 - Fichier inexistant
    try:
        response = requests.get(f"{base_url}/inexistent-file.html")
        if response.status_code == 404:
            results["success"].append("✓ Erreur 404 pour fichier inexistant")
        else:
            results["warnings"].append(f"⚠ Fichier inexistant: code {response.status_code} (attendu 404)")
    except Exception as e:
        results["errors"].append(f"✗ Test 404: Exception - {str(e)}")

    # Test 405 - Méthode non autorisée
    try:
        # PUT sur /files/ qui n'autorise que GET
        response = requests.put(f"{base_url}/files/")
        if response.status_code == 405:
            results["success"].append("✓ Erreur 405 pour méthode non autorisée")
        else:
            results["warnings"].append(f"⚠ Méthode non autorisée: code {response.status_code} (attendu 405)")
    except Exception as e:
        results["errors"].append(f"✗ Test 405: Exception - {str(e)}")

    # Test 501 - Méthode non implémentée
    try:
        response = requests.patch(f"{base_url}/")
        if response.status_code == 501:
            results["success"].append("✓ Erreur 501 pour méthode non implémentée (PATCH)")
        else:
            results["warnings"].append(f"⚠ Méthode PATCH: code {response.status_code} (attendu 501)")
    except Exception as e:
        results["errors"].append(f"✗ Test 501: Exception - {str(e)}")

    # Test 413 - Payload trop large
    try:
        large_data = {"data": "x" * 200000}  # > 128k limit
        response = requests.post(f"{base_url}/form/", data=large_data)
        if response.status_code == 413:
            results["success"].append("✓ Erreur 413 pour payload trop large")
        else:
            results["warnings"].append(f"⚠ Payload large: code {response.status_code} (attendu 413)")
    except Exception as e:
        results["errors"].append(f"✗ Test 413: Exception - {str(e)}")

    # Test 400 - Requête malformée (simulation avec headers invalides)
    try:
        headers = {"Content-Length": "invalid"}
        response = requests.get(f"{base_url}/", headers=headers)
        if response.status_code == 400:
            results["success"].append("✓ Erreur 400 pour requête malformée")
        else:
            results["warnings"].append(f"⚠ Headers invalides: code {response.status_code}")
    except Exception as e:
        results["warnings"].append(f"⚠ Test 400: Exception - {str(e)}")

    return results

def test_redirections(base_url):
    """Test des redirections"""
    results = {
        "success": [],
        "errors": [],
        "warnings": []
    }

    print("\n=== Tests des redirections ===")
    
    # Test redirection configurée /test3/ -> http://127.0.0.1:8082
    try:
        response = requests.get(f"{base_url}/test3/", allow_redirects=False)
        if response.status_code == 302:
            location = response.headers.get('Location', '')
            if "127.0.0.1:8082" in location:
                results["success"].append("✓ Redirection 302 /test3/ configurée correctement")
            else:
                results["warnings"].append(f"⚠ Redirection /test3/ vers: {location}")
        else:
            results["warnings"].append(f"⚠ Redirection /test3/: code {response.status_code} (attendu 302)")
    except Exception as e:
        results["errors"].append(f"✗ Test redirection: Exception - {str(e)}")

    return results

def test_file_upload(base_url):
    """Test de l'upload de fichiers"""
    results = {
        "success": [],
        "errors": [],
        "warnings": []
    }

    print("\n=== Tests d'upload de fichiers ===")
    
    try:
        files = {'file': ('test.txt', 'Contenu de test', 'text/plain')}
        response = requests.post(f"{base_url}/upload/", files=files)
        if response.status_code in [200, 201]:
            results["success"].append("✓ Upload de fichier réussi")
        else:
            results["warnings"].append(f"⚠ Upload: code {response.status_code}")
    except Exception as e:
        results["warnings"].append(f"⚠ Test upload: Exception - {str(e)}")

    return results

def display_results(all_results):
    """Affichage des résultats consolidés"""
    total_success = sum(len(r["success"]) for r in all_results)
    total_errors = sum(len(r["errors"]) for r in all_results)
    total_warnings = sum(len(r["warnings"]) for r in all_results)
    
    print("\n" + "="*60)
    print("📊 RÉSUMÉ DES TESTS")
    print("="*60)
    print(f"✅ Succès: {total_success}")
    print(f"⚠️  Avertissements: {total_warnings}")
    print(f"❌ Erreurs: {total_errors}")
    print(f"📈 Score: {total_success}/{total_success + total_errors + total_warnings}")
    
    print("\n" + "="*60)
    print("📋 DÉTAILS")
    print("="*60)
    
    for results in all_results:
        for success in results["success"]:
            print(success)
    
    if total_warnings > 0:
        print("\n⚠️  AVERTISSEMENTS:")
        for results in all_results:
            for warning in results["warnings"]:
                print(warning)
    
    if total_errors > 0:
        print("\n❌ ERREURS:")
        for results in all_results:
            for error in results["errors"]:
                print(error)

def main():
    """Fonction principale"""
    if len(sys.argv) > 1:
        SERVER_URL = sys.argv[1]
    else:
        SERVER_URL = "http://localhost:8080"
    
    print(f"🔗 Test du serveur: {SERVER_URL}")
    
    # Vérification de la connexion
    try:
        response = requests.get(SERVER_URL, timeout=5)
        print(f"✅ Serveur accessible (code: {response.status_code})")
    except Exception as e:
        print(f"❌ Impossible de se connecter au serveur: {e}")
        return
    
    # Exécution des tests
    all_results = []
    
    all_results.append(test_webserve_functionality(SERVER_URL))
    all_results.append(test_http_errors(SERVER_URL))
    all_results.append(test_redirections(SERVER_URL))
    all_results.append(test_file_upload(SERVER_URL))
    
    # Affichage des résultats
    display_results(all_results)

if __name__ == "__main__":
    main()
