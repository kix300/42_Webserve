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
#include <stdexcept>

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

	if (method != "GETA" && method != "POST" && method != "DELETE")
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
