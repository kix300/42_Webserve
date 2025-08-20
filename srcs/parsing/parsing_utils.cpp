/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 14:04:36 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/25 14:15:32 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

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

