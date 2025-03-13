/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   syn_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 16:41:32 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/07 12:13:23 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Voyager.h"


unsigned short checksum(unsigned short *ptr, int nbytes)
{
	long sum = 0;
	unsigned short oddbyte, answer;

	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1) {
		oddbyte = 0;
		*((unsigned char *) &oddbyte) = *(unsigned char *) ptr;
		sum += oddbyte;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = (unsigned short) ~sum;
	return answer;
}

struct pseudo_header
{
	unsigned int source_address;
	unsigned int dest_address;
	unsigned char placeholder;
	unsigned char protocol;
	unsigned short tcp_length;
};

void preBuildSynPacket(char *packet, const char *src_ip, unsigned short dst_port)
{
	memset(packet, 0, PACKET_SIZE);

	// build IP header
	struct iphdr *iph = (struct iphdr *) packet;
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
	iph->id = htons(54321);
	iph->frag_off = 0;
	iph->ttl = 64;
	iph->protocol = IPPROTO_TCP;
	iph->saddr = inet_addr(src_ip);
	iph->check = 0;

	// build TCP header
	struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof(struct iphdr));
	tcph->dest = htons(dst_port);
	tcph->seq = htonl(0);
	tcph->ack_seq = 0;
	tcph->doff = 5;
	tcph->syn = 1;
	tcph->ack = 0;
	tcph->rst = 0;
	tcph->fin = 0;
	tcph->psh = 0;
	tcph->urg = 0;
	tcph->window = htons(5840);
	tcph->check = 0;
	tcph->urg_ptr = 0;
}

int addInfoSynPacket(char *packet, const char *dst_ip, unsigned short src_port)
{
	struct iphdr *iph = (struct iphdr *) packet;
	
	iph->daddr = inet_addr(dst_ip);
	iph->check = checksum((unsigned short *) packet, sizeof(struct iphdr));
	
	struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof(struct iphdr));
	tcph->source = htons(src_port);


	// pseudo header to compute checksum
	struct pseudo_header psh;
	psh.source_address = iph->saddr;
	psh.dest_address = iph->daddr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(sizeof(struct tcphdr));

	int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
	char *pseudogram = malloc(psize);
	if (!pseudogram) {
		perror("malloc");
		return (0);
	}
	memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));
	tcph->check = checksum((unsigned short *) pseudogram, psize);
	free(pseudogram);
	return (1);
}

int sendSynPacket(int send_sock, char *packet, const char *src_ip, const char *dst_ip, unsigned short src_port, unsigned short dst_port)
{

	if (!addInfoSynPacket(packet, dst_ip, src_port)) {
        fprintf(stderr, "Failed to update packet for destination %s\n", dst_ip);
        return 0;
    }

	struct sockaddr_in dest;
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(dst_ip);
	dest.sin_port = htons(dst_port);

	int packet_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
	if (sendto(send_sock, packet, packet_len, 0,
			   (struct sockaddr *) &dest, sizeof(dest)) < 0) {
		perror("sendto");
		return (0);
	}
	return (1);
}

int receiveSynResponse(int recv_sock, t_receive *receive)
{
	uint32_t start_ip = ipToInt(receive->scan->start_ip);
	uint32_t end_ip = ipToInt(receive->scan->end_ip);

	unsigned short src_port = receive->scan->src_port;
	unsigned short dst_port = receive->scan->dst_port;
	
	float timeout_sec = receive->timeout_sec;
	int sucess_num = 0;

	struct timeval start, now;

	while (1)
	{
		if (!receive->scan_ended)
			gettimeofday(&start, NULL);

		gettimeofday(&now, NULL);
		double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0;
		if (elapsed >= timeout_sec)
			break ;

		double remaining = timeout_sec - elapsed;

		fd_set read_set;
		FD_ZERO(&read_set);
		FD_SET(recv_sock, &read_set);

		struct timeval tv;
		tv.tv_sec = (int) remaining;
		tv.tv_usec = (remaining - tv.tv_sec) * 1000000;

		int sel = select(recv_sock + 1, &read_set, NULL, NULL, &tv);
		if (sel < 0)
		{
			perror("select");
			break ;
		}
		else if (sel == 0)
			continue ;

		char buffer[PACKET_SIZE];
		struct sockaddr_in src_addr;
		socklen_t addr_len = sizeof(src_addr);
		ssize_t data_size = recvfrom(recv_sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &src_addr, &addr_len);
		if (data_size < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				continue ;
			perror("recvfrom");
			return (-1);
		}
		
		uint32_t host_ip = ntohl(src_addr.sin_addr.s_addr);
		if (host_ip < start_ip || host_ip > end_ip)
			continue ;
		
		struct iphdr *iph = (struct iphdr *) buffer;
		int ip_header_len = iph->ihl * 4;
		if (data_size < ip_header_len + sizeof(struct tcphdr))
			continue ;
		struct tcphdr *tcph = (struct tcphdr *) (buffer + ip_header_len);
		
		// if (tcph->dest != htons(src_port))
		// 	continue ;
		
		if (tcph->syn && tcph->ack)
		{
			if (!receive->process_func || receive->process_func(tcph, src_addr))
				sucess_num++;
		}
		else if (tcph->rst)
			continue ;
	}

	return (sucess_num);
}