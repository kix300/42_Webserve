/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 16:20:38 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/25 14:15:52 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
#define DEBUG true
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <fcntl.h>
#include <map>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <csignal>
#include <set>
#include <stdexcept>

#include "class/parsing_class.hpp"

//possibilite de class surtout pour le parsing / conf file

class Parsing_class;
struct ClientData {
	int fd;
	std::string read_buff;
	std::string write_buff;
	bool keep_alive;
};


//server/
//main.cpp
int	create_server_socket( const int port );
int	setup_epoll(int server_fd);
void	run_server(int epoll_fd, int server_fd);

//handle_connection.cpp
void handle_new_connection(int epoll_fd, int server_fd, std::map<int, ClientData>& clients);
void handle_client_event(int epoll_fd, const epoll_event &events, std::map<int, ClientData>& clients);

//handle_inout.cpp
bool handle_write(int client_fd, ClientData &client);
bool handle_read(int client_fd, ClientData &client);

//utils.cpp
void close_client(int epoll_fd, int client_fd, std::map<int, ClientData> client);

//response.cpp
void prepare_response(ClientData & client);

//parsing/
// parse_server.cpp
std::map<int, Parsing_class> count_nginx_servers(const std::string &filename, std::map<int, Parsing_class> serverMap);
std::string trim(const std::string& str);
int fill_server(std::string line, std::string root_str, std::string servername_str, int port);

//handle_file.cpp
void processServerBlockContent(const std::string &line, int line_number, Parsing_class &current_server, std::set<int> &used_ports, int &flag_listen, bool in_location_block);
void validateDirectivesOutsideServerBlock(const std::string &line, int line_number, bool in_server_block);
void handleServerBlockStart(const std::string &line, int line_number, bool &in_server_block, int &brace_level, int &server_count, Parsing_class &current_server);
void handleLocationBlock(const std::string &line, bool &in_location_block, int &brace_level);
void handleClosingBrace(int line_number, bool &in_server_block, bool &in_location_block, int &brace_level, int &flag_listen, int server_count, Parsing_class &current_server, std::map<int, Parsing_class> &serverMap);

//handle_directive.cpp
template <typename T> std::string toString(const T &value);
void handleListenDirective(const std::string &line, int line_number, Parsing_class &current_server, std::set<int> &used_ports, int &flag_listen);
void handleServerNameDirective(const std::string &line, int line_number, Parsing_class &current_server);
void handleRootDirective(const std::string &line, int line_number, Parsing_class &current_server);

//parsing_utils.cpp
std::string trim(const std::string &str);
bool isValidPort(const std::string &portStr);
bool directoryExists(const std::string &path);
#endif // !SERVER_HPP
