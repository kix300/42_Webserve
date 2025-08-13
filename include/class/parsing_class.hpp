/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_class.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 10:39:44 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/25 12:02:00 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#ifndef PARSING_CLASS_HPP
#define PARSING_CLASS_HPP
#include "../server.hpp"

//savoir combien de server on le saura deja 
//savoir le port
//savoir la localisation des fichiers plusieur type
//savoir le nom du domaine
struct LocationData;
struct ClientData;
class Parsing_class {
    //variable qui sera donner a server class
    protected:
        int _port;
        std::string _root;
        std::string _name;
        int _server_fd;
        int _server_id;
		std::map<int, std::string> _error_pages;
		long long _client_max_body_size;
		bool _error;
		std::string _index;
		std::map<std::string, LocationData> _LocationMap;
		std::map<int, ClientData> _clients;

        // fonction de parsing
        public:
        Parsing_class();
        ~Parsing_class();
        void display();

        void setRoot(std::string &root);
        void setName(std::string &name);
        void setIndex(std::string &index);
        void setFd(int fd);
        void setPort(int port);
        void setId(int id);
        void setError(bool error);
		void setMap(const std::string &path, const LocationData &data);
		void setErrorPage(int error_code, const std::string& page);
		void setClientMaxBodySize(long long size);

        std::string getRoot();
        std::string getName();
        std::string getIndex();
        int getFd();
        int getPort();
        int getId();
        bool getError();
		LocationData* getLocation(const std::string& path);
		std::string getErrorPage(int error_code) ;
		long long getClientMaxBodySize() const;

		// Client management methods
		void addClient(int client_fd, const ClientData& client);
		void removeClient(int client_fd);
		ClientData* getClient(int client_fd);
		void closeAllClients(int epoll_fd);
		std::map<int, ClientData>& getClients();
		

        void clear();
        std::string findFirstIndexFile();


};
#endif
