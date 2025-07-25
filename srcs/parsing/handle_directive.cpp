/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_directive.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 14:04:38 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/25 14:15:37 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

template <typename T> std::string toString(const T &value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void handleListenDirective(const std::string &line, int line_number, Parsing_class &current_server, std::set<int> &used_ports, int &flag_listen)
{
    if (flag_listen == 0)
    {
        flag_listen = 1;
        std::string port_str = trim(line.substr(6));
        size_t semicolon = port_str.find(';');
        if (semicolon != std::string::npos)
        {
            port_str = trim(port_str.substr(0, semicolon));
        }
        else
        {
            throw std::runtime_error("Missing ';' at end of listen directive (line " + toString(line_number) + ")");
        }

        if (!isValidPort(port_str))
        {
            throw std::runtime_error("Invalid port number (line " + toString(line_number) + ")");
        }

        std::istringstream iss(port_str);
        int port;
        if (iss >> port)
        {
            if (used_ports.find(port) != used_ports.end())
            {
                throw std::runtime_error("Duplicate port (line " + toString(line_number) + ")");
            }
            used_ports.insert(port);
            current_server.setPort(port);
        }
    }
}

void handleServerNameDirective(const std::string &line, int line_number, Parsing_class &current_server)
{
    std::string name_str = trim(line.substr(11));
    size_t semicolon = name_str.find(';');
    if (semicolon != std::string::npos)
    {
        name_str = trim(name_str.substr(0, semicolon));
    }
    else
    {
        throw std::runtime_error("Missing ';' at end of server_name directive (line " + toString(line_number) + ")");
    }

    if (name_str.empty())
    {
        throw std::runtime_error("Empty server_name directive (line " + toString(line_number) + ")");
    }

    current_server.setName(name_str);
}

void handleRootDirective(const std::string &line, int line_number, Parsing_class &current_server)
{
    std::string root_str = trim(line.substr(4));
    size_t semicolon = root_str.find(';');
    if (semicolon != std::string::npos)
    {
        root_str = trim(root_str.substr(0, semicolon));
    }
    else
    {
        throw std::runtime_error("Missing ';' at end of root directive (line " + toString(line_number) + ")");
    }

    if (root_str.empty())
    {
        throw std::runtime_error("Empty root directive (line " + toString(line_number) + ")");
    }

    if (root_str[root_str.length() - 1] != '/')
    {
        root_str += "/";
    }

    current_server.setRoot(root_str);
}
