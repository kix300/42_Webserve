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
#include <map>
#include <sstream>

// Fonction pour décoder les caractères URL-encoded
std::string urlDecode(const std::string& str) {
	std::string decoded;
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '+') {
			decoded += ' ';
		} else if (str[i] == '%' && i + 2 < str.length()) {
			int value;
			std::istringstream is(str.substr(i + 1, 2));
			if (is >> std::hex >> value) {
				decoded += static_cast<char>(value);
				i += 2;
			} else {
				decoded += str[i];
			}
		} else {
			decoded += str[i];
		}
	}
	return decoded;
}

// Fonction pour parser les données de formulaire URL-encoded
std::map<std::string, std::string> parseFormData(const std::string& body) {
	std::map<std::string, std::string> data;
	std::istringstream stream(body);
	std::string pair;

	while (std::getline(stream, pair, '&')) {
		size_t pos = pair.find('=');
		if (pos != std::string::npos) {
			std::string key = urlDecode(pair.substr(0, pos));
			std::string value = urlDecode(pair.substr(pos + 1));
			data[key] = value;
		}
	}

	return data;
}

std::string read_file(const std::string &path) {
	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) != 0) {
		throw std::runtime_error("404 Not Found: File does not exist");
	}

	//autoindex ?!
	if (S_ISDIR(file_stat.st_mode)) {
		throw std::runtime_error("404 Not Found: Path is a directory");
	}

	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("404 Not Found: Could not open file");
	}

	std::string content;
	file.seekg(0, std::ios::end);
	content.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	content.assign((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());

	return content;
}

void prepare_response(ClientData &client)
{
	// il faut gerer la methode POST et DELETE
	// construction du body (peut aussi construire toute la réponse pour redirections)
	// regarder si on est uatoriser a faire la methode dans une location

	if (client.methode == "GET")
	{
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
	//POST c'est on recoit un body/header avec des information a recuperer exemple:
	// on a deja check la taille et recuperer la reponse maintenant il faut afficher cette reponse
	else if (client.methode == "POST"){

		//on regarde si on a la methode allowed dans locationserver
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
				throw std::runtime_error("405 Methode Not Allowed: Bad Methode");
		}
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
	// on trouve et tej le fichier 
	else if (client.methode == "DELETE"){
		//on regarde si on a la methode allowed dans locationserver
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
		
		if (client.write_buff.size() == 0)
		{
			// Vérifier que le chemin commence par "/delete/" pour autoriser uniquement les suppressions dans ce dossier
			if (client.path.find("/delete/") != 0) {
				throw std::runtime_error("403 Forbidden: Deletion not allowed outside /delete/ directory");
			}
			
			// Construire le chemin complet du fichier à supprimer
			std::string full_path = "www" + client.path; // Supposant que www est le dossier racine
			
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

	size_t request_end = client.read_buff.find("\r\n\r\n");
	if (request_end != std::string::npos)
	{
		client.read_buff.erase(0, request_end + 4);
	}
	else
{
		client.read_buff.clear();
	}
}
