/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_class.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 10:40:00 by kduroux           #+#    #+#             */
/*   Updated: 2025/07/24 16:56:00 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/class/parsing_class.hpp"

Parsing_class::Parsing_class(int port, std::string root, std::string name) : _port(port), _root(root), _name(name){}
Parsing_class::Parsing_class() : _port(0), _root("default"), _name("default"){}

Parsing_class::~Parsing_class(){}

void Parsing_class::display(){
    std::cout << "name : " <<  _name << std::endl;
    std::cout << "port : " <<  _port << std::endl;
    std::cout << "root : " <<  _root << std::endl;
}

void Parsing_class::setPort(int port){
    _port = port;
}
void Parsing_class::setRoot(std::string root){
    _root = root;
}
void Parsing_class::setName(std::string name){
    _name = name;
}

int Parsing_class::getPort(){
    return _port;
}
std::string Parsing_class::getRoot(){
    return _root;
}
std::string Parsing_class::getName(){
    return _name;
}

void Parsing_class::clear(){
    _port = 0;
    _name.clear();
    _root.clear();
}