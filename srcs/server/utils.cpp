/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:04:36 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/07 11:08:06 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

void close_client(int epoll_fd, int client_fd, std::map<int, ClientData> client){
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
	close(client_fd);
	client.erase(client_fd);
}
