/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 16:20:38 by kduroux           #+#    #+#             */
/*   Updated: 2025/09/11 12:57:22 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
#define DEBUG false 
#define CORRECTION false
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <fcntl.h>
#include <map>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <csignal>
#include <set>
#include <stdexcept>
#include <sys/stat.h>
#include <ctime>

#include "class/parsing_class.hpp"

class Parsing_class;
struct ClientData {
	int fd;
	std::string read_buff;
	std::string write_buff;
	bool keep_alive;
	std::string methode;
	std::string path;
	std::string client_body;
	Parsing_class *server;
	time_t last_activity;
	int epoll_fd;
};

struct LocationData {
	std::string path;
	std::string root;
	std::string index;
	std::string redirect;
	std::vector<std::string> allowed_methods;
	bool autoindex;
	std::map<std::string, std::string> cgi_extensions;
	std::string cgi_timeout;

	std::string expires;
	std::string fastcgi_pass;
	std::vector<std::string> limit_except;
	std::vector<std::string> try_files;
	std::map<std::string, std::string> add_header;

	LocationData() : autoindex(false) {}
};


//server/
//main.cpp
int	create_server_socket( const int port );
int	setup_epoll(std::map<int, Parsing_class> serverMap);
void	run_server(int epoll_fd, std::map<int, Parsing_class> serverMap);

//handle_connection.cpp
void handle_new_connection(int epoll_fd, int server_fd, Parsing_class &server);
void handle_client_event(int epoll_fd, const epoll_event& event, ClientData& client, Parsing_class& server) ;
bool errorPageExists(const std::string& path);
int sendErrorResponse(ClientData& client, const std::string& e_mesg);

//handle_inout.cpp
bool handle_write(int client_fd, ClientData &client);
bool handle_read(int client_fd, ClientData &client);

//timeout.cpp
void check_clients_timeout(int epoll_fd, Parsing_class &server);


//utils.cpp
void close_client(int epoll_fd, int client_fd, Parsing_class& server);
std::map<std::string, std::string> parseFormData(const std::string& body);
std::string urlDecode(const std::string& str);
std::string generateDirectoryListing(const std::string& directory_path, const std::string& request_path);
std::string read_file_or_directory(const std::string& path, const std::string& request_path, bool autoindex_enabled);

// upload.cp
std::string extractBoundary(const std::string& request);
bool isMultipartFormData(const std::string& request);
std::string extractFileName(const std::string& contentDisposition);
bool handleFileUpload(ClientData& client);

//response.cpp
void prepare_response(ClientData& client);
void methode_get(ClientData& client);
void methode_post(ClientData& client);
void methode_delete(ClientData& client);
void check_location_methode(ClientData& client);

// cgi.cpp 
bool isCGIRequest(const std::string& path, const LocationData* location);
std::string executeCGI(ClientData& client, const std::string& script_path, const LocationData* location);
std::map<std::string, std::string> buildCGIEnvironment(ClientData& client, const std::string& script_path);
std::string getCGIInterpreter(const std::string& file_extension, const LocationData* location);
bool parseCGIResponse(const std::string& cgi_output, std::string& headers, std::string& body);
void buildHTTPResponse(ClientData& client, const std::string& cgi_output);


//parsing/
// parse_server.cpp
std::map<int, Parsing_class> count_nginx_servers(const std::string &filename, std::map<int, Parsing_class> serverMap);
std::string trim(const std::string& str);
int fill_server(std::string line, std::string root_str, std::string servername_str, int port);

//handle_file.cpp
void processServerBlockContent(const std::string &line, int line_number, Parsing_class &current_server, std::set<int> &used_ports, int &flag_listen, bool in_location_block, const std::string &current_location_path);
void validateDirectivesOutsideServerBlock(const std::string &line, int line_number, bool in_server_block);
void handleServerBlockStart(const std::string &line, int line_number, bool &in_server_block, int &brace_level, int &server_count, Parsing_class &current_server);
void handleLocationBlock(const std::string &line, int line_number, bool &in_location_block, int &brace_level, std::string &current_location_path, Parsing_class &current_server);
void handleClosingBrace(int line_number, bool &in_server_block, bool &in_location_block, int &brace_level, int &flag_listen, int server_count, Parsing_class &current_server, std::map<int, Parsing_class> &serverMap);

//handle_directive.cpp
template <typename T> std::string tostring(const T &value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
void handleListenDirective(const std::string &line, int line_number, Parsing_class &current_server, std::set<int> &used_ports, int &flag_listen);
void handleServerNameDirective(const std::string &line, int line_number, Parsing_class &current_server);
void handleRootDirective(const std::string &line, int line_number, Parsing_class &current_server);
void handleLocationDirective(const std::string &line, int line_number, Parsing_class &current_server, const std::string &current_location_path);
void handleErrorPageDirective(const std::string &line, int line_number, Parsing_class &current_server);
void handleClientMaxBodySizeDirective(const std::string &line, int line_number, Parsing_class &current_server);
void handleIndexNameDirective(const std::string &line, int line_number, Parsing_class &current_server);

//parsing_utils.cpp
std::string trim(const std::string &str);
bool isValidPort(const std::string &portStr);


//parsing_response.cpp
ClientData &parsing_response(ClientData &client);
std::string findFirstIndexFile(std::string index, std::string root);
std::string create_body(ClientData &client);
std::string locationinserver(LocationData *locationserver, ClientData client, std::string full_path);
#endif // !SERVER_HPP
