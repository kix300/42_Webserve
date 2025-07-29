/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 16:34:17 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/28 18:24:28 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

extern volatile sig_atomic_t g_stop_server;

//Creation dun socket en fonction du port
int create_server_socket(const int port){
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1){
		perror("socket");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
		perror("socket opt");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0 ){
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, SOMAXCONN) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	}

	return server_fd;
}

//on va setup epoll aavec tous les fd des servers
int setup_epoll(std::map<int, Parsing_class> serverMap){
	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	struct epoll_event event;
	
	for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
		event.events = EPOLLIN;
		event.data.fd = it->second.getFd();
		
		

		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, it->second.getFd(), &event) == -1) {
			perror("epoll_ctl: server_fd");
			exit(EXIT_FAILURE);
		}
	}
	return epoll_fd;
}

void run_server(int epoll_fd, std::map<int, Parsing_class> serverMap){
	std::map<int,  ClientData> clients;
	struct epoll_event events[MAX_EVENTS];

	while (!g_stop_server) {
		int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); //gerer timeout
		if (nfds == -1) {
			if (errno == EINTR) { // Interrupted by signal, check the flag
				continue;
			}
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < nfds; ++i) {
			bool is_server_socket = false;
			for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
				if (events[i].data.fd == it->second.getFd()) {
					is_server_socket = true;
					handle_new_connection(epoll_fd, it->second.getFd(), clients);
					break;
				}
			}
			if (!is_server_socket)
					handle_client_event(epoll_fd, events[i], clients);
		}
	}
}
