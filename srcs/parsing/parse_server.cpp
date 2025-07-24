/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_server.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 11:25:02 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/23 12:03:22 by kduroux          ###   ########.fr       */
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

int count_nginx_servers(const std::string &filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier " << filename << std::endl;
        return -1;
    }

    int server_count = 0;
    int brace_level = 0;
    bool in_server_block = false;
    int port = 0;
    std::string line;

    while (getline(file, line))
    {
        line = trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        if (brace_level == 1)
        {
            //ici on creer un variable class et on remplis en fonction de compte
            if (line.compare(0, 6, "listen") == 0)
            {
                    std::string port_str = trim(line.substr(6 + 1));
                    if (!port_str.empty())
                    {
                        // Conversion de string à int (C++98)
                        std::istringstream iss(port_str);
                        iss >> port;
                        std::cout << "port : " << port << "\nserver_count : " << server_count << std::endl;
                    }
            }
        }
        if (line.compare(0, 6, "server") == 0)
        {
            size_t open_brace_pos = line.find('{');
            if (open_brace_pos != std::string::npos)
            {
                in_server_block = true;
                brace_level = 1;
                server_count++;
            }
        }
        else if (line.find('{') != std::string::npos)
        {
            if (in_server_block)
                brace_level++;
        }
        // dans les brace on regarde ce quil nous faut
        else if (line.find('}') != std::string::npos)
        {
            if (in_server_block)
            {
                brace_level--;
                if (brace_level == 0)
                    in_server_block = false;
            }
        }
    }

    file.close();
    return server_count;
}
