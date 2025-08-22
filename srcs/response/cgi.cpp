/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 10:00:00 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/22 14:23:27 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include <sys/wait.h>
#include <cstring>
#include <unistd.h>

bool isCGIRequest(const std::string& path, const LocationData* location) {
    if (!location || location->cgi_extensions.empty()) {
        return false;
	}

	size_t dot_pos = path.find_last_of('.');
	if (dot_pos == std::string::npos) {
		return false;
	}

	std::string extension = path.substr(dot_pos);
	return location->cgi_extensions.find(extension) != location->cgi_extensions.end();
}

std::string getCGIInterpreter(const std::string& file_extension, const LocationData* location) {
	if (!location) {
		return "";
	}

	std::map<std::string, std::string>::const_iterator it = location->cgi_extensions.find(file_extension);
	if (it != location->cgi_extensions.end()) {
		return it->second;
	}
	return "";
}

// Construit l'environnement CGI selon RFC 3875
std::map<std::string, std::string> buildCGIEnvironment(ClientData& client, const std::string& script_path) {
	std::map<std::string, std::string> env;

	// Variables CGI obligatoires
	env["REQUEST_METHOD"] = client.methode;
	env["SCRIPT_NAME"] = client.path;
	env["SCRIPT_FILENAME"] = script_path;
	env["SERVER_NAME"] = client.server->getName();
	env["SERVER_PORT"] = tostring(client.server->getPort());
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_SOFTWARE"] = "Webserve/1.0";
	env["GATEWAY_INTERFACE"] = "CGI/1.1";

	// Variables de requête
	size_t query_pos = client.path.find('?');
	if (query_pos != std::string::npos) {
		env["QUERY_STRING"] = client.path.substr(query_pos + 1);
		env["SCRIPT_NAME"] = client.path.substr(0, query_pos);
	} else {
		env["QUERY_STRING"] = "";
	}

	// Variables pour POST
	if (client.methode == "POST") {
		env["CONTENT_LENGTH"] = tostring(client.client_body.length());

		// Extraire Content-Type depuis les headers de la requête
		size_t content_type_pos = client.read_buff.find("Content-Type:");
		if (content_type_pos != std::string::npos) {
			size_t line_end = client.read_buff.find("\r\n", content_type_pos);
			if (line_end != std::string::npos) {
				std::string content_type_line = client.read_buff.substr(content_type_pos + 13, line_end - content_type_pos - 13);
				// Trim whitespace
				size_t start = content_type_line.find_first_not_of(" \t");
				if (start != std::string::npos) {
					env["CONTENT_TYPE"] = content_type_line.substr(start);
				}
			}
		}
	}

	// Variables d'environnement système
	env["PATH_INFO"] = "";
	env["PATH_TRANSLATED"] = "";
	env["REMOTE_ADDR"] = "127.0.0.1";  // Pour localhost
	env["REMOTE_HOST"] = "";
	env["REMOTE_USER"] = "";
	env["AUTH_TYPE"] = "";

	// Headers HTTP comme variables d'environnement
	// Convertir les headers HTTP en variables CGI (HTTP_*)
	size_t header_start = client.read_buff.find("\r\n") + 2;
	size_t header_end = client.read_buff.find("\r\n\r\n");

	if (header_start < header_end) {
		std::string headers = client.read_buff.substr(header_start, header_end - header_start);
		std::istringstream header_stream(headers);
		std::string line;

		while (std::getline(header_stream, line)) {
			if (line.empty() || line == "\r") break;

			size_t colon_pos = line.find(':');
			if (colon_pos != std::string::npos) {
				std::string header_name = line.substr(0, colon_pos);
				std::string header_value = line.substr(colon_pos + 1);

				// Nettoyer et convertir le nom du header
				// Remplacer - par _ et convertir en majuscules
				for (size_t i = 0; i < header_name.length(); ++i) {
					if (header_name[i] == '-') {
						header_name[i] = '_';
					} else {
						header_name[i] = std::toupper(header_name[i]);
					}
				}

				// Nettoyer la valeur
				size_t value_start = header_value.find_first_not_of(" \t\r");
				if (value_start != std::string::npos) {
					header_value = header_value.substr(value_start);
					size_t value_end = header_value.find_last_not_of(" \t\r");
					if (value_end != std::string::npos) {
						header_value = header_value.substr(0, value_end + 1);
					}
				}

				env["HTTP_" + header_name] = header_value;
			}
		}
	}

	return env;
}

// Exécute un script CGI et retourne la sortie
std::string executeCGI(ClientData& client, const std::string& script_path, const LocationData* location) {
	// Vérifier que le fichier existe
	struct stat file_stat;
	if (stat(script_path.c_str(), &file_stat) != 0) {
		throw std::runtime_error("404 Not Found: CGI script not found");
	}

	// Obtenir l'extension et l'interpréteur
	size_t dot_pos = script_path.find_last_of('.');
	if (dot_pos == std::string::npos) {
		throw std::runtime_error("500 Internal Server Error: No file extension");
	}

	std::string extension = script_path.substr(dot_pos);
	std::string interpreter = getCGIInterpreter(extension, location);

	if (interpreter.empty()) {
		throw std::runtime_error("500 Internal Server Error: No interpreter found for extension");
	}
	
	// check si cle interpreter existe
	if (stat(interpreter.c_str(), &file_stat) != 0)
		throw std::runtime_error("500 Internal Server Error: No interpreter found for extension");

	// Créer les pipes pour la communication
	int pipe_in[2], pipe_out[2];
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
		throw std::runtime_error("500 Internal Server Error: Failed to create pipes");
	}

	// Fork pour exécuter le script
	pid_t pid = fork();
	if (pid == -1) {
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		throw std::runtime_error("500 Internal Server Error: Fork failed");
	}

	if (pid == 0) {
		// Processus enfant
		close(pipe_in[1]);   // Fermer écriture du pipe d'entrée
		close(pipe_out[0]);  // Fermer lecture du pipe de sortie

		// Rediriger stdin et stdout
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);

		close(pipe_in[0]);
		close(pipe_out[1]);


		// Préparer l'environnement
		std::map<std::string, std::string> cgi_env = buildCGIEnvironment(client, script_path);

		// Convertir l'environnement en format char**
		std::vector<std::string> env_strings;
		std::vector<char*> env_ptrs;

		for (std::map<std::string, std::string>::iterator it = cgi_env.begin(); it != cgi_env.end(); ++it) {
			env_strings.push_back(it->first + "=" + it->second);
		}

		for (std::vector<std::string>::iterator it = env_strings.begin(); it != env_strings.end(); ++it) {
			env_ptrs.push_back(const_cast<char*>(it->c_str()));
		}
		env_ptrs.push_back(NULL);

		// Préparer les arguments
		char* args[] = {
			const_cast<char*>(interpreter.c_str()),
			const_cast<char*>(script_path.c_str()),
			NULL
		};

		// Exécuter le script
		execve(interpreter.c_str(), args, &env_ptrs[0]);

		// Si on arrive ici, execve a échoué
		std::cout << "Error execve" << std::endl;
		exit(1);
	} else {
		// Processus parent
		close(pipe_in[0]);   // Fermer lecture du pipe d'entrée
		close(pipe_out[1]);  // Fermer écriture du pipe de sortie

		// Envoyer le body de la requête au script (pour POST)
		if (client.methode == "POST" && !client.client_body.empty()) {
			ssize_t written = write(pipe_in[1], client.client_body.c_str(), client.client_body.length());
			if (written == -1) {
				std::cout << "Error write" << std::endl;
			}
		}
		close(pipe_in[1]);

		// Lire la sortie du script avec timeout
		std::string output;
		char buffer[BUFFER_SIZE];
		time_t start_time = time(NULL);
		int timeout_seconds = 3; // Default timeout
		int status;

		if (location && !location->cgi_timeout.empty()) {
			timeout_seconds = std::atoi(location->cgi_timeout.c_str());
			if (timeout_seconds <= 0) {
				timeout_seconds = 3; // a default value
			}
		}

		while (true) {
			if (difftime(time(NULL), start_time) > timeout_seconds) {
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0); // Clean up the zombie process
				close(pipe_out[0]);
				throw std::runtime_error("504 Gateway Timeout: CGI script timed out");
			}

			fd_set read_fds;
			FD_ZERO(&read_fds);
			FD_SET(pipe_out[0], &read_fds);

			struct timeval tv;
			tv.tv_sec = 1; // Wait up to 1 second.
			tv.tv_usec = 0;

			int retval = select(pipe_out[0] + 1, &read_fds, NULL, NULL, &tv);

			if (retval == -1) {
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				close(pipe_out[0]);
				throw std::runtime_error("500 Internal Server Error: select() failed");
			} else if (retval > 0) {
				ssize_t bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1);
				if (bytes_read > 0) {
					buffer[bytes_read] = '\0';
					output += buffer;
				} else {
					break;
				}
			}
		}

		close(pipe_out[0]);
		waitpid(pid, &status, 0);

		if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
			throw std::runtime_error("500 Internal Server Error: CGI script exited with error");
		}

		return output;
	}
}

// Parse la sortie CGI pour séparer headers et body
bool parseCGIResponse(const std::string& cgi_output, std::string& headers, std::string& body) {
	size_t header_end = cgi_output.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		// Pas de séparation headers/body trouvée, traiter comme du contenu simple
		headers = "";
		body = cgi_output;
		return false;
	}

	headers = cgi_output.substr(0, header_end);
	body = cgi_output.substr(header_end + 4);
	return true;
}

// Construit une réponse HTTP complète à partir de la sortie CGI
void buildHTTPResponse(ClientData& client, const std::string& cgi_output) {
	std::string cgi_headers, cgi_body;
	bool has_headers = parseCGIResponse(cgi_output, cgi_headers, cgi_body);

	std::string response = "HTTP/1.1 200 OK\r\n";

	bool has_content_type = false;
	bool has_content_length = false;

	if (has_headers && !cgi_headers.empty()) {
		// Analyser les headers CGI
		std::istringstream header_stream(cgi_headers);
		std::string line;

		while (std::getline(header_stream, line)) {
			if (line.empty() || line == "\r") break;

			// Nettoyer la ligne
			if (!line.empty() && line[line.length() - 1] == '\r') {
				line = line.substr(0, line.length() - 1);
			}

			if (line.empty()) continue;

			// Vérifier les headers spéciaux
			if (line.find("Content-Type:") == 0) {
				has_content_type = true;
			} else if (line.find("Content-Length:") == 0) {
				has_content_length = true;
			} else if (line.find("Status:") == 0) {
				// Remplacer la ligne de statut HTTP
				response = "HTTP/1.1 " + line.substr(8) + "\r\n";
				continue;
			}

			response += line + "\r\n";
		}
	}

	// Ajouter les headers manquants
	if (!has_content_type) {
		response += "Content-Type: text/html\r\n";
	}
	if (!has_content_length) {
		response += "Content-Length: " + tostring(cgi_body.length()) + "\r\n";
	}

	response += "Connection: " + (client.keep_alive ? std::string("keep-alive") : std::string("close")) + "\r\n";
	response += "\r\n";
	response += cgi_body;

	client.write_buff = response;
}
