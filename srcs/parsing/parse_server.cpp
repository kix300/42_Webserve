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

bool isValidPort(const std::string& portStr) {
    if (portStr.empty()) return false;
    
    for (size_t i = 0; i < portStr.size(); ++i) {
        if (!isdigit(portStr[i])) return false;
    }
    
    int port = atoi(portStr.c_str());
    return (port > 0 && port <= 65535);
}

bool directoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) return false;
    return (info.st_mode & S_IFDIR);
}

// renvoi une map de parsing_class
//  on lui donne  une map x | server
//  puis on remplis chaque server en fonction du nombre de server
std::map<int, Parsing_class> count_nginx_servers(const std::string &filename, std::map<int, Parsing_class> serverMap) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return serverMap;
    }

    int server_count = 0;
    int brace_level = 0;
    int line_number = 0;
    bool in_server_block = false;
    bool in_location_block = false;
    std::string line;
    Parsing_class current_server;
    std::set<int> used_ports;

    while (getline(file, line)) {
        line_number++;
        line = trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        if (!in_server_block && 
            (line.find("listen") == 0 ||
             line.find("server_name") == 0 ||
             line.find("root") == 0 ||
             line.find("location") == 0 ||
             line.find("error_page") == 0)) {
            std::cerr << "Error (line " << line_number << "): Directive outside server block" << std::endl;
            return std::map<int, Parsing_class>();
        }

        if (line.compare(0, 6, "server") == 0 && (line[6] == ' ' || line[6] == '{')) {
            size_t open_brace_pos = line.find('{');
            if (open_brace_pos != std::string::npos) {
                if (in_server_block) {
                    std::cerr << "Error (line " << line_number << "): Nested server blocks are not allowed" << std::endl;
                    if (file.is_open())
                        file.close();
                    return std::map<int, Parsing_class>();
                }

                std::string between = trim(line.substr(6, open_brace_pos - 6));
                if (!between.empty()) {
                    std::cerr << "Error (line " << line_number << "): Unexpected tokens after 'server' directive" << std::endl;
                    return std::map<int, Parsing_class>();
                }

                in_server_block = true;
                brace_level = 1;
                server_count++;
                current_server.clear();
            }
        }
        else if (in_server_block && line.compare(0, 8, "location") == 0) {
            size_t open_brace_pos = line.find('{');
            if (open_brace_pos != std::string::npos) {
                in_location_block = true;
                brace_level++;
            }
        }
        else if (line.find('}') != std::string::npos) {
            if (in_server_block) {
                brace_level--;
                if (brace_level == 0) {
                    if (current_server.getPort() == 0) {
                        std::cerr << "Error (line " << line_number << "): Missing 'listen' directive in server block" << std::endl;
                        return std::map<int, Parsing_class>();
                    }

                    current_server.setId(server_count);
                    serverMap[server_count] = current_server;
                    in_server_block = false;
                    in_location_block = false;
                } else if (brace_level < 0) {
                    std::cerr << "Error (line " << line_number << "): Unexpected '}'" << std::endl;
                    return std::map<int, Parsing_class>();
                }
            } else {
                std::cerr << "Error (line " << line_number << "): Unexpected '}' without matching '{'" << std::endl;
                return std::map<int, Parsing_class>();
            }
        }
        else if (in_server_block && brace_level >= 1) {
            if (line.compare(0, 6, "listen") == 0 && (line[6] == ' ' || line[6] == '\t')) {
                if (in_location_block) {
                    std::cerr << "Error (line " << line_number << "): 'listen' directive not allowed in location block" << std::endl;
                    return std::map<int, Parsing_class>();
                }

                std::string port_str = trim(line.substr(6));
                size_t semicolon = port_str.find(';');
                if (semicolon != std::string::npos) {
                    port_str = trim(port_str.substr(0, semicolon));
                } else {
                    std::cerr << "Error (line " << line_number << "): Missing ';' at end of listen directive" << std::endl;
                    return std::map<int, Parsing_class>();
                }

                if (!isValidPort(port_str)) {
                    std::cerr << "Error (line " << line_number << "): Invalid port number '" << port_str << "'" << std::endl;
                    return std::map<int, Parsing_class>();
                }

                std::istringstream iss(port_str);
                int port;
                if (iss >> port) {
                    if (used_ports.find(port) != used_ports.end()) {
                        std::cerr << "Error (line " << line_number << "): Duplicate port " << port << std::endl;
                        return std::map<int, Parsing_class>();
                    }
                    used_ports.insert(port);
                    current_server.setPort(port);
                }
            }
          
        }
    }

    if (in_server_block) {
        std::cerr << "Error: Unclosed server block" << std::endl;
        return std::map<int, Parsing_class>();
    }

    if (brace_level != 0) {
        std::cerr << "Error: Unbalanced braces in configuration" << std::endl;
        return std::map<int, Parsing_class>();
    }

    file.close();
    return serverMap;
}
