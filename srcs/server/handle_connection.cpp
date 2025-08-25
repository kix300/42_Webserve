/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_connection.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 17:24:20 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/11 13:18:30 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

// on ajoute notre nouveau client dans notre map en non bloquant
void handle_new_connection(int epoll_fd, int server_fd, Parsing_class &server) {
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
	if (client_fd == -1) {
		return;
	}
	
	std::cout << "=== NOUVELLE CONNEXION CLIENT " << client_fd << " ===" << std::endl;

	fcntl(client_fd, F_SETFL, O_NONBLOCK); // met le fd a non bloquant

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = client_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
		close(client_fd);
		return;
	}

	ClientData new_client = {client_fd, std::string(), std::string(), false, std::string(), std::string(), std::string(), &server, time(NULL), epoll_fd};
	server.addClient(client_fd, new_client);
}

// handle_client_event : Gestion des événements pour un client, si in alors on parse et prepare une responce
// sinon on gere le write
void handle_client_event(int epoll_fd, const epoll_event& event, ClientData& client, Parsing_class& server) {

	client.last_activity = time(NULL);

	int client_fd = event.data.fd;
	if (event.events & EPOLLIN){
		try {
			if (!handle_read(client_fd, client)){
				close_client(epoll_fd, client_fd, server);
				return;
			}

			if (client.read_buff.find("\r\n\r\n") != std::string::npos){
				parsing_response(client);
				prepare_response(client);

				struct epoll_event ev;
				ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
				ev.data.fd = client_fd;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
			}
		} catch (const std::exception &e) {
			std::cerr << "Error handling client " << client_fd << ": " << e.what() << std::endl;
			std::string e_mesg = e.what();
        	size_t colon= e_mesg.find(':');
            e_mesg = trim(e_mesg.substr(0, colon));
			if (sendErrorResponse(client, e_mesg) != 0){
				client.write_buff = "HTTP/1.1 " + e_mesg + " \r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
				client.keep_alive = false;
			}
			client.read_buff.clear();
			
			if (e_mesg.find("504") != std::string::npos) {
				client.keep_alive = false;
				handle_write(client_fd, client);
				close_client(epoll_fd, client_fd, server);
				return;
			}

			//Plus de EPOLLIN Donc a voir les form
			struct epoll_event ev;
			ev.events = EPOLLOUT | EPOLLET;
			ev.data.fd = client_fd;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
			return;
		}
	}
	if (event.events & EPOLLOUT){
		if (!handle_write(client_fd, client)){
			close_client(epoll_fd, client_fd, server);
			return;
		}
		if (client.write_buff.empty()){
			struct epoll_event ev;
			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = client_fd;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
			if (!client.keep_alive){
				close_client(epoll_fd, client_fd, server);
			}
		}
	}

	if (event.events & (EPOLLERR | EPOLLHUP)){
		close_client(epoll_fd, client_fd, server);
	}
}

bool errorPageExists(const std::string& path) {
    struct stat fileStat;
    return (stat(path.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode));
}

// sendErrorResponse : Gestion des principales pages d'erreurs
int sendErrorResponse(ClientData& client, const std::string& e_mesg) {
    int error_code = 0;
    if (e_mesg.find("403") != std::string::npos) error_code = 403;
    else if (e_mesg.find("404") != std::string::npos) error_code = 404;
    else if (e_mesg.find("405") != std::string::npos) error_code = 405;
	else if (e_mesg.find("408") != std::string::npos) error_code = 408;

    if (error_code != 0) {
        std::string error_page_path = client.server->getErrorPage(error_code);
        if (!error_page_path.empty() && errorPageExists(error_page_path)) {
            std::string file_content = read_file_or_directory(error_page_path, "", false);
            client.write_buff =
                "HTTP/1.1 " + e_mesg + "\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + tostring(file_content.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" +
                file_content;
            client.keep_alive = false; // Force la fermeture de la connexion
            return 0;
        }
    }
	return 1;
}
