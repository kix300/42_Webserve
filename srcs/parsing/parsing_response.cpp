/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_response.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 10:40:20 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/11 15:46:42 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cctype>
#include <algorithm>
#include <cstdlib>

//Parse le header + body si il ya (Quand body parser on doit le parser en chunk et timout si il est trop long)
ClientData &parsing_response(ClientData &client){

	std::string request(client.read_buff);
    std::string method, path, http;

	if (request.empty()) {
		throw std::runtime_error("400 Bad Request: Empty request");
	}
	size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        throw std::runtime_error("400 Bad Request: Malformed headers");
    }
    client.client_body = request.substr(header_end + 4);
    if ((long long)client.client_body.size() > client.server->getClientMaxBodySize()) {
        throw std::runtime_error("413 Payload Too Large: Request body size exceeds limit");
    }
    size_t first_line_end = request.find("\r\n");
    if (first_line_end == std::string::npos) {
        throw std::runtime_error("400 Bad Request: Invalid request line");
    }

    std::string request_line = request.substr(0, first_line_end);
    std::stringstream ss(request_line);
    ss >> method >> path >> http;

	std::cout << request << std::endl;
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		if (client.keep_alive)
			client.read_buff.clear();
		throw std::runtime_error("501 Not Implemented: Unsupported method");
	}

	if (path.empty() || http.empty())
	{
		throw std::runtime_error("400 Bad Request: Malformed request line");
	}

	if (DEBUG)
		std::cout << method  << " " << path << " " << http << std::endl;
    client.methode = method;
    client.path = path;
	return client;
};


static std::string reason_phrase(int code) {
	switch (code) {
		case 200: return "OK";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";
		default: return "OK";
	}
}

// Fonction pour combiner root et path en évitant les doublons
std::string combinePaths(const std::string& root, const std::string& path) {
	if (root.empty()) return path;
	if (path.empty()) return root;
	
	std::string cleanRoot = root;
	std::string cleanPath = path;
	
	if (!cleanRoot.empty() && cleanRoot[cleanRoot.length() - 1] == '/') {
		cleanRoot.erase(cleanRoot.length() - 1);
	}
	
	if (!cleanPath.empty() && cleanPath[0] != '/') {
		cleanPath = "/" + cleanPath;
	}
	
	if (!cleanPath.empty() && cleanPath != "/") {
		size_t lastSlash = cleanRoot.find_last_of('/');
		if (lastSlash != std::string::npos) {
			std::string rootLastSegment = cleanRoot.substr(lastSlash);
			if (cleanPath.length() > rootLastSegment.length() && 
				cleanPath.substr(0, rootLastSegment.length()) == rootLastSegment) {
				cleanPath = cleanPath.substr(rootLastSegment.length());
				if (cleanPath.empty()) cleanPath = "/";
			}
		}
	}
	
	return cleanRoot + cleanPath;
}

//dans create body, je doit regarder si le root existe, si le path de la methode existe dans root ou dans une location
//si c'est dans une location je doit gerer les options
//ensuite je creer le body et jenvois ca au client
std::string create_body(ClientData &client){


	struct stat sb;
	bool is_http_1_1 = (client.read_buff.find("HTTP/1.1") != std::string::npos);
	bool has_keepalive = (client.read_buff.find("Connection: keep-alive") != std::string::npos);
	bool has_close = (client.read_buff.find("Connection: close") != std::string::npos);

	// Règles HTTP/1.1 pour keep-alive (par défaut activé en 1.1)
	client.keep_alive = is_http_1_1 ? !has_close : has_keepalive;

	if (DEBUG){
		client.server->display();
	}

	
	if (client.server->getRoot() == "default")
		throw std::runtime_error("500 Internal Server Error: Bad root");
	std::string full_path;
	LocationData *locationserver = client.server->getLocation(client.path);
	full_path = client.server->getRoot() + client.path;
	if (client.path == "/")
		full_path = client.server->findFirstIndexFile();

	// si client path est dans une location alors full_path = location + params
	else if (locationserver != NULL){
		if (!locationserver->redirect.empty()) {
			std::string val = trim(locationserver->redirect);
			std::istringstream iss(val);
			std::string first;
			iss >> first;
			int code = 302; // défaut: redirection
			std::string rest;
			std::getline(iss, rest);
			rest = trim(rest);

			bool numeric = true;
			if (first.empty()) numeric = false;
			for (size_t i = 0; i < first.size(); ++i) {
				if (!std::isdigit(static_cast<unsigned char>(first[i]))) { numeric = false; break; }
			}
			if (numeric) {
				code = std::atoi(first.c_str());
			} else {
				rest = val;
			}

			if (code == 200) {
				// Réponse 200 avec texte/URL comme corps
				const std::string body = rest;
				client.write_buff =
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/plain\r\n"
					"Content-Length: " + tostring(body.size()) + "\r\n"
					"Connection: " + std::string(client.keep_alive ? "keep-alive" : "close") + "\r\n\r\n" +
					body;
				return "";
			}

			if (code >= 300 && code < 400) {
				// Redirection avec en-tête Location
				const std::string target = rest;
				client.write_buff =
					"HTTP/1.1 " + tostring(code) + " " + reason_phrase(code) + "\r\n"
					"Location: " + target + "\r\n"
					"Content-Length: 0\r\n"
					"Connection: " + std::string(client.keep_alive ? "keep-alive" : "close") + "\r\n\r\n";
				return "";
			}

			// Code non géré: fallback 302
			const std::string target = rest.empty() ? first : rest;
			client.write_buff =
				"HTTP/1.1 302 Found\r\n"
				"Location: " + target + "\r\n"
				"Content-Length: 0\r\n"
				"Connection: " + std::string(client.keep_alive ? "keep-alive" : "close") + "\r\n\r\n";
			return "";
		}
		//quel method est autorisée 
		//si autoindex fonctionne
		full_path = findFirstIndexFile(locationserver->index, combinePaths(locationserver->root, locationserver->path)); // utilise combinePaths pour éviter les doublons
		

	}
	if (!(stat(full_path.c_str(), &sb) == 0))
		throw std::runtime_error("404 Not Found: Bad path");

	std::string body = read_file(full_path);
	return body;
}

std::string findFirstIndexFile(std::string index, std::string root){
	std::stringstream ss(index);
	std::string index_file;
	struct stat sb;
	while (ss >> index_file) {
		std::string path = root + index_file;
		if (stat(path.c_str(), &sb) == 0)
			return path;
	}
	return "";
}

