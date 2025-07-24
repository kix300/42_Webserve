# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_errors.py                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/07 20:22:16 by kduroux           #+#    #+#              #
#    Updated: 2025/07/07 20:22:29 by kduroux          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import requests
import json

def test_http_errors(base_url):
    results = {
        "success": [],
        "errors": [],
        "not_tested": []
    }

    print(f"\nDébut des tests sur le serveur: {base_url}\n")

    ## Tests pour les erreurs 4xx (400-418)
    print("=== Tests pour erreurs 4xx (400-418) ===")
    
    # Dictionnaire des méthodes pour générer chaque erreur
    error_4xx_tests = {
        400: ("POST", "/bad-request", {"invalid": "json"}, {'Content-Type': 'application/json'}, "Corps JSON malformé"),
        401: ("GET", "/protected", None, None, "Accès non autorisé"),
        402: ("POST", "/payment-required", None, None, "Paiement requis"),
        403: ("GET", "/forbidden", None, None, "Accès interdit"),
        404: ("GET", "/non-existent-route", None, None, "Route inexistante"),
        405: ("POST", "/", None, None, "Méthode non autorisée"),
        406: ("GET", "/not-acceptable", None, {'Accept': 'application/invalid'}, "Non acceptable"),
        407: ("GET", "/proxy-auth", None, None, "Authentification proxy requise"),
        408: ("GET", "/timeout", None, None, "Délai expiré"),
        409: ("PUT", "/conflict", {"version": "old"}, None, "Conflit de version"),
        410: ("GET", "/gone", None, None, "Ressource supprimée"),
        411: ("POST", "/length-required", None, None, "Longueur requise"),
        412: ("GET", "/precondition-failed", None, {'If-Match': 'invalid-etag'}, "Précondition échouée"),
        413: ("POST", "/too-large", {"data": "a"*10000000}, None, "Charge utile trop grande"),
        414: ("GET", "/too-long?" + "a"*10000, None, None, "URI trop longue"),
        415: ("POST", "/unsupported-media", None, {'Content-Type': 'invalid/type'}, "Type de média non supporté"),
        416: ("GET", "/range-not-satisfiable", None, {'Range': 'bytes=1000-2000'}, "Plage non satisfaisable"),
        417: ("GET", "/expectation-failed", None, {'Expect': '100-continue'}, "Attente échouée"),
        418: ("GET", "/teapot", None, None, "Je suis une théière")
    }

    for code, (method, endpoint, data, headers, description) in error_4xx_tests.items():
        try:
            if method == "GET":
                response = requests.get(f"{base_url}{endpoint}", headers=headers)
            elif method == "POST":
                response = requests.post(f"{base_url}{endpoint}", json=data, headers=headers)
            elif method == "PUT":
                response = requests.put(f"{base_url}{endpoint}", json=data, headers=headers)

            if response.status_code == code:
                results["success"].append(f"Test {code}: {description} - réussi")
            else:
                results["errors"].append(f"Test {code}: {description} - échoué (code: {response.status_code})")
        except Exception as e:
            results["errors"].append(f"Test {code}: Exception - {str(e)}")

    ## Tests pour les erreurs 5xx (500-511)
    print("\n=== Tests pour erreurs 5xx (500-511) ===")
    
    error_5xx_tests = {
        500: ("GET", "/server-error", None, None, "Erreur interne du serveur"),
        501: ("PATCH", "/not-implemented", None, None, "Non implémenté"),
        502: ("GET", "/bad-gateway", None, None, "Mauvaise passerelle"),
        503: ("GET", "/unavailable", None, None, "Service indisponible"),
        504: ("GET", "/gateway-timeout", None, None, "Délai de passerelle expiré"),
        505: ("GET", "/http-version", None, {'HTTP-Version': 'HTTP/2.0'}, "Version HTTP non supportée"),
        506: ("GET", "/variant-negotiates", None, None, "Variante négociée"),
        507: ("GET", "/insufficient-storage", None, None, "Stockage insuffisant"),
        508: ("GET", "/loop-detected", None, None, "Boucle détectée"),
        510: ("GET", "/not-extended", None, None, "Non étendu"),
        511: ("GET", "/network-auth", None, None, "Authentification réseau requise")
    }

    for code, (method, endpoint, data, headers, description) in error_5xx_tests.items():
        try:
            if method == "GET":
                response = requests.get(f"{base_url}{endpoint}", headers=headers)
            elif method == "POST":
                response = requests.post(f"{base_url}{endpoint}", json=data, headers=headers)
            elif method == "PATCH":
                response = requests.patch(f"{base_url}{endpoint}", json=data, headers=headers)

            if response.status_code == code:
                results["success"].append(f"Test {code}: {description} - réussi")
            else:
                results["errors"].append(f"Test {code}: {description} - échoué (code: {response.status_code})")
        except Exception as e:
            results["errors"].append(f"Test {code}: Exception - {str(e)}")

    # Affichage des résultats
    print("\n=== Résultats des tests ===")
    print(f"\nTests réussis ({len(results['success'])}):")
    for success in results["success"]:
        print(f"✓ {success}")
    
    print(f"\nErreurs ({len(results['errors'])}):")
    for error in results["errors"]:
        print(f"✗ {error}")
    
    print(f"\nNon testés ({len(results['not_tested'])}):")
    for not_tested in results["not_tested"]:
        print(f"? {not_tested}")
    
    return results

if __name__ == "__main__":
    # Remplacez cette URL par l'URL de votre serveur
    SERVER_URL = "http://localhost:8080"
    test_http_errors(SERVER_URL)
