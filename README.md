# Webserve

Ce projet est une réimplémentation d'un serveur HTTP en C++98. Il est capable de servir des sites web statiques et dynamiques, en se basant sur un fichier de configuration détaillé. Ce projet a été réalisé dans le cadre du cursus de l'école 42.

## Fonctionnalités

*   **Serveurs virtuels** : Gestion de plusieurs serveurs sur différents ports à partir d'un seul fichier de configuration.
*   **Méthodes HTTP** : Prise en charge des requêtes `GET`, `POST` et `DELETE`.
*   **Contenu statique** : Service de fichiers statiques (HTML, CSS, images, etc.).
*   **CGI** : Exécution de scripts CGI (testé avec Python et PHP) pour générer du contenu dynamique.
*   **Gestion des erreurs** : Pages d'erreur personnalisables pour les codes HTTP courants (403, 404, 405, etc.).
*   **Redirection** : Configuration de redirections HTTP.
*   **Autoindex** : Affichage du contenu d'un répertoire si aucun fichier d'index n'est trouvé (configurable par `location`).
*   **Upload de fichiers** : Possibilité de téléverser des fichiers sur le serveur.
*   **Timeout CGI** : Gestion du timeout pour les scripts CGI afin d'éviter les processus bloquants.

## Prérequis

*   Un compilateur C++ (comme `c++` ou `g++`) supportant la norme C++98.
*   `make` pour la compilation du projet.

## Compilation

Pour compiler le projet, exécutez la commande suivante à la racine du répertoire :

```bash
make
```

Cela générera un exécutable nommé `webserv`.

## Utilisation

Pour démarrer le serveur, vous devez spécifier un fichier de configuration :

```bash
./webserv [chemin/vers/votre/fichier.conf]
```

Un fichier de configuration d'exemple est fourni :

```bash
./webserv conf/exemple.conf
```

## Configuration

Le serveur est entièrement configurable via un fichier `.conf`. La syntaxe est inspirée de Nginx. Voici un exemple de base :

```nginx
# Serveur principal écoutant sur le port 8080
server {
    listen 8080;
    server_name example.com;
    root www/;
    index index.html;

    # Pages d'erreur personnalisées
    error_page 404 www/error/404.html;

    # Servir des fichiers statiques avec autoindex activé
    location /files/ {
        root www/;
        autoindex on;
        allowed_methods GET;
    }

    # Gérer les scripts CGI en Python et PHP
    location /cgi/ {
        root www/;
        cgi_extension .py /usr/bin/python;
        cgi_extension .php /usr/bin/php;
        cgi_timeout 5; # Timeout de 5 secondes
        allowed_methods GET POST;
    }

    # Gérer l'upload de fichiers
    location /upload/ {
        root www/;
        allowed_methods POST;
    }
}

# Un autre serveur virtuel sur un autre port
server {
    listen 8082;
    root www/kix/;
    index index.html;
}
```

### Directives principales

*   `listen` : Le port d'écoute.
*   `server_name` : Nom(s) du serveur.
*   `root` : Le répertoire racine pour les requêtes.
*   `index` : Fichier à servir par défaut.
*   `error_page` : Spécifie un fichier pour une page d'erreur donnée.
*   `client_body_buffer_size` : Taille maximale du corps de la requête client.
*   `location` : Définit des règles pour des URI spécifiques.
    *   `allowed_methods` : Méthodes HTTP autorisées.
    *   `autoindex` : `on` ou `off`.
    *   `cgi_extension` : Associe une extension de fichier à un interpréteur CGI.
    *   `cgi_timeout` : Temps maximum d'exécution pour un script CGI.
    *   `return` : Effectue une redirection.

## Auteurs

*   **ozen** - *(merci de compléter avec votre nom/login)*
