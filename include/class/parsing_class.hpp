/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_class.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 10:39:44 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/23 14:30:19 by kduroux          ###   ########.fr       */
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
class Parsing_class {
    //variable qui sera donner a server class
    protected:
        int port = 0;
        std::string root = NULL;
        std::string name = NULL;
        //map de location
        //liste index

    //fonction de parsing
    public:
    ~Parsing_class();
    Parsing_class();
    Parsing_class(int port, std::string root, std::string name);
    void display();


};
#endif