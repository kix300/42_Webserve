/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 16:34:17 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/11 13:11:34 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cerrno>
#include <cstring>

extern volatile sig_atomic_t g_stop_server;

//Creation d'un socket en fonction du port
int create_server_socket(const int port){
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1){
		throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));
	}

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
		close(server_fd);
		throw std::runtime_error("setsockopt() failed: " + std::string(strerror(errno)));
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0 ){
		close(server_fd);
		throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));
	}

	if (listen(server_fd, SOMAXCONN) < 0){
		close(server_fd);
		throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
	}

	return server_fd;
}

//Setup_epoll : Configuration de l'epoll pour surveiller les événements sur les sockets
int setup_epoll(std::map<int, Parsing_class> serverMap){
	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		throw std::runtime_error("epoll_create1() failed: " + std::string(strerror(errno)));
	}

	struct epoll_event event;
	
	for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
		event.events = EPOLLIN;
		event.data.fd = it->second.getFd();
		
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, it->second.getFd(), &event) == -1) {
			close(epoll_fd);
			throw std::runtime_error("epoll_ctl() failed: " + std::string(strerror(errno)));
		}
	}
	return epoll_fd;
}
// run_server : Coeur du server, gestion des connexions clients
void run_server(int epoll_fd, std::map<int, Parsing_class> serverMap){
	struct epoll_event events[MAX_EVENTS];

	while (!g_stop_server) {
		int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000); //gerer timeout
		if (nfds == -1) {
			if (errno == EINTR) {
				continue;
			}
			throw std::runtime_error("epoll_wait() failed: " + std::string(strerror(errno)));
		}

		for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
			check_clients_timeout(epoll_fd, it->second);
		}

		for (int i = 0; i < nfds; ++i) {
			bool is_server_socket = false;
			for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
				if (events[i].data.fd == it->second.getFd()) {
					is_server_socket = true;
					handle_new_connection(epoll_fd, it->second.getFd(), it->second);
					break;
				}
			}
			if (!is_server_socket){
				for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
					ClientData* client = it->second.getClient(events[i].data.fd);
					if (client != NULL) {
						handle_client_event(epoll_fd, events[i], *client, it->second);
						break;
					}
				}
			}
		}
	}
	for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
		it->second.closeAllClients(epoll_fd);
	}
}
