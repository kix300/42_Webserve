/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_response.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 10:40:20 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/22 14:30:32 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cctype>
#include <algorithm>
#include <cstdlib>

//parsing_response : Parse le header + body si il ya (Quand body parser on doit le parser en chunk et timout si il est trop long)
ClientData &parsing_response(ClientData &client){

	std::string request(client.read_buff);
	std::cout << request << std::endl;
    std::string method, path, http;

	if (request.empty()) {
		throw std::runtime_error("400 Bad Request: Empty request");
	}
	size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        throw std::runtime_error("400 Bad Request: Malformed headers");
	}

	client.client_body = request.substr(header_end + 4);
	std::string headers = request.substr(0, header_end);

	size_t content_length_pos = headers.find("Content-Length:");
	if (content_length_pos == std::string::npos) {
		content_length_pos = headers.find("content-length:");
	}

	long content_length = -1;
	if (content_length_pos != std::string::npos) {
		size_t value_start = headers.find(':', content_length_pos) + 1;
		size_t value_end = headers.find('\r', value_start);
		if (value_end == std::string::npos) {
			value_end = headers.find('\n', value_start);
		}
		if (value_end != std::string::npos) {
			std::string content_length_str = headers.substr(value_start, value_end - value_start);
			size_t start = content_length_str.find_first_not_of(" \t");
			size_t end = content_length_str.find_last_not_of(" \t");
			if (start != std::string::npos && end != std::string::npos) {
				content_length_str = content_length_str.substr(start, end - start + 1);
				content_length = std::atoll(content_length_str.c_str());
			}
		}
	}

	if (content_length >= 0 && (long)client.client_body.size() != content_length) {
		throw std::runtime_error("413 Payload Too Large: Request body size exceeds limit");
	}

	long size_to_check = (content_length >= 0) ? content_length : (long)client.client_body.size();
	if (size_to_check > client.server->getClientMaxBodySize()) {
		if (!(client.read_buff.find(".png") != std::string::npos)){
			throw std::runtime_error("413 Payload Too Large: Request body size exceeds limit");
		}
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
		if (client.keep_alive)
			client.read_buff.clear();
		throw std::runtime_error("405 Not Implemented: Unsupported method");
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
}


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

// combinePaths:  fonction pour combiner root et path en evitant les doublons
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

// locationinserver : on  parse la location pour trouver si redirection, cgi, etc on return un path
std::string locationinserver(LocationData *locationserver, ClientData client, std::string full_path){
	if (!locationserver->redirect.empty()) {
		std::string val = trim(locationserver->redirect);
		std::istringstream iss(val);
		std::string first;
		iss >> first;
		int code = 302;
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
			const std::string target = rest;
			client.write_buff =
				"HTTP/1.1 " + tostring(code) + " " + reason_phrase(code) + "\r\n"
				"Location: " + target + "\r\n"
				"Content-Length: 0\r\n"
				"Connection: " + std::string(client.keep_alive ? "keep-alive" : "close") + "\r\n\r\n";
			if(DEBUG)
				std::cout << client.write_buff << std::endl;
			return "";
		}

		const std::string target = rest.empty() ? first : rest;
		client.write_buff =
			"HTTP/1.1 302 Found\r\n"
			"Location: " + target + "\r\n"
			"Content-Length: 0\r\n"
			"Connection: " + std::string(client.keep_alive ? "keep-alive" : "close") + "\r\n\r\n";
		return "";
	}
	full_path = findFirstIndexFile(locationserver->index, combinePaths(locationserver->root, locationserver->path));

	if (full_path.empty()) {
		std::string directory_path = combinePaths(locationserver->root, locationserver->path);
		struct stat dir_stat;
		if (stat(directory_path.c_str(), &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode)) {
			if (locationserver->autoindex) {
				std::string body = generateDirectoryListing(directory_path, client.path);
				return body;
			} else {
				throw std::runtime_error("403 Forbidden: Directory listing disabled");
			}
		} else {
			throw std::runtime_error("404 Not Found: No index file found");
		}
	}

	return full_path;
}


//dans create body, je doit regarder si le root existe, si le path de la methode existe dans root ou dans une location
//si c'est dans une location je doit gerer les options
//ensuite je creer le body et jenvois ca au client
std::string create_body(ClientData &client){
	struct stat sb;
	bool is_http_1_1 = (client.read_buff.find("HTTP/1.1") != std::string::npos);
	bool has_keepalive = (client.read_buff.find("Connection: keep-alive") != std::string::npos);
	bool has_close = (client.read_buff.find("Connection: close") != std::string::npos);

	client.keep_alive = is_http_1_1 ? !has_close : has_keepalive;

	if (DEBUG){
		client.server->display();
	}
	if (client.server->getRoot() == "default")
		throw std::runtime_error("500 Internal Server Error: Bad root");
	std::string full_path;
	LocationData *locationserver = client.server->getLocation(client.path);

	if (locationserver != NULL && !locationserver->root.empty()) {
		full_path = locationserver->root + client.path;
	} else {
		full_path = client.server->getRoot() + client.path;
	}

	// if (client.path == "/")
	// 	full_path = client.server->findFirstIndexFile();
	if (locationserver != NULL && isCGIRequest(client.path, locationserver)) {
		return executeCGI(client, full_path, locationserver);
	}


	if (locationserver != NULL){
		if (!locationserver->redirect.empty()) {
			std::string val = trim(locationserver->redirect);
			std::istringstream iss(val);
			std::string first;
			iss >> first;
			int code = 302;
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
				const std::string target = rest;
				client.write_buff =
					"HTTP/1.1 " + tostring(code) + " " + reason_phrase(code) + "\r\n"
					"Location: " + target + "\r\n"
					"Content-Length: 0\r\n"
					"Connection: " + std::string(client.keep_alive ? "keep-alive" : "close") + "\r\n\r\n";
				return "";
			}

			const std::string target = rest.empty() ? first : rest;
			client.write_buff =
				"HTTP/1.1 302 Found\r\n"
				"Location: " + target + "\r\n"
				"Content-Length: 0\r\n"
				"Connection: " + std::string(client.keep_alive ? "keep-alive" : "close") + "\r\n\r\n";
			return "";
		}
		full_path = findFirstIndexFile(locationserver->index, combinePaths(locationserver->root, locationserver->path));

		if (full_path.empty()) {
			std::string directory_path = combinePaths(locationserver->root, locationserver->path);
			struct stat dir_stat;
			if (stat(directory_path.c_str(), &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode)) {
				if (locationserver->autoindex) {
					std::string body = generateDirectoryListing(directory_path, client.path);
					return body;
				} else {
					throw std::runtime_error("403 Forbidden: Directory listing disabled");
				}
			} else {
				throw std::runtime_error("404 Not Found: No index file found");
			}
		}
	} else {
		if (client.path == "/") {
			full_path = client.server->findFirstIndexFile();
		} else {
			full_path = client.server->getRoot() + client.path;
		}
		struct stat dir_stat;
		if (stat(full_path.c_str(), &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode)) {
			std::string index_path = findFirstIndexFile(client.server->getIndex(), full_path + "/");
			if (index_path.empty()) {
				throw std::runtime_error("403 Forbidden: Directory listing disabled");
			}
			full_path = index_path;
		}
	}

	if (!(stat(full_path.c_str(), &sb) == 0))
		throw std::runtime_error("404 Not Found: Bad path");

	std::string body = read_file_or_directory(full_path, client.path, false);
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

