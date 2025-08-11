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

std::string read_file(const std::string &path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        return "";

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
    else if (client.methode == "POST"){

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
