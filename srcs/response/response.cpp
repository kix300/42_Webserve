/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:34:40 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/24 11:43:12 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cstdio>

template <typename T>
std::string to_string(const T& value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string read_file(const std::string& path) {
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file) return "";

	std::string content;
	file.seekg(0, std::ios::end);
	content.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	content.assign((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());
	return content;
}

void prepare_response(ClientData& client) {
	//ici on regarde la requet du navigateur
	bool is_http_1_1 = (client.read_buff.find("HTTP/1.1") != std::string::npos);
	bool has_keepalive = (client.read_buff.find("Connection: keep-alive") != std::string::npos);
	bool has_close = (client.read_buff.find("Connection: close") != std::string::npos);

	// Règles HTTP/1.1 pour keep-alive (par défaut activé en 1.1)
	client.keep_alive = is_http_1_1 ? !has_close : has_keepalive;

	size_t start = client.read_buff.find(' ');
	size_t end = client.read_buff.find(' ', start + 1);

	std::string path = client.read_buff.substr(start + 1, end - start - 1);
	if (path == "/") path = "/index.html";
	std::string full_path = "www" + path; // Vos fichiers sont dans un dossier 'static'
	std::string body = read_file(full_path);

	// Construction des en-têtes
	client.write_buff = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: " + to_string(body.size()) + "\r\n"
		"Connection: " + (client.keep_alive ? "keep-alive" : "close") + "\r\n"
		"\r\n"
		+ body;

	size_t request_end = client.read_buff.find("\r\n\r\n");
	if (request_end != std::string::npos) {
		client.read_buff.erase(0, request_end + 4);
	} else {
		client.read_buff.clear();
	}
}


void error_response(ClientData& client, std::string error);
