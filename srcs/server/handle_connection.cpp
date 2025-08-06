/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_connection.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 17:24:20 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/29 16:05:21 by ozen             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

// on ajoute notre nouveau client dans notre map en non bloquant
void handle_new_connection(int epoll_fd, int server_fd, std::map<int, ClientData>& clients, Parsing_class &server) {
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

	clients[client_fd] = (ClientData){client_fd, std::string(), std::string(), false, std::string(), std::string(), &server};

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
		try {
			if (!handle_read(client_fd, client)){
				close_client(epoll_fd, client_fd, clients);
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

			//ici soit on creer un body error soit on renvoi l'erreur en fonction de e_mesg // a faire 
			std::string e_mesg = e.what();
        	size_t colon= e_mesg.find(':');
            e_mesg = trim(e_mesg.substr(0, colon));
			// if (e_mesg.find("403") != std::string::npos && client.server->getErrorPage(403) != ""){ // faut faire un stat sur le path de error page si il existe alors on sort la page sinon non
			// 	client.write_buff =
			// 		"HTTP/1.1 " + e_mesg + "\r\n"
			// 		"Content-Type: text/html\r\n"
			// 		"Content-Length: " + tostring(read_file(client.server->getErrorPage(403)).size()) + "\r\n"
			// 		"Connection: close" + "\r\n"
			// 		"\r\n"
			// 		+ read_file(client.server->getErrorPage(403));
			// }
			// else if (e_mesg.find("404") != std::string::npos && client.server->getErrorPage(404) != ""){ // faut faire un stat sur le path de error page si il existe alors on sort la page sinon non
			// 	//envoyer header
			// 	//readfile path
			// 	client.write_buff =
			// 		"HTTP/1.1 " + e_mesg + "\r\n"
			// 		"Content-Type: text/html\r\n"
			// 		"Content-Length: " + tostring(read_file(client.server->getErrorPage(404)).size()) + "\r\n"
			// 		"Connection: close" + "\r\n"
			// 		"\r\n"
			// 		+ read_file(client.server->getErrorPage(404));
			// }
			// else if (e_mesg.find("405") != std::string::npos && client.server->getErrorPage(405) != ""){ // faut faire un stat sur le path de error page si il existe alors on sort la page sinon non
			// 	//envoyer header
			// 	//readfile path
			// 	client.write_buff =
			// 		"HTTP/1.1 " + e_mesg + "\r\n"
			// 		"Content-Type: text/html\r\n"
			// 		"Content-Length: " + tostring(read_file(client.server->getErrorPage(405)).size()) + "\r\n"
			// 		"Connection: close" + "\r\n"
			// 		"\r\n"
			// 		+ read_file(client.server->getErrorPage(405));
			// }
			if (sendErrorResponse(client, e_mesg) != 0){
				client.write_buff = "HTTP/1.1 " + e_mesg + " \r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
				client.keep_alive = false;
			}

			struct epoll_event ev;
			ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
			ev.data.fd = client_fd;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
		}
	}

	if (event.events & EPOLLOUT){
		if (!handle_write(client_fd, client)){
			close_client(epoll_fd, client_fd, clients);
			return;
		}
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
		close_client(epoll_fd, client_fd, clients);
	}
}

bool errorPageExists(const std::string& path) {
    struct stat fileStat;
    return (stat(path.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode));
}

// Main error handling
int sendErrorResponse(ClientData& client, const std::string& e_mesg) {
    int error_code = 0;
    if (e_mesg.find("403") != std::string::npos) error_code = 403;
    else if (e_mesg.find("404") != std::string::npos) error_code = 404;
    else if (e_mesg.find("405") != std::string::npos) error_code = 405;

    if (error_code != 0) {
        std::string error_page_path = client.server->getErrorPage(error_code);
        if (!error_page_path.empty() && errorPageExists(error_page_path)) {
            std::string file_content = read_file(error_page_path);
            client.write_buff =
                "HTTP/1.1 " + e_mesg + "\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + tostring(file_content.size()) + "\r\n"
                "Connection: close\r\n"
                "\r\n" +
                file_content;
            return 0;
        }
    }
	return 1;
}
