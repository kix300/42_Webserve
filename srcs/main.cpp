/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 16:22:53 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/29 16:05:21 by ozen             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"

// Global flag to control the server loop.
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
	if (ac != 2)
	{
		std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
		return 1;
	}

	// Register signal handlers for graceful shutdown
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	std::map<int, Parsing_class> serverMap;
	int epoll_fd = -1;

	try
	{
		serverMap = count_nginx_servers(av[1], serverMap);

		if (serverMap.empty())
		{
			std::cerr << "No valid server configurations found or parsing failed." << std::endl;
			return 1;
		}

		for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
			it->second.setFd(create_server_socket(it->second.getPort()));
    	}

		epoll_fd = setup_epoll(serverMap);

		if (DEBUG){
			for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it){
				it->second.display();
			}
		}

		for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
			std::cout << "listen on http://localhost:" << it->second.getPort() << std::endl;
		}

		run_server(epoll_fd, serverMap);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		// Cleanup on error
		if (epoll_fd != -1)
			close(epoll_fd);
		for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
			if (it->second.getFd() > 0)
				close(it->second.getFd());
		}
		return 1;
	}

	// Cleanup after the server loop has finished
	std::cout << "\nServer shutting down gracefully..." << std::endl;
	for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
		if (it->second.getFd() > 0)
			close(it->second.getFd());
	}
	if (epoll_fd != -1)
		close(epoll_fd);
	std::cout << "Server stopped." << std::endl;

	return 0;
}