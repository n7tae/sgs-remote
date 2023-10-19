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

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

#include "TLSClient.h"

CTLSClient::~CTLSClient() {
	if (m_ssl)
		SSL_free(m_ssl);
	if (m_sock >= 0)
		close(m_sock);
	if (m_ctx)
		SSL_CTX_free(m_ctx);
}

bool CTLSClient::CreateContext(const SSL_METHOD *method)
{
	m_ctx = SSL_CTX_new(method);
    if (!m_ctx) {
		fprintf(stderr, "Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		return true;
    }
	return false;
}


bool CTLSClient::Open(const char *address, int unsigned short port)
{
	m_address.assign(address);
	m_port = port;

	return CreateSocket();
}

void CTLSClient::SendCommand(const char *password, const char *command)
{
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	const SSL_METHOD *method = TLS_client_method();

	if (CreateContext(method)) {
		fprintf(stderr, "Unable to create client context\n");
		return;
	}

	m_ssl = SSL_new(m_ctx);
	SSL_set_fd(m_ssl, m_sock);
	int ret = SSL_connect(m_ssl);
	if (ret < 1) {
		fprintf(stderr, "SSL_get_error = %d\n", SSL_get_error(m_ssl, ret));
		return;
	}

	//printf("Using %s encryption\n", SSL_get_cipher(m_ssl));

	int size = (int)strlen(password);
	if (size != SSL_write(m_ssl, password, size)) {
		fprintf(stderr, "couldn't write the password\n");
		return;
	}

	char buf[128];
	if (4 != SSL_read(m_ssl, buf, 128)) {
		fprintf(stderr, "couldn't get password response\n");
		return;
	}
	if (memcmp(buf, "pass", 4)) {
		fprintf(stderr, "Password validation failed!\n");
		return;
	}

	size = (int)strlen(command);
	if (size != SSL_write(m_ssl, command, size)) {
		fprintf(stderr, "could not set command!\n");
		return;
	}
}

bool CTLSClient::CreateSocket()
{
	struct sockaddr_storage addr;
	memset(&addr, 0, sizeof(struct sockaddr_storage));

	int family;
	if (m_address.npos != m_address.find(':')) {
		struct sockaddr_in6 *a = (struct sockaddr_in6 *)&addr;
		a->sin6_family = family = AF_INET6;
		a->sin6_port = htons(m_port);
		inet_pton(AF_INET6, m_address.c_str(), &(a->sin6_addr));
	} else if (m_address.npos != m_address.find('.')) {
		struct sockaddr_in *a = (struct sockaddr_in *)&addr;
		a->sin_family = family = AF_INET;
		a->sin_port = htons(m_port);
		inet_pton(AF_INET, m_address.c_str(), &(a->sin_addr));
	} else {
		fprintf(stderr, "Improper address [%s], remote socket creation failed!\n", m_address.c_str());
		return true;
	}

	//printf("Create %s TCP socket on %u port\n", (AF_INET == family) ? "AF_INET" : "AF_INET6", m_port);

	m_sock = socket(family, SOCK_STREAM, 0);
	if (m_sock < 0) {
		fprintf(stderr, "Unable to create socket, %s\n", strerror(errno));
		return true;
	}

	socklen_t size = (AF_INET == family) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

	if (connect(m_sock, (struct sockaddr *)&addr, size)) {
		fprintf(stderr, "connect() failed, cannot create socket, %s\n", strerror(errno));
		close(m_sock);
		return true;
	}

	return false;
}

int CTLSClient::Read(char *buf, int size)
{
	return SSL_read(m_ssl, buf, size);
}
