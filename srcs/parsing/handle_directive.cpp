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


void handleLocationDirective(const std::string &line, int line_number, Parsing_class &current_server, const std::string &current_location_path)
{
	std::string directive, value;
	std::istringstream iss(line);
	iss >> directive;
	std::getline(iss, value);
	value = trim(value);
	if (!value.empty() && value[value.length() - 1] == ';') {
		value.erase(value.length() - 1);
	}

	LocationData* loc = current_server.getLocation(current_location_path);
	if (directive == "root") {
		loc->root = value;
	} else if (directive == "index") {
		loc->index = value;
	} else if (directive == "autoindex") {
		loc->autoindex = (value == "on");
	} else if (directive == "return") {
		loc->redirect = value;
	} else if (directive == "allowed_methods") {
		std::istringstream iss_methods(value);
		std::string method;
		while (iss_methods >> method) {
			for (size_t i = 0; i < method.size(); ++i) {
				if (!isalpha(method[i])) {
					method.erase(i, 1);
					--i;
				}
			}
			if (method == "GET" || method == "POST" || method == "DELETE") {  // Ajoutez d'autres méthodes si besoin
				loc->allowed_methods.push_back(method);
			}
		}
	} else if (directive == "try_files") {
		std::istringstream iss_files(value);
		std::string file;
		while (iss_files >> file) {
			loc->try_files.push_back(file);
		}
	} else if (directive == "fastcgi_pass") {
		loc->fastcgi_pass = value;
	} else if (directive == "expires") {
		loc->expires = value;
	} else if (directive == "add_header") {
		std::string header_name, header_value;
		std::istringstream iss_header(value);
		iss_header >> header_name;
		std::getline(iss_header, header_value);
		loc->add_header[trim(header_name)] = trim(header_value);
	}
	else {
		throw std::runtime_error("Unknown directive in location block (line " + tostring(line_number) + ")");
	}
}

void handleListenDirective(const std::string &line, int line_number, Parsing_class &current_server, std::set<int> &used_ports, int &flag_listen)
{
	if (flag_listen == 0)
	{
		flag_listen = 0;
		std::string port_str = trim(line.substr(6));
		size_t semicolon = port_str.find(';');
		if (semicolon != std::string::npos)
		{
			port_str = trim(port_str.substr(0, semicolon));
		}
		else
	{
			throw std::runtime_error("Missing ';' at end of listen directive (line " + tostring(line_number) + ")");
		}

		if (!isValidPort(port_str))
		{
			throw std::runtime_error("Invalid port number (line " + tostring(line_number) + ")");
		}

		std::istringstream iss(port_str);
		int port;
		if (iss >> port)
		{
			if (used_ports.find(port) != used_ports.end())
			{
				throw std::runtime_error("Duplicate port (line " + tostring(line_number) + ")");
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
		throw std::runtime_error("Missing ';' at end of server_name directive (line " + tostring(line_number) + ")");
	}
	current_server.setName(name_str);
}

void handleIndexNameDirective(const std::string &line, int line_number, Parsing_class &current_server)
{
	std::string name_str = trim(line.substr(5));
	size_t semicolon = name_str.find(';');
	if (semicolon != std::string::npos)
	{
		name_str = trim(name_str.substr(0, semicolon));
	}
	else
{
		throw std::runtime_error("Missing ';' at end of server_name directive (line " + tostring(line_number) + ")");
	}
	current_server.setIndex(name_str);
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
		throw std::runtime_error("Missing ';' at end of root directive (line " + tostring(line_number) + ")");
	}

	if (root_str.empty())
	{
		throw std::runtime_error("Empty root directive (line " + tostring(line_number) + ")");
	}

	if (root_str[root_str.length() - 1] != '/')
	{
		root_str += "/";
	}

	current_server.setRoot(root_str);
}

void handleErrorPageDirective(const std::string &line, int line_number, Parsing_class &current_server)
{
	std::istringstream iss(line.substr(10));
	std::vector<int> error_codes;
	std::string page;
	int code;

	while (iss >> code)
	{
		error_codes.push_back(code);
	}

	if (iss.fail() && !iss.eof())
	{
		iss.clear();
		iss >> page;
	}

	size_t semicolon = page.find(';');
	if (semicolon != std::string::npos)
	{
		page = trim(page.substr(0, semicolon));
	}
	else
{
		throw std::runtime_error("Missing ';' at end of error_page directive (line " + tostring(line_number) + ")");
	}

	for (size_t i = 0; i < error_codes.size(); ++i)
	{
		current_server.setErrorPage(error_codes[i], page);
	}
}

void handleClientMaxBodySizeDirective(const std::string &line, int line_number, Parsing_class &current_server)
{
	std::string size_str = trim(line.substr(20));
	size_t semicolon = size_str.find(';');
	if (semicolon != std::string::npos)
	{
		size_str = trim(size_str.substr(0, semicolon));
	}
	else
{
		throw std::runtime_error("Missing ';' at end of client_max_body_size directive (line " + tostring(line_number) + ")");
	}

	long long size;
	char unit;
	std::istringstream iss(size_str);
	iss >> size;
	if (iss.peek() != EOF)
	{
		iss >> unit;
		switch (unit)
		{
			case 'k':
			case 'K':
				size *= 1024;
				break;
			case 'm':
			case 'M':
				size *= 1024 * 1024;
				break;
			case 'g':
			case 'G':
				size *= 1024 * 1024 * 1024;
				break;
			default:
				throw std::runtime_error("Invalid unit in client_max_body_size directive (line " + tostring(line_number) + ")");
		}
	}

	current_server.setClientMaxBodySize(size);
}
