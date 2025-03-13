/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minecraft_protocol.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 14:51:29 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/13 16:47:30 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Voyager.h"


int writeVarInt(unsigned int value, unsigned char *buffer) {
	int count = 0;
	while (1) {
		unsigned char temp = value & 0x7F;
		value >>= 7;
		if (value != 0)
			temp |= 0x80;
		buffer[count++] = temp;
		if (value == 0)
			break;
	}
	return count;
}


void processProtocolJson(unsigned char *response, int received, const char *server_ip, unsigned short port)
{
	printf("\rReceived %d bytes from %s:%d\n", received, server_ip, port);
	// write(1, response, received); 

	int i = 0;

	while (i < received)
	{
		if (strncmp((unsigned char *)response + i, "version", 7) == 0)
		{
			i += 8;
			int len = 0;
			while (response[++i] != '}')
				len++;
			write(1, response + i - len, len + 1);
			write(1, "\n", 1);
		}

		if (strncmp((unsigned char *)response + i, "players", 7) == 0)
		{
			i += 8;
			int len = 0;
			while (response[++i] != '}')
				len++;
			write(1, response + i - len, len + 1);
			write(1, "\n", 1);
		}
		
		i++;
	}
	write(1, "\n", 1);
}


bool processSyn(struct tcphdr *tcph, struct sockaddr_in src_addr)
{
	unsigned short port = ntohs(tcph->source);
	const char *server_ip = inet_ntoa(src_addr.sin_addr);

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	struct timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 0.5;
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
		perror("inet_pton");
		close(sock);
		return 0;
	}

	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("connect");
		close(sock);
		return 0;
	}
	unsigned char handshakePayload[256];
	int offset = 0;
	
	offset += writeVarInt(0x00, handshakePayload + offset);
	offset += writeVarInt(754, handshakePayload + offset);

	int addrLen = strlen(server_ip);
	offset += writeVarInt(addrLen, handshakePayload + offset);

	memcpy(handshakePayload + offset, server_ip, addrLen);
	offset += addrLen;

	handshakePayload[offset++] = (port >> 8) & 0xFF;
	handshakePayload[offset++] = port & 0xFF;

	offset += writeVarInt(1, handshakePayload + offset);

	unsigned char handshakePacket[4096];
	int lengthBytes = writeVarInt(offset, handshakePacket);

	memcpy(handshakePacket + lengthBytes, handshakePayload, offset);
	int handshakePacketSize = lengthBytes + offset;

	if (send(sock, handshakePacket, handshakePacketSize, 0) != handshakePacketSize) {
		perror("send handshake");
		close(sock);
		return 0;
	}

	unsigned char statusPayload[2];
	int statusOffset = 0;
	statusOffset += writeVarInt(0x00, statusPayload + statusOffset);
	unsigned char statusPacket[16];
	int statusLengthBytes = writeVarInt(statusOffset, statusPacket);
	memcpy(statusPacket + statusLengthBytes, statusPayload, statusOffset);
	int statusPacketSize = statusLengthBytes + statusOffset;

	if (send(sock, statusPacket, statusPacketSize, 0) != statusPacketSize) {
		perror("send status request");
		close(sock);
		return 0;
	}

	unsigned char response[4096];
	int received = recv(sock, response, sizeof(response) - 1, 0);

	if (received <= 0)
		return (false);

	processProtocolJson(response, received, server_ip, port);

	close(sock);
	
	return (true);
}
