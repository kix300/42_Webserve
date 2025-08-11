/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:34:40 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/11 13:40:13 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cstdio>


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

void prepare_response(ClientData &client)
{
    // construction du body
    // gestion de root et location
    std::string body = create_body(client);
    // Construction des en-têtes
    client.write_buff =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " +
        tostring(body.size()) + "\r\n"
                                "Connection: " +
        (client.keep_alive ? "keep-alive" : "close") + "\r\n"
                                                       "\r\n" +
        body;

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
