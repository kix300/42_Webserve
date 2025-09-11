/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_inout.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 12:04:16 by kduroux           #+#    #+#             */
/*   Updated: 2025/09/11 11:24:59 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <cerrno>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>


bool handle_write(int client_fd, ClientData & client){

	if (client_fd < 0) {
		return false;
	}
	if (client.write_buff.empty())
		return (true);
	ssize_t count = write(client_fd, client.write_buff.data(), client.write_buff.size());
	if (count == -1){
			return true;
	}
	else{
		client.write_buff.erase(0, count);
	}

	return true;
}


bool handle_read(int client_fd, ClientData & client){
	char buffer[BUFFER_SIZE];
	
	if (client_fd < 0) {
		return false;
	}
	
	ssize_t count = read(client_fd, buffer, BUFFER_SIZE);
	if (count == -1){
		return true;
	}
	else if (count == 0){
		return false;
	}
	else{
		client.read_buff.append(buffer, count);
	}
	
	return true;
}
