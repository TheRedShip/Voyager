/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 16:42:08 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/07 12:16:53 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Voyager.h"

void scanRange(const char *start_ip, const char *end_ip, unsigned short dst_port)
{
	char ip_str[32];
	uint32_t start = ipToInt(start_ip);
	uint32_t end = ipToInt(end_ip);

	unsigned short src_port = 54321;

	for (uint32_t ip = start; ip <= end; ip++)
	{
		IntToIp(ip, ip_str);
		printf("Scanning %s on port %d\n", ip_str, dst_port);

		voyagerScan("192.168.201.97", ip_str, src_port + (ip - start), dst_port);
		usleep(1000);
	}


	int result = voyagerReceive(start, end, src_port, dst_port, 10.0);
	printf("%d\n", result);
	// if (result > 0)
	// 	printf("Port %d on %s is OPEN (SYN-ACK received) %d times.\n", dst_port, dst_ip, result);
	// else if (result == 0)
	// 	printf("Port %d on %s is CLOSED (RST received).\n", dst_port, dst_ip);
	// else
	// 	printf("No valid response received from %s:%d within %.1f second(s).\n", dst_ip, dst_port, timeout_sec);


}

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		printf("Usage: %s <start_ip> <end_ip> <dst_port>\n", argv[0]);
		return (1);
	}

	char *start_ip = argv[1];
	char *end_ip = argv[2];
	unsigned short dst_port = atoi(argv[3]);

	const char *src_ip = "192.168.201.97";

	double timeout_sec = 0.1;

	if (!voyagerInit())
		return (1);

	
	scanRange(start_ip, end_ip, dst_port);
	
	// for (int i = 0; i < 100; i++)
	// 	voyagerScan(src_ip, dst_ip, src_port + i, dst_port);

	return 0;
}
