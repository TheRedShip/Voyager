/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Voyager.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 16:41:45 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/07 12:04:09 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VOYAGER_H
# define VOYAGER_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/ip.h>
# include <netinet/tcp.h>
# include <errno.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/ioctl.h>
# include <net/if.h>
# include <pthread.h>
# include <stdbool.h>

# define PACKET_SIZE 1024

typedef struct s_scan
{
	const char		*src_ip;
	const char		*start_ip;
	const char		*end_ip;
	unsigned short	src_port;
	unsigned short	dst_port;
}				t_scan;

typedef struct s_receive
{
	t_scan			*scan;
	double			timeout_sec;
	bool			scan_ended;
	bool			(*process_func)(struct tcphdr *, struct sockaddr_in);
}				t_receive;

int voyagerInit();
int voyagerScan(const char *src_ip, const char *dst_ip, unsigned short src_port, unsigned short dst_port);
int voyagerReceive(t_receive *receive);

int receiveSynResponse(int recv_sock, t_receive *receive);
int sendSynPacket(int send_sock, char *packet, const char *src_ip, const char *dst_ip, unsigned short src_port, unsigned short dst_port);
void preBuildSynPacket(char *packet, const char *src_ip, unsigned short dst_port);


// IP UTILS //

uint32_t    ipToInt(const char *ip_str);
void        IntToIp(uint32_t ip, char *buffer);

//

// MC PROTOCOL //


bool processSyn(struct tcphdr *tcph, struct sockaddr_in src_addr);

//

#endif