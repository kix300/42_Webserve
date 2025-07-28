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

		// This part seems to be a placeholder. The actual server setup should likely
		// loop through the serverMap and set up multiple servers if needed.
		for (std::map<int, Parsing_class>::iterator it = serverMap.begin(); it != serverMap.end(); ++it) {
			it->second.setFd(create_server_socket(it->second.getPort()));
    	}
		int server_fd = serverMap[1].getFd();
		if (server_fd < 0)
			return 1;

		int epoll_fd = setup_epoll(server_fd);
		if (epoll_fd < 0)
		{
			close(server_fd);
			return 1;
		}

		if (DEBUG){
			serverMap[1].display();
			// Assuming serverMap[2] might exist based on original code, but not guaranteed.
			// If it doesn't, this line would cause a crash. Consider safer iteration.
			serverMap[2].display();
		}

		std::cout << "listen on http://localhost:" << serverMap[1].getPort() << std::endl;
		run_server(epoll_fd, server_fd);

		// Cleanup after the server loop has finished
		std::cout << "\nServer shutting down gracefully..." << std::endl;
		close(server_fd);
		close(epoll_fd);
		std::cout << "Server stopped." << std::endl;

		return 0;
	}
	else{
		std::cerr << "Need a conf file" << std::endl;
		return 1;
	}
}
