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

bool isValidPort(const std::string &portStr)
{
    if (portStr.empty())
        return false;

    for (size_t i = 0; i < portStr.size(); ++i)
    {
        if (!isdigit(portStr[i]))
            return false;
    }

    int port = atoi(portStr.c_str());
    return (port > 0 && port <= 65535);
}

template <typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

bool directoryExists(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR);
}

// renvoi une map de parsing_class
//  on lui donne  une map x | server
//  puis on remplis chaque server en fonction du nombre de server
std::map<int, Parsing_class> count_nginx_servers(const std::string &filename, std::map<int, Parsing_class> serverMap)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return serverMap;
    }

    try
    {
        int server_count = 0;
        int brace_level = 0;
        int line_number = 0;
        bool in_server_block = false;
        bool in_location_block = false;
        std::string line;
        Parsing_class current_server;
        std::set<int> used_ports;

        while (getline(file, line))
        {
            line_number++;
            line = trim(line);

            if (line.empty() || line[0] == '#')
                continue;

            if (!in_server_block &&
                (line.find("listen") == 0 ||
                 line.find("server_name") == 0 ||
                 line.find("root") == 0 ||
                 line.find("location") == 0 ||
                 line.find("error_page") == 0))
            {
               throw std::runtime_error("Directive outside server block (line " + toString(line_number) + ")");
            }

            if (line.compare(0, 6, "server") == 0 && (line[6] == ' ' || line[6] == '{'))
            {
                size_t open_brace_pos = line.find('{');
                if (open_brace_pos != std::string::npos)
                {
                    if (in_server_block){
                        throw std::runtime_error("Nested server blocks are not allowed (line " + toString(line_number) + ")");
                    }

                    std::string between = trim(line.substr(6, open_brace_pos - 6));
                    if (!between.empty()){
                        throw std::runtime_error("Unexpected tokens after 'server' directive (line " + toString(line_number) + ")");
                    }

                    in_server_block = true;
                    brace_level = 1;
                    server_count++;
                    current_server.clear();
                }
            }
            else if (in_server_block && line.compare(0, 8, "location") == 0)
            {
                size_t open_brace_pos = line.find('{');
                if (open_brace_pos != std::string::npos)
                {
                    in_location_block = true;
                    brace_level++;
                }
            }
            else if (line.find('}') != std::string::npos)
            {
                if (in_server_block)
                {
                    brace_level--;
                    if (brace_level == 0)
                    {
                        if (current_server.getPort() == 0){
                            throw std::runtime_error("Missing 'listen' directive in server block (line " + toString(line_number) + ")");
                        }

                        current_server.setId(server_count);
                        serverMap[server_count] = current_server;
                        in_server_block = false;
                        in_location_block = false;
                    }
                    else if (brace_level < 0){
                        throw std::runtime_error("Unexpected '}' (line " + toString(line_number) + ")");
                    }
                }
                else{
                    throw std::runtime_error("Unexpected '}' without matching '{' (line " + toString(line_number) + ")");
                }
            }
            else if (in_server_block && brace_level >= 1)
            {
                if (line.compare(0, 6, "listen") == 0 && (line[6] == ' ' || line[6] == '\t'))
                {
                    if (in_location_block){
                        throw std::runtime_error("listen' directive not allowed in location block (line " + toString(line_number) + ")");
                    }

                    std::string port_str = trim(line.substr(6));
                    size_t semicolon = port_str.find(';');
                    if (semicolon != std::string::npos)
                    {
                        port_str = trim(port_str.substr(0, semicolon));
                    }
                    else{
                        throw std::runtime_error("Missing ';' at end of listen directive (line " + toString(line_number) + ")");
                    }

                    if (!isValidPort(port_str)){
                        throw std::runtime_error("Invalid port number (line " + toString(line_number) + ")");
                    }

                    std::istringstream iss(port_str);
                    int port;
                    if (iss >> port)
                    {
                        if (used_ports.find(port) != used_ports.end()){
                            throw std::runtime_error("Duplicate port (line " + toString(line_number) + ")");
                        }
                        used_ports.insert(port);
                        current_server.setPort(port);
                    }
                }
            }
        }

        if (in_server_block){
            throw std::runtime_error("Unclosed server block");
        }

        if (brace_level != 0){
            throw std::runtime_error("Unbalanced braces in configuration");
        }

        file.close();
        return serverMap;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (file.is_open()) {
            file.close();
        }
        return std::map<int, Parsing_class>(); // Retourne une map vide en cas d'erreur
    }
}
