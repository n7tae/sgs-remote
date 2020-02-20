#pragma once

/*
 *   Copyright (C) 2020 by Thomas A. Early N7TAE
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

class CTLSClient
{
public:
	CTLSClient() : m_sock(-1) , m_ctx(NULL), m_ssl(NULL) {}
	~CTLSClient();
	bool Open(const char *address, unsigned short port);
	void SendCommand(const char *password, const char *command);
	int Read(char *buf, int size);
protected:
	bool CreateContext(const SSL_METHOD *method);
	virtual bool CreateSocket();

	int m_sock;
	SSL_CTX *m_ctx;
	SSL *m_ssl;
	std::string m_address;
	unsigned short m_port;
};
