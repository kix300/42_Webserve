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
class Parsing_class {
    //variable qui sera donner a server class
    protected:
        int _port;
        std::string _root;
        std::string _name;
        int _server_fd;
        int _server_id;
		bool _error;
		std::string _index;
		std::map<std::string, LocationData> _LocationMap;
        //map de location
        //liste index

        // fonction de parsing
        public:
        Parsing_class();
        ~Parsing_class();
        void display();

        void setRoot(std::string &root);
        void setName(std::string &name);
        void setFd(int fd);
        void setPort(int port);
        void setId(int id);
        void setError(bool error);
		void setMap(const std::string &path, const LocationData &data);

        std::string getRoot();
        std::string getName();
        int getFd();
        int getPort();
        int getId();
        bool getError();
		const LocationData& getLocation(const std::string& path) const;
		

        void clear();


};
#endif
