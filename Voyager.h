/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Voyager.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 16:41:45 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/06 16:42:55 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VOYAGER_H
# define VOYAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <sys/time.h>

int receiveSynResponse(int recv_sock, const char *dst_ip, unsigned short src_port, unsigned short dst_port, double timeout_sec);
int sendSynPacket(int send_sock, char *packet, const char *src_ip, const char *dst_ip, unsigned short src_port, unsigned short dst_port);
void preBuildSynPacket(char *packet, const char *src_ip, unsigned short dst_port);


#endif