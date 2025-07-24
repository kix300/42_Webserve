/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 16:22:53 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/24 17:05:40 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"

// class abstraite qui parse
// class server qui herite de class parse on en fait autant quil faut

volatile sig_atomic_t stop = 0;

void signalHandler(int sig)
{
	// je suppose quil faudra faire une class proprement pour exit proprement
	(void)sig;
	stop = 1;
}

int main(int ac, char **av)
{
	if (ac == 2)
	{
		std::map<int, Parsing_class> serverMap;
		std::signal(SIGINT, signalHandler);
		std::signal(SIGTERM, signalHandler);

		// ici parsing
		const int port = 8080;

		serverMap = count_nginx_servers(av[1], serverMap);
		serverMap[1].display();
		serverMap[2].display();
		// en fonction du nombre de server nous devront avoir obligatoirement plusieurs socket et peuteter plusieurs epoll je sais pas encore
		int server_fd = create_server_socket(port);
		// revoir le setup d'epoll pour plusieurs server
		int epoll_fd = setup_epoll(server_fd);

		std::cout << "listen on http://localhost:8080" << std::endl;
		run_server(epoll_fd, server_fd);

		close(server_fd);

		return 0;
	}
	else{
		std::cerr << "Need a conf file" << std::endl;
		return 1;
	}
}