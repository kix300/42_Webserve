/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_response.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 10:40:20 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/29 11:45:24 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

ClientData &parsing_response(ClientData &client){

	std::string request(client.read_buff);
    std::string method, path, http;

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

	
	std::string full_path = client.server->getRoot() + client.path;
	if (client.path == "/") full_path = client.server->findFirstIndexFile();
	std::cout << full_path << std::endl;
	if (stat(full_path.c_str(), &sb) != 0)
		throw std::runtime_error("404 Not Found: Bad path");


	std::string body = read_file(full_path);
	return body;
}
