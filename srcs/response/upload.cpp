/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   upload.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kduroux <kduroux@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/18 15:26:05 by kduroux           #+#    #+#             */
/*   Updated: 2025/08/18 15:26:06 by kduroux          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

std::string extractBoundary(const std::string& request) {
	size_t content_type_pos = request.find("Content-Type:");
	if (content_type_pos == std::string::npos) {
		return "";
	}
	
	size_t line_end = request.find("\r\n", content_type_pos);
	if (line_end == std::string::npos) {
		return "";
	}
	
	std::string content_type_line = request.substr(content_type_pos, line_end - content_type_pos);
	
	size_t boundary_pos = content_type_line.find("boundary=");
	if (boundary_pos == std::string::npos) {
		return "";
	}
	
	std::string boundary = content_type_line.substr(boundary_pos + 9);
	size_t end = boundary.find_first_of(" \t\r\n");
	if (end != std::string::npos) {
		boundary = boundary.substr(0, end);
	}
	
	return boundary;
}

bool isMultipartFormData(const std::string& request) {
	size_t content_type_pos = request.find("Content-Type:");
	if (content_type_pos == std::string::npos) {
		return false;
	}
	
	size_t line_end = request.find("\r\n", content_type_pos);
	if (line_end == std::string::npos) {
		return false;
	}
	
	std::string content_type_line = request.substr(content_type_pos, line_end - content_type_pos);
	return content_type_line.find("multipart/form-data") != std::string::npos;
}

std::string extractFileName(const std::string& contentDisposition) {
	size_t filename_pos = contentDisposition.find("filename=\"");
	if (filename_pos == std::string::npos) {
		return "";
	}
	
	filename_pos += 10;
	size_t end_pos = contentDisposition.find("\"", filename_pos);
	if (end_pos == std::string::npos) {
		return "";
	}
	
	return contentDisposition.substr(filename_pos, end_pos - filename_pos);
}

bool handleFileUpload(ClientData& client) {
	std::string request = client.read_buff;
	
	if (!isMultipartFormData(request)) {
		return false;
	}
	
	std::string boundary = extractBoundary(request);
	if (boundary.empty()) {
		throw std::runtime_error("400 Bad Request: Missing boundary in multipart data");
	}
	
	std::string full_boundary = "--" + boundary;
	std::string body = client.client_body;
	
	size_t start_pos = body.find(full_boundary);
	if (start_pos == std::string::npos) {
		throw std::runtime_error("400 Bad Request: Invalid multipart data");
	}
	
	start_pos += full_boundary.length();
	if (body.substr(start_pos, 2) == "\r\n") {
		start_pos += 2;
	}
	
	size_t end_pos = body.find(full_boundary, start_pos);
	//quand c'est une image on n'as pas le dernier boundary
	if (end_pos == std::string::npos) {
		throw std::runtime_error("400 Bad Request: Incomplete multipart data");
	}
	
	std::string file_part = body.substr(start_pos, end_pos - start_pos);
	
	size_t headers_end = file_part.find("\r\n\r\n");
	if (headers_end == std::string::npos) {
		throw std::runtime_error("400 Bad Request: Invalid file part headers");
	}
	
	std::string headers = file_part.substr(0, headers_end);
	std::string file_content = file_part.substr(headers_end + 4);
	
	if (file_content.length() >= 2 && file_content.substr(file_content.length() - 2) == "\r\n") {
		file_content = file_content.substr(0, file_content.length() - 2);
	}
	
	std::string filename = extractFileName(headers);
	if (filename.empty()) {
		throw std::runtime_error("400 Bad Request: No filename provided");
	}
	
	std::string upload_dir = client.server->getRoot() + "/upload/";
	struct stat st;
	if (stat(upload_dir.c_str(), &st) != 0) {
		if (mkdir(upload_dir.c_str(), 0755) != 0) {
			throw std::runtime_error("500 Internal Server Error: Cannot create upload directory");
		}
	}
	
	std::string file_path = upload_dir + filename;
	std::ofstream output_file(file_path.c_str(), std::ios::binary);
	if (!output_file.is_open()) {
		throw std::runtime_error("500 Internal Server Error: Cannot create file");
	}
	
	output_file.write(file_content.c_str(), file_content.length());
	output_file.close();
	
	return true;
}

