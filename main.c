/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 16:42:08 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/06 17:07:12 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Voyager.h"


int main(int argc, char **argv) {
	const char *src_ip = "192.168.201.97";
	const char *dst_ip = argc == 1 ? "89.33.12.4" : argv[1];
	unsigned short src_port = 54321;
	unsigned short dst_port = 25565;
	double timeout_sec = 0.1;

	int send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (send_sock < 0)
	{
		perror("socket send");
		exit(EXIT_FAILURE);
	}
	int one = 1;
	if (setsockopt(send_sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (recv_sock < 0)
	{
		perror("socket recv");
		exit(EXIT_FAILURE);
	}

	char packet[4096];
	preBuildSynPacket(packet, src_ip, dst_port);

	if (!sendSynPacket(send_sock, packet, src_ip, dst_ip, src_port, dst_port))
	{
		fprintf(stderr, "Failed to send SYN packet.\n");
		close(send_sock);
		close(recv_sock);
		exit(EXIT_FAILURE);
	}

	int result = receiveSynResponse(recv_sock, dst_ip, src_port, dst_port, timeout_sec);
	if (result > 0)
		printf("Port %d on %s is OPEN (SYN-ACK received) %d times.\n", dst_port, dst_ip, result);
	else if (result == 0)
		printf("Port %d on %s is CLOSED (RST received).\n", dst_port, dst_ip);
	else
		printf("No valid response received from %s:%d within %.1f second(s).\n", dst_ip, dst_port, timeout_sec);

	close(send_sock);
	close(recv_sock);
	return 0;
}
