/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:34:40 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/22 14:34:21 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <sstream>

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

// methode_get : on traite les requetes GET en regardant les CGI egalement on regarde si on est dans une location
// on creer un body
// on check si le body est un cgi
// ou on envoi
void methode_get(ClientData& client){
	check_location_methode(client);
	std::string body = create_body(client);

	if (client.write_buff.size() == 0) {
		if (body.find("Content-Type:") != std::string::npos || body.find("Status:") != std::string::npos) {
			buildHTTPResponse(client, body);
		} else {
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
}

// methode_post : on traite les requetes POST en checkant les CGI, upload et formulaire
void methode_post(ClientData& client){
	check_location_methode(client);

	LocationData *locationserver = client.server->getLocation(client.path);
	if (locationserver && isCGIRequest(client.path, locationserver)) {
		std::string full_path = client.server->getRoot() + client.path;
		if (locationserver) {
			if (!locationserver->root.empty()) {
				full_path = locationserver->root + client.path;
			}
		}

		std::string cgi_output = executeCGI(client, full_path, locationserver);
		buildHTTPResponse(client, cgi_output);
		return;
	}

	if (client.write_buff.size() == 0)
	{
		if (isMultipartFormData(client.read_buff)) {
			if (handleFileUpload(client)) {
				std::string body_content = client.client_body;
				size_t headers_end = body_content.find("\r\n\r\n");
				std::string filename = "unknown";
				if (headers_end != std::string::npos) {
					std::string headers = body_content.substr(0, headers_end);
					filename = extractFileName(headers);
					if (filename.empty()) filename = "unknown";
				}

				std::string body = "<html><body>"
					"<h1>Upload Réussi!</h1>"
					"<p>Le fichier a été uploadé avec succès.</p>"
					"<p>Nom du fichier: " + filename + "</p>"
					"<p>Taille: " + tostring(client.client_body.size()) + " bytes</p>"
					"<script>alert('Fichier uploadé avec succès: " + filename + "');</script>"
					"<a href='/upload/'>Voir les fichiers uploadés</a><br>"
					"<a href='/methode/'>Retour à l'interface</a>"
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
			}
		} else {
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
		}
		client.read_buff.clear(); // au cas ou il y a un resend
	}
}

// methode_delete : on cherche si le fichier a delete existe et est au bon endroit ( /delete/)
// puis on essaye de le delete et on creer un body pour dire que le ficher a bien etet suppr
void methode_delete(ClientData& client){
	check_location_methode(client);

	if (client.write_buff.size() == 0)
	{

		if (client.path.find("/delete/") != 0) {
			throw std::runtime_error("403 Forbidden: Deletion not allowed outside /delete/ directory");
		}
		std::string full_path = client.server->getRoot() + client.path;
		struct stat file_stat;
		if (stat(full_path.c_str(), &file_stat) != 0) {
			throw std::runtime_error("404 Not Found: File does not exist");
		}
		if (S_ISDIR(file_stat.st_mode)) {
			throw std::runtime_error("403 Forbidden: Cannot delete directories");
		}
		if (remove(full_path.c_str()) != 0) {
			throw std::runtime_error("500 Internal Server Error: Could not delete file");
		}
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

// check_location_methode : on verifie si la location est autoriser dans la location
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
