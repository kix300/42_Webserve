/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_connection.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 17:24:20 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/29 11:21:17 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

// on ajoute notre nouveau client dans notre map en non bloquant
void handle_new_connection(int epoll_fd, int server_fd, std::map<int, ClientData>& clients) {
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
	if (client_fd == -1) {
		perror("accept");
		return;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK); // met le fd a non bloquant

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
		perror("epoll_ctl: client_fd");
		close(client_fd);
		return;
	}

	clients[client_fd] = (ClientData){client_fd, std::string(), std::string(), false, std::string(), std::string()};

}

//ici on va regarder les events, les clients fd et le fd de epoll pour voir si c'est inout 
//ensuite si c'est in alors on read
//si c'est out alors on write
void handle_client_event(int epoll_fd, const epoll_event& event, std::map<int, ClientData>& clients) {

	int client_fd = event.data.fd;
	//on cherche le bon client dans notre map
	std::map<int, ClientData>::iterator it = clients.find(client_fd);
    if (it == clients.end()) {
        close(client_fd);
        return;
    }
    ClientData& client = it->second;
	if (event.events & EPOLLIN){
		//recup avec handle read
		if (! handle_read(client_fd, client)){
			close_client(epoll_fd, client_fd, clients);
		}
		//on donne une reponse
		if (DEBUG)
			std::cout << client.read_buff << std::endl;
		//ici on doit parser le read_buff pour choper des infos sur les methodes
		client = parsing_response(client);
		if (client.read_buff.find("\r\n\r\n") != std::string::npos){
			prepare_response(client);

			//puis on surveille epollout
			struct epoll_event ev;
			ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
			ev.data.fd = client_fd;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);

		}
	}

	if (event.events & EPOLLOUT){
		//handle write
		if (! handle_write(client_fd, client)){
			close_client(epoll_fd, client_fd, clients);
		}
		//on retourn sur epollint
		if (client.write_buff.empty()){
			struct epoll_event ev;
			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = client_fd;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
			if (!client.keep_alive){
				close_client(epoll_fd, client_fd, clients);
			}
		}
	}

	if (event.events & (EPOLLERR | EPOLLHUP)){
		//close clients
		close_client(epoll_fd, client_fd, clients);
	}
}
/*
*/
