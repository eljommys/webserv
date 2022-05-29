#pragma once

#include "webserv.hpp"

struct Status
{
	int			code;
	std::string	msg;

	Status(int code, std::string msg);
	Status();
};

class	Response
{
	private:
		//Status line
		std::string		protocol; //	Usually HTTP/1.1
		Status	status;

		//Headers
		//std::string	cnt_encoding;
		std::string	cnt_type; //	Usually text/html; charset=utf-8
		int			cnt_length;

		//Body
		std::string	body;

		Status	_checkStatus(std::string file_route, std::string pwd);

	public:
		Response(std::string file_route, std::string pwd);
		~Response();

		std::string		get();
};
