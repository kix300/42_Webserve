/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 16:22:53 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/24 17:11:59by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"

// Global flag to control the server loop.
// volatile sig_atomic_t is used to ensure that operations on this variable are atomic
// and to prevent the compiler from optimizing away reads of this variable.
volatile sig_atomic_t g_stop_server = 0;

// Signal handler function
void signal_handler(int signum)
{
	(void)signum; // Unused parameter
	g_stop_server = 1;
}



//Fonction principal du projet, on parse le fichier quon met dans une map
//on parcourt cette map pour creer des fd pour chaque server et on add a epoll
//puis on run le server
int main(int ac, char **av)
{
	if (ac == 2)
	{
		// Register signal handlers for graceful shutdown
		std::signal(SIGINT, signal_handler);
		std::signal(SIGTERM, signal_handler);

		std::map<int, Parsing_class> serverMap;
		serverMap = count_nginx_servers(av[1], serverMap);

		if (serverMap.empty() || serverMap[1].getId() == 0)
		{
			std::cerr << "No valid server configurations found or parsing failed." << std::endl;
			return 1;
		}

		for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
			it->second.setFd(create_server_socket(it->second.getPort()));
			if (it->second.getFd() < 0)
				return 1;
    	}
		int epoll_fd = setup_epoll(serverMap);
		if (epoll_fd < 0)
		{
			for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
				close(it->second.getFd());
			}
			return 1;
		}

		if (DEBUG){
			for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it){
				it->second.display();
			}
		}

		for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
			std::cout << "listen on http://localhost:" << it->second.getPort() << std::endl;
		}
		run_server(epoll_fd, serverMap);

		// Cleanup after the server loop has finished
		std::cout << "\nServer shutting down gracefully..." << std::endl;
		for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
			close(it->second.getFd());
		}
		close(epoll_fd);
		std::cout << "Server stopped." << std::endl;

		return 0;
	}
	else{
		std::cerr << "Need a conf file" << std::endl;
		return 1;
	}
}
