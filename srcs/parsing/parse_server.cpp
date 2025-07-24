/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_server.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 11:25:02 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/24 16:02:55by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

// Fonction pour supprimer les espaces en début et fin de chaîne
std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first)
    {
        return "";
    }
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

// renvoi une map de parsing_class
//  on lui donne  une map x | server
//  puis on remplis chaque server en fonction du nombre de server
std::map<int, Parsing_class> count_nginx_servers(const std::string &filename, std::map<int, Parsing_class> serverMap)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier " << filename << std::endl;
        return serverMap;
    }

    int server_count = 0;
    int brace_level = 0;
    bool in_server_block = false;
    std::string line;
    Parsing_class current_server;

    while (getline(file, line))
    {
        line = trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        if (line.compare(0, 6, "server") == 0)
        {
            size_t open_brace_pos = line.find('{');
            if (open_brace_pos != std::string::npos)
            {
                in_server_block = true;
                brace_level = 1;
                server_count++;
                current_server.clear();
            }
        }
        // else if (line.find('{') != std::string::npos)
        // {
        //     if (in_server_block)
        //         brace_level++;
        // }
        // dans les brace on regarde ce quil nous faut
        else if (line.find('}') != std::string::npos)
        {
            if (in_server_block)
            {
                brace_level--;
                if (brace_level == 0)
                {
                    std::cout << server_count << std::endl;
                    serverMap[server_count] = current_server;
                    in_server_block = false;
                }
            }
        }
        if (brace_level == 1)
        {
            if (line.compare(0, 6, "listen") == 0)
            {
                std::string port_str = trim(line.substr(6));
                std::istringstream iss(port_str);
                int port;
                if (iss >> port)
                    current_server.setPort(port);
                if (DEBUG)
                    std::cout << " port : " << current_server.getPort() << std::endl;
            }
            else if (line.compare(0, 11, "server_name") == 0)
            {
                current_server.setName(trim(line.substr(11)));
            }
            else if (line.compare(0, 4, "root") == 0)
            {
                current_server.setRoot(trim(line.substr(4)));
            }
        }
    }

    file.close();
    return serverMap;
}
