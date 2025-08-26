/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_inout.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 12:04:16 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/11 13:11:50 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cerrno>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>


bool handle_write(int client_fd, ClientData & client){

	if (client.write_buff.empty())
		return (true);
	ssize_t count = write(client_fd, client.write_buff.data(), client.write_buff.size());
	if (count == -1){
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return true;
		}
		std::cout << "Error write" << std::endl;
		return false;
	}
	else{
		client.write_buff.erase(0, count);
	}

	return true;
}


bool handle_read(int client_fd, ClientData & client){
	char buffer[BUFFER_SIZE];
	while (true){
		ssize_t count = read(client_fd, buffer, BUFFER_SIZE);
		if (count == -1){
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return true;
			}
			std::cout << "Error read" << std::endl;
			return false;
		}
		else if (count  == 0){
			return false;
		}
		else{
			client.read_buff.append(buffer, count);
		}
	}
	return (true);
}
