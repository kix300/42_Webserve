/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_response.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 10:40:20 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/11 14:31:29 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cctype>
#include <algorithm>

//Parse le header + body si il ya (Quand body parser on doit le parser en chunk et timout si il est trop long)
ClientData &parsing_response(ClientData &client){

	std::string request(client.read_buff);
    std::string method, path, http;

	if (request.empty()) {
		throw std::runtime_error("400 Bad Request: Empty request");
	}
	if ((long long)request.size() > client.server->getClientMaxBodySize()) {
		throw std::runtime_error("413 Payload Too Large: Request size exceeds limit");
	}
    size_t first_line_end = request.find("\r\n");
    if (first_line_end == std::string::npos) {
        throw std::runtime_error("400 Bad Request: Invalid request line");
    }

    std::string request_line = request.substr(0, first_line_end);
    std::stringstream ss(request_line);
    ss >> method >> path >> http;

	if (method != "GET" && method != "POST" && method != "DELETE")
	{
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

//dans create body, je doit regarder si le root existe, si le path de la methode existe dans root ou dans une location
//si c'est dans une location je doit gerer les options
//ensuite je creer le body et jenvois ca au client

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
		full_path = findFirstIndexFile(locationserver->index,locationserver->root + locationserver->path); // a changer avec les index

		//si autoindex fonctionne
		//etc
	}

	std::cout << full_path << std::endl;
	if (stat(full_path.c_str(), &sb) != 0)
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

