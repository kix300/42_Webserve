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

void close_client(int epoll_fd, int client_fd, Parsing_class& server){
	if (epoll_fd != -1) {
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
	}
	server.removeClient(client_fd);
}

std::string read_file(const std::string &path) {
	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) != 0) {
		throw std::runtime_error("404 Not Found: File does not exist");
	}

	//autoindex ?!
	if (S_ISDIR(file_stat.st_mode)) {
		throw std::runtime_error("404 Not Found: Path is a directory");
	}

	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("404 Not Found: Could not open file");
	}

	std::string content;
	file.seekg(0, std::ios::end);
	content.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	content.assign((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());

	return content;
}

std::string urlDecode(const std::string& str) {
	std::string decoded;
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '+') {
			decoded += ' ';
		} else if (str[i] == '%' && i + 2 < str.length()) {
			int value;
			std::istringstream is(str.substr(i + 1, 2));
			if (is >> std::hex >> value) {
				decoded += static_cast<char>(value);
				i += 2;
			} else {
				decoded += str[i];
			}
		} else {
			decoded += str[i];
		}
	}
	return decoded;
}

// Fonction pour parser les données de formulaire URL-encoded
std::map<std::string, std::string> parseFormData(const std::string& body) {
	std::map<std::string, std::string> data;
	std::istringstream stream(body);
	std::string pair;

	while (std::getline(stream, pair, '&')) {
		size_t pos = pair.find('=');
		if (pos != std::string::npos) {
			std::string key = urlDecode(pair.substr(0, pos));
			std::string value = urlDecode(pair.substr(pos + 1));
			data[key] = value;
		}
	}

	return data;
}

