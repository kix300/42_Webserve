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
#include <cstddef>

Parsing_class::Parsing_class() : _port(0), _root("default"), _name("default"), _server_fd(0), _server_id(0), _client_max_body_size(128 * 1024), _error(false), _index("default"){}

Parsing_class::~Parsing_class(){
	closeAllClients(-1);
}

void Parsing_class::display(){
	std::cout << "server_id : " <<  _server_id << std::endl;
	std::cout << "name : " <<  _name << std::endl;
	std::cout << "port : " <<  _port << std::endl;
	std::cout << "root : " <<  _root << std::endl;
	std::cout << "server_fd : " <<  _server_fd << std::endl;
	std::cout << "index : " <<  _index << std::endl;
	std::cout << "client_max_body_size : " << _client_max_body_size << std::endl;
	for (std::map<int, std::string>::iterator it = _error_pages.begin(); it != _error_pages.end(); it++){
		std::cout << "error_pages : " << it->second << std::endl;
	}
	for (std::map<std::string, LocationData>::iterator it = _LocationMap.begin(); it != _LocationMap.end(); it++){
		std::cout << "Location path : " << it->second.path << std::endl;
		std::cout << "Location root : " << it->second.root << std::endl;
		std::cout << "Location index : " << it->second.index << std::endl;
		std::cout << "Location fastcgi_pass : " << it->second.fastcgi_pass<< std::endl;
		std::cout << "Location redirect : " << it->second.redirect << std::endl;
		for (std::vector<std::string>::iterator itt = it->second.allowed_methods.begin(); itt != it->second.allowed_methods.end(); itt++){
			std::cout << "Location allowed_methods : " << *itt << std::endl;
		}
		for (std::map<std::string, std::string>::iterator itt = it->second.cgi_extensions.begin(); itt != it->second.cgi_extensions.end(); itt++){
			std::cout << "Location cgi_extension : " << itt->first << " -> " << itt->second << std::endl;
		}
	}
	std::cout << std::endl;

}

void Parsing_class::clear(){
	_port = 0;
	_root = "default";
	_name = "default";
	if (_server_fd > 0)
		close(_server_fd);
	_server_fd = 0;
	_server_id = 0;
	_client_max_body_size = 128 * 1024; // ici chnager le body en fonction du fichier
	_error = false;
	_index = "default";
	_error_pages.clear();
	_LocationMap.clear();
	closeAllClients(-1);
}

void Parsing_class::setPort(int port){ _port = port; }

void Parsing_class::setFd(int fd){ _server_fd = fd; }

void Parsing_class::setId(int id){ _server_id = id; }

void Parsing_class::setError(bool error){ _error = error; }

void Parsing_class::setRoot(std::string &root){ _root = root; }

void Parsing_class::setName(std::string &name){ _name = name; }

void Parsing_class::setIndex(std::string &index){ _index = index; }

void Parsing_class::setMap(const std::string &path, const LocationData &data){ 
	_LocationMap[path] = data;
}

void Parsing_class::setErrorPage(int error_code, const std::string& page) {
    _error_pages[error_code] = page;
}

void Parsing_class::setClientMaxBodySize(long long size) {
    _client_max_body_size = size;
}

LocationData* Parsing_class::getLocation(const std::string& path) {
	LocationData* best_match = NULL;
	size_t best_match_length = 0;
	
	for (std::map<std::string, LocationData>::iterator it = _LocationMap.begin(); 
		 it != _LocationMap.end(); ++it) {
		const std::string& location_path = it->first;
		
		if (path.find(location_path) == 0) {
			if (location_path.length() > best_match_length) {
				if (path.length() == location_path.length() || 
					path[location_path.length()] == '/' ||
					location_path[location_path.length() - 1] == '/') {
					best_match = &(it->second);
					best_match_length = location_path.length();
				}
			}
		}
	}
	
	return best_match;
}

std::string Parsing_class::getErrorPage(int error_code){
    std::map<int, std::string>::const_iterator it = _error_pages.find(error_code);
    if (it == _error_pages.end()) {
		return "";
    }
    return (it->second);
}

long long Parsing_class::getClientMaxBodySize() const {
    return _client_max_body_size;
}

int Parsing_class::getPort(){ return _port; }

int Parsing_class::getFd(){ return _server_fd; }

bool Parsing_class::getError(){ return _error; }

int Parsing_class::getId(){ return _server_id; }

std::string Parsing_class::getRoot(){ return _root; }

std::string Parsing_class::getIndex(){ return _index; }

std::string Parsing_class::getName(){ return _name; }

std::string Parsing_class::findFirstIndexFile(){
    std::stringstream ss(_index);
    std::string index_file;
    while (ss >> index_file) {
        std::string path = _root + index_file;
    	return path;
    }
	return "";
}

void Parsing_class::addClient(int client_fd, const ClientData& client) {
    _clients[client_fd] = client;
}

void Parsing_class::removeClient(int client_fd) {
    std::map<int, ClientData>::iterator it = _clients.find(client_fd);
    if (it != _clients.end()) {
        close(client_fd);
        _clients.erase(it);
    }
}

ClientData* Parsing_class::getClient(int client_fd) {
    std::map<int, ClientData>::iterator it = _clients.find(client_fd);
    if (it != _clients.end()) {
        return &(it->second);
    }
    return NULL;
}

void Parsing_class::closeAllClients(int epoll_fd) {
    for (std::map<int, ClientData>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (epoll_fd != -1) {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
        }
        close(it->first);
    }
    _clients.clear();
}

std::map<int, ClientData>& Parsing_class::getClients() {
    return _clients;
}

