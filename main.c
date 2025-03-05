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

unsigned short checksum(unsigned short *ptr, int nbytes) {
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

int send_syn_packet(int send_sock, const char *src_ip, const char *dst_ip, unsigned short src_port, unsigned short dst_port)
{
	char packet[4096];
	memset(packet, 0, sizeof(packet));

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
	iph->daddr = inet_addr(dst_ip);
	iph->check = 0;
	iph->check = checksum((unsigned short *) packet, sizeof(struct iphdr));

	// build TCP header
	struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof(struct iphdr));
	tcph->source = htons(src_port);
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
		return -1;
	}
	memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));
	tcph->check = checksum((unsigned short *) pseudogram, psize);
	free(pseudogram);

	struct sockaddr_in dest;
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(dst_ip);
	dest.sin_port = htons(dst_port);

	int packet_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
	if (sendto(send_sock, packet, packet_len, 0,
			   (struct sockaddr *) &dest, sizeof(dest)) < 0) {
		perror("sendto");
		return -1;
	}
	return 0;
}

int receive_syn_response(int recv_sock, const char *dst_ip, unsigned short src_port, unsigned short dst_port, double timeout_sec)
{
	int sucess_num = 0;

	struct timeval start, now;
	gettimeofday(&start, NULL);

	while (1)
	{
		gettimeofday(&now, NULL);
		double elapsed = (now.tv_sec - start.tv_sec) +
						 (now.tv_usec - start.tv_usec) / 1000000.0;
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

		char buffer[4096];
		struct sockaddr_in src_addr;
		socklen_t addr_len = sizeof(src_addr);
		ssize_t data_size = recvfrom(recv_sock, buffer, sizeof(buffer), 0,
									 (struct sockaddr *) &src_addr, &addr_len);
		if (data_size < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				continue ;
			perror("recvfrom");
			return (-1);
		}

		if (src_addr.sin_addr.s_addr != inet_addr(dst_ip))
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
			char ip[30];
  			strcpy(ip, (char*)inet_ntoa((struct in_addr)src_addr.sin_addr));
			sucess_num++;
			continue ;
			// return (1);
		}
		else if (tcph->rst)
			return (0);
	}

	printf("Port %d on %s is OPEN (SYN-ACK received). %d times\n", dst_port, dst_ip, sucess_num);
	return (sucess_num > 0);
}

int main(int argc, char **argv) {
	const char *src_ip = "192.168.201.140";
	const char *dst_ip = argc == 1 ? "89.33.12.106" : argv[1];
	unsigned short src_port = 12345;
	unsigned short dst_port = 25565;
	double timeout_sec = 1.0;

	int send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (send_sock < 0) {
		perror("socket send");
		exit(EXIT_FAILURE);
	}
	int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (recv_sock < 0) {
		perror("socket recv");
		close(send_sock);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < 1000; i++)
	{
		if (send_syn_packet(send_sock, src_ip, dst_ip, src_port + i, dst_port) != 0) {
			fprintf(stderr, "Failed to send SYN packet.\n");
			close(send_sock);
			close(recv_sock);
			exit(EXIT_FAILURE);
		}
	}


	int result = receive_syn_response(recv_sock, dst_ip, src_port, dst_port, timeout_sec);
	if (result == 1)
		printf("Port %d on %s is OPEN (SYN-ACK received).\n", dst_port, dst_ip);
	else if (result == 0)
		printf("Port %d on %s is CLOSED (RST received).\n", dst_port, dst_ip);
	else
		printf("No valid response received from %s:%d within %.1f second(s).\n", dst_ip, dst_port, timeout_sec);

	close(send_sock);
	close(recv_sock);
	return 0;
}
