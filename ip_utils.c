/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ip_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/07 11:51:34 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/07 11:52:50 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Voyager.h"

uint32_t ipToInt(const char *ip_str)
{
    struct in_addr addr;
    if (inet_aton(ip_str, &addr) == 0)
    {
        fprintf(stderr, "Invalid IP address: %s\n", ip_str);
        return (0);
    }
    return (ntohl(addr.s_addr));
}

void IntToIp(uint32_t ip, char *buffer)
{
    struct in_addr addr;
    addr.s_addr = htonl(ip);
    strcpy(buffer, inet_ntoa(addr));
}