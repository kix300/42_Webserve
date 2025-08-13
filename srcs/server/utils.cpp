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
#include <algorithm>

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

std::string generateDirectoryListing(const std::string& directory_path, const std::string& request_path) {
	DIR* dir = opendir(directory_path.c_str());
	if (!dir) {
		throw std::runtime_error("403 Forbidden: Cannot access directory");
	}

	std::string html = "<!DOCTYPE html>\n<html>\n<head>\n";
	html += "<title>Index of " + request_path + "</title>\n";
	html += "<style>\n";
	html += "body { font-family: Arial, sans-serif; margin: 20px; }\n";
	html += "h1 { color: #333; }\n";
	html += "table { border-collapse: collapse; width: 100%; }\n";
	html += "th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n";
	html += "th { background-color: #f2f2f2; }\n";
	html += "a { text-decoration: none; color: #0066cc; }\n";
	html += "a:hover { text-decoration: underline; }\n";
	html += "</style>\n";
	html += "</head>\n<body>\n";
	html += "<h1>Index of " + request_path + "</h1>\n";
	html += "<table>\n";
	html += "<thead>\n<tr><th>Name</th><th>Last modified</th><th>Size</th></tr>\n</thead>\n";
	html += "<tbody>\n";

	// Add parent directory link if not root
	if (request_path != "/" && request_path != "") {
		html += "<tr>\n";
		std::string parent_path = request_path;
		if (!parent_path.empty() && parent_path[parent_path.size() - 1] == '/') {
			parent_path.erase(parent_path.size() - 1);
		}
		size_t last_slash = parent_path.find_last_of('/');
		if (last_slash != std::string::npos) {
			parent_path = parent_path.substr(0, last_slash + 1);
		} else {
			parent_path = "/";
		}
		html += "<td><a href=\"" + parent_path + "\">../</a></td>\n";
		html += "<td>-</td>\n<td>-</td>\n";
		html += "</tr>\n";
	}

	struct dirent* entry;
	std::vector<std::string> directories;
	std::vector<std::string> files;

	// Read directory entries
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == "." || name == "..") {
			continue;
		}

		std::string full_path = directory_path;
		if (!full_path.empty() && full_path[full_path.size() - 1] != '/') {
			full_path += "/";
		}
		full_path += name;

		struct stat file_stat;
		if (stat(full_path.c_str(), &file_stat) == 0) {
			if (S_ISDIR(file_stat.st_mode)) {
				directories.push_back(name);
			} else {
				files.push_back(name);
			}
		}
	}
	closedir(dir);

	// Sort directories and files
	std::sort(directories.begin(), directories.end());
	std::sort(files.begin(), files.end());

	// Add directories first
	for (std::vector<std::string>::iterator it = directories.begin(); it != directories.end(); ++it) {
		std::string name = *it;
		std::string full_path = directory_path;
		if (!full_path.empty() && full_path[full_path.size() - 1] != '/') {
			full_path += "/";
		}
		full_path += name;

		struct stat file_stat;
		if (stat(full_path.c_str(), &file_stat) == 0) {
			char time_buf[100];
			struct tm* timeinfo = localtime(&file_stat.st_mtime);
			strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", timeinfo);

			html += "<tr>\n";
			std::string request_url = request_path;
			if (!request_url.empty() && request_url[request_url.size() - 1] != '/') {
				request_url += "/";
			}
			html += "<td><a href=\"" + request_url + name + "/\">" + name + "/</a></td>\n";
			html += "<td>" + std::string(time_buf) + "</td>\n";
			html += "<td>-</td>\n";
			html += "</tr>\n";
		}
	}

	// Add files
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it) {
		std::string name = *it;
		std::string full_path = directory_path;
		if (!full_path.empty() && full_path[full_path.size() - 1] != '/') {
			full_path += "/";
		}
		full_path += name;

		struct stat file_stat;
		if (stat(full_path.c_str(), &file_stat) == 0) {
			char time_buf[100];
			struct tm* timeinfo = localtime(&file_stat.st_mtime);
			strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", timeinfo);

			std::string size_str;
			if (file_stat.st_size < 1024) {
				std::stringstream ss;
				ss << file_stat.st_size << " B";
				size_str = ss.str();
			} else if (file_stat.st_size < 1024 * 1024) {
				std::stringstream ss;
				ss << (file_stat.st_size / 1024) << " KB";
				size_str = ss.str();
			} else {
				std::stringstream ss;
				ss << (file_stat.st_size / (1024 * 1024)) << " MB";
				size_str = ss.str();
			}

			html += "<tr>\n";
			std::string request_url = request_path;
			if (!request_url.empty() && request_url[request_url.size() - 1] != '/') {
				request_url += "/";
			}
			html += "<td><a href=\"" + request_url + name + "\">" + name + "</a></td>\n";
			html += "<td>" + std::string(time_buf) + "</td>\n";
			html += "<td>" + size_str + "</td>\n";
			html += "</tr>\n";
		}
	}

	html += "</tbody>\n</table>\n";
	html += "</body>\n</html>\n";

	return html;
}

std::string read_file_or_directory(const std::string& path, const std::string& request_path, bool autoindex_enabled) {
	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) != 0) {
		throw std::runtime_error("404 Not Found: File does not exist");
	}

	if (S_ISDIR(file_stat.st_mode)) {
		if (autoindex_enabled) {
			return generateDirectoryListing(path, request_path);
		} else {
			throw std::runtime_error("403 Forbidden: Directory listing disabled");
		}
	}

	// It's a file, use the original read_file function
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
	file.close();

	return content;
}

