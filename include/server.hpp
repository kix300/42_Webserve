/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 16:20:38 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/08 11:55:17 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
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

//possibilite de class surtout pour le parsing / conf file

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

//parsing
//count_server.cpp
int count_nginx_servers(const std::string& filename);
std::string trim(const std::string& str);

#endif // !SERVER_HPP
