/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:34:40 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/11 15:49:46 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <sstream>

// Fonction pour décoder les caractères URL-encoded

void prepare_response(ClientData &client)
{
	if (client.methode == "GET")
		methode_get(client);

	else if (client.methode == "POST")
		methode_post(client);

	else if (client.methode == "DELETE")
		methode_delete(client);

	client.read_buff.clear();
}


void methode_get(ClientData& client){
	//on regarde si on a la methode allowed dans locationserver
	check_location_methode(client);
	std::string body = create_body(client);
	// Si create_body a déjà fabriqué la réponse complète (ex: redirect), ne pas écraser
	if (client.write_buff.size() == 0)
	{
		// Construction des en-têtes par défaut 200 OK
		client.write_buff =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " +
			tostring(body.size()) + "\r\n"
			"Connection: " +
			(client.keep_alive ? "keep-alive" : "close") + "\r\n"
			"\r\n" +
			body;
	}
}

void methode_post(ClientData& client){

	//on regarde si on a la methode allowed dans locationserver
	check_location_methode(client);
	//create_body back
	if (client.write_buff.size() == 0)
	{
		std::map<std::string, std::string> formData = parseFormData(client.client_body);

		std::string popupContent = "Donnees recues:\\n";
		for (std::map<std::string, std::string>::iterator it = formData.begin(); it != formData.end(); ++it) {
			popupContent += it->first + ": " + it->second + "\\n";
		}

		std::string body = "<html><body>"
			"<h1>Page avec popup</h1>"
			"<script>alert('" + popupContent + "');</script>"
			"</body></html>";

		client.write_buff =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " +
			tostring(body.size()) + "\r\n"
			"Connection: " +
			(client.keep_alive ? "keep-alive" : "close") + "\r\n"
			"\r\n" +
			body;
		client.read_buff.clear(); // au cas ou il y a un resend
	}
}

void methode_delete(ClientData& client){
	//on regarde si on a la methode allowed dans locationserver
	check_location_methode(client);

	if (client.write_buff.size() == 0)
	{
		// Vérifier que le chemin commence par "/delete/" pour autoriser uniquement les suppressions dans ce dossier
		if (client.path.find("/delete/") != 0) {
			throw std::runtime_error("403 Forbidden: Deletion not allowed outside /delete/ directory");
		}
		// Construire le chemin complet du fichier à supprimer
		std::string full_path = client.server->getRoot() + client.path;
		// Vérifier que le fichier existe
		struct stat file_stat;
		if (stat(full_path.c_str(), &file_stat) != 0) {
			throw std::runtime_error("404 Not Found: File does not exist");
		}
		// Ne pas autoriser la suppression de dossiers, uniquement les fichiers
		if (S_ISDIR(file_stat.st_mode)) {
			throw std::runtime_error("403 Forbidden: Cannot delete directories");
		}
		// Tenter de supprimer le fichier
		if (remove(full_path.c_str()) != 0) {
			throw std::runtime_error("500 Internal Server Error: Could not delete file");
		}
		// Créer la réponse de succès
		std::string body = "<html><body>"
			"<h1>File Deleted Successfully</h1>"
			"<p>The file '" + client.path + "' has been deleted.</p>"
			"<script>alert('Fichier supprime avec succes: " + client.path + "');</script>"
			"</body></html>";

		client.write_buff =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " +
			tostring(body.size()) + "\r\n"
			"Connection: " +
			(client.keep_alive ? "keep-alive" : "close") + "\r\n"
			"\r\n" +
			body;
		client.read_buff.clear();
	}
}


void check_location_methode(ClientData& client){
	LocationData *locationserver = client.server->getLocation(client.path);
	if (locationserver != NULL){
		int count = 0;
		for (std::vector<std::string>::iterator it = locationserver->allowed_methods.begin(); it < locationserver->allowed_methods.end(); it++){
			if (DEBUG)
				std::cout <<"methode allowed in this location : " << *it << std::endl;
			if (client.methode == *it)
				count++;
		}
		if (count == 0)
			throw std::runtime_error("405 Method Not Allowed: Bad Method");
	}
}
