/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 16:42:08 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/07 11:30:58 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Voyager.h"


int main(int argc, char **argv) {
	const char *src_ip = "192.168.201.97";
	const char *dst_ip = argc == 1 ? "89.33.12.4" : argv[1];
	unsigned short src_port = 54321;
	unsigned short dst_port = 25565;
	double timeout_sec = 0.1;

	if (!voyagerInit())
		return (1);
	
	for (int i = 0; i < 100; i++)
		voyagerScan(src_ip, dst_ip, src_port + i, dst_port);

	int result = voyagerReceive(dst_ip, src_port, dst_port, timeout_sec);

	if (result > 0)
		printf("Port %d on %s is OPEN (SYN-ACK received) %d times.\n", dst_port, dst_ip, result);
	else if (result == 0)
		printf("Port %d on %s is CLOSED (RST received).\n", dst_port, dst_ip);
	else
		printf("No valid response received from %s:%d within %.1f second(s).\n", dst_ip, dst_port, timeout_sec);

	return 0;
}
