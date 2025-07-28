/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_class.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 10:40:00 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/25 12:04:05 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/class/parsing_class.hpp"

Parsing_class::Parsing_class() : _port(0), _root("default"), _name("default"), _server_fd(0), _server_id(0), _error(false), _index("default"){}

Parsing_class::~Parsing_class(){}

void Parsing_class::display(){
	std::cout << "server_id : " <<  _server_id << std::endl;
	std::cout << "name : " <<  _name << std::endl;
	std::cout << "port : " <<  _port << std::endl;
	std::cout << "root : " <<  _root << std::endl;
	std::cout << "server_fd : " <<  _server_fd << "\n" << std::endl;
}

void Parsing_class::clear(){
	_port = 0;
	_name.clear();
	_root.clear();
	if (_server_fd > 0)
		close(_server_fd);
	_server_id = 0;
}

void Parsing_class::setPort(int port){ _port = port; }

void Parsing_class::setFd(int fd){ _server_fd = fd; }

void Parsing_class::setId(int id){ _server_id = id; }

void Parsing_class::setError(bool error){ _error = error; }

void Parsing_class::setRoot(std::string &root){ _root = root; }

void Parsing_class::setName(std::string &name){ _name = name; }

void Parsing_class::setMap(const std::string &path, const LocationData &data){ 
	_LocationMap[path] = data;
}

const LocationData& Parsing_class::getLocation(const std::string& path) const {
	std::map<std::string, LocationData>::const_iterator it = _LocationMap.find(path);
	if (it == _LocationMap.end()) {
		throw std::runtime_error("Location not found");
	}
	return it->second;
}

int Parsing_class::getPort(){ return _port; }

int Parsing_class::getFd(){ return _server_fd; }

bool Parsing_class::getError(){ return _error; }

int Parsing_class::getId(){ return _server_id; }

std::string Parsing_class::getRoot(){ return _root; }

std::string Parsing_class::getName(){ return _name; }

