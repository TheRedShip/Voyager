/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   voyager.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/07 11:13:37 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/07 12:06:27 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Voyager.h"

int send_sock;
int recv_sock;

int voyagerInit()
{
    send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (send_sock < 0)
	{
		perror("socket send");
		return (0);
	}
	int one = 1;
	if (setsockopt(send_sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0)
	{
		perror("setsockopt");
		return (0);
	}

	recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (recv_sock < 0)
	{
		perror("socket recv");
		return (0);
	}

    return (1);
}

int voyagerScan(const char *src_ip, const char *dst_ip, unsigned short src_port, unsigned short given_dst_port)
{
	static char packet[PACKET_SIZE];
    static unsigned short dst_port = 0;
    
    if (!dst_port)
    {
        dst_port = given_dst_port;
        preBuildSynPacket(packet, src_ip, dst_port);
    }

	if (!sendSynPacket(send_sock, packet, src_ip, dst_ip, src_port, dst_port))
	{
		fprintf(stderr, "Failed to send SYN packet.\n");
		close(send_sock);
		close(recv_sock);
		return (0);
	}

    
    // close(send_sock);
	// close(recv_sock);
    
    return (1);
}

int voyagerReceive(const char *start_ip, const char *end_ip, unsigned short src_port, unsigned short dst_port, double timeout_sec)
{
	uint32_t start = ipToInt(start_ip);
	uint32_t end = ipToInt(end_ip);

	int result = receiveSynResponse(recv_sock, start, end, src_port, dst_port, timeout_sec);
    return (result);
}