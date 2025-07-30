/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_file.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 14:02:13 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/25 14:29:26 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

void processServerBlockContent(const std::string &line, int line_number, Parsing_class &current_server, std::set<int> &used_ports, int &flag_listen, bool in_location_block, const std::string &current_location_path)
{
	if (in_location_block){
		handleLocationDirective(line, line_number, current_server, current_location_path);
	}
	else {

		if (line.compare(0, 6, "listen") == 0 && (line[6] == ' ' || line[6] == '\t'))
		{
			handleListenDirective(line, line_number, current_server, used_ports, flag_listen);
		}
		else if (line.compare(0, 11, "server_name") == 0 && (line[11] == ' ' || line[11] == '\t'))
		{
			handleServerNameDirective(line, line_number, current_server);
		}
		else if (line.compare(0, 4, "root") == 0 && (line[4] == ' ' || line[4] == '\t'))
		{
			handleRootDirective(line, line_number, current_server);
		}
		else if (line.compare(0, 10, "error_page") == 0 && (line[10] == ' ' || line[10] == '\t'))
		{
			handleErrorPageDirective(line, line_number, current_server);
		}
		else if (line.compare(0, 20, "client_max_body_size") == 0 && (line[20] == ' ' || line[20] == '\t'))
		{
			handleClientMaxBodySizeDirective(line, line_number, current_server);
		}
		else if (line.compare(0, 5, "index") == 0 && (line[5] == ' ' || line[5] == '\t')){
			handleIndexNameDirective(line, line_number, current_server);
		}
	}
}

void validateDirectivesOutsideServerBlock(const std::string &line, int line_number, bool in_server_block)
{
	if (!in_server_block &&
		(line.find("listen") == 0 ||
		line.find("server_name") == 0 ||
		line.find("root") == 0 ||
		line.find("location") == 0 ||
		line.find("error_page") == 0))
	{
		throw std::runtime_error("Directive outside server block (line " + tostring(line_number) + ")");
	}
}

void handleServerBlockStart(const std::string &line, int line_number, bool &in_server_block, int &brace_level, int &server_count, Parsing_class &current_server)
{
	size_t open_brace_pos = line.find('{');
	if (open_brace_pos != std::string::npos)
	{
		if (in_server_block)
		{
			throw std::runtime_error("Nested server blocks are not allowed (line " + tostring(line_number) + ")");
		}

		std::string between = trim(line.substr(6, open_brace_pos - 6));
		if (!between.empty())
		{
			throw std::runtime_error("Unexpected tokens after 'server' directive (line " + tostring(line_number) + ")");
		}

		in_server_block = true;
		brace_level = 1;
		server_count++;
		current_server.clear();
	}
}

void handleLocationBlock(const std::string &line, int line_number, bool &in_location_block, int &brace_level, std::string &current_location_path, Parsing_class &current_server)
{
	size_t open_brace_pos = line.find('{');
	if (open_brace_pos != std::string::npos)
	{
		std::string path = trim(line.substr(8, open_brace_pos - 8));
		if (path.empty())
		{
			throw std::runtime_error("Missing location path in location directive on line " + tostring(line_number));
		}
		current_location_path = path;

		// The fix: Create and insert the new location context into the map immediately.
		LocationData new_location_data;
		new_location_data.path = path;
		current_server.setMap(path, new_location_data);

		in_location_block = true;
		brace_level++;
	}
}

void handleClosingBrace(int line_number, bool &in_server_block, bool &in_location_block, int &brace_level, int &flag_listen, int server_count, Parsing_class &current_server, std::map<int, Parsing_class> &serverMap)
{
	if (in_server_block)
	{
		brace_level--;
		if (brace_level == 0)
		{
			if (current_server.getPort() == 0)
			{
				throw std::runtime_error("Missing 'listen' directive in server block (line " + tostring(line_number) + ")");
			}

			current_server.setId(server_count);
			serverMap[server_count] = current_server;
			in_server_block = false;
			in_location_block = false;
			flag_listen = 0;
		}
		else if (brace_level < 0)
		{
			throw std::runtime_error("Unexpected '}' (line " + tostring(line_number) + ")");
		}
	}
	else
{
		throw std::runtime_error("Unexpected '}' without matching '{' (line " + tostring(line_number) + ")");
	}
}
