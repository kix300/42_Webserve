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
        int flag_listen = 0;
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

            validateDirectivesOutsideServerBlock(line, line_number, in_server_block);

            if (line.compare(0, 6, "server") == 0 && (line[6] == ' ' || line[6] == '{'))
            {
                handleServerBlockStart(line, line_number, in_server_block, brace_level, server_count, current_server);
            }
            else if (in_server_block && line.compare(0, 8, "location") == 0)
            {
                handleLocationBlock(line, in_location_block, brace_level);
            }
            else if (line.find('}') != std::string::npos)
            {
                handleClosingBrace(line_number, in_server_block, in_location_block,
                                   brace_level, flag_listen, server_count,
                                   current_server, serverMap);
            }
            else if (in_server_block && brace_level >= 1)
            {
                processServerBlockContent(line, line_number, current_server,
                                          used_ports, flag_listen, in_location_block);
            }
        }

        if (in_server_block)
        {
            throw std::runtime_error("Unclosed server block");
        }

        if (brace_level != 0)
        {
            throw std::runtime_error("Unbalanced braces in configuration");
        }

        file.close();
        return serverMap;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        if (file.is_open())
        {
            file.close();
        }
        return std::map<int, Parsing_class>();
    }
}
