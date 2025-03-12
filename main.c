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

void *scanRange(t_scan *scan)
{
	char ip_str[32];
	uint32_t start = ipToInt(scan->start_ip);
	uint32_t end = ipToInt(scan->end_ip);

	const char *src_ip = scan->src_ip;
	unsigned short src_port = scan->src_port;
	unsigned short dst_port = scan->dst_port;

	int range = end - start + 1;
	printf("Scanning %d hosts\n", range);

	for (uint32_t ip = start; ip <= end; ip++)
	{
		IntToIp(ip, ip_str);
		//pretty show with \r printf
		printf("\r%d / %d", (ip - start), range, ip_str);
		fflush(stdout);

		voyagerScan(src_ip, ip_str, src_port + (ip - start), dst_port);
		usleep(2000);
	}
}

int startScan(const char *src_ip, const char *start_ip, const char *end_ip, unsigned short src_port, unsigned short dst_port)
{
	pthread_t		scan_thread;
	pthread_t		receive_thread;
	t_scan *scan;
	
	scan = malloc(sizeof(t_scan));
	
	scan->src_ip = src_ip;
	scan->start_ip = start_ip;
	scan->end_ip = end_ip;
	scan->src_port = src_port;
	scan->dst_port = dst_port;

	if (pthread_create(&scan_thread, NULL, scanRange, scan))
	{
		perror("pthread_create");
		return (0);
	}
	// scanRange(scan);

	int result = voyagerReceive(start_ip, end_ip, src_port, dst_port, 0.5);
	printf("\n%d\n", result);

	return (result);
}

char *getLocalIp()
{
	int	fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	snprintf(ifr.ifr_name, IFNAMSIZ, "eth0");

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	return (strdup(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)));
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
	const char *src_ip = getLocalIp();
	
	printf("Using source IP: %s\n", src_ip);

	unsigned short src_port = 54321;
	unsigned short dst_port = atoi(argv[3]);
	
	if (!voyagerInit())
		return (1);

	startScan(src_ip, start_ip, end_ip, src_port, dst_port);

	return 0;
}
