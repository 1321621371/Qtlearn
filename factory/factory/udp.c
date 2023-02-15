#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "common.h"
#include "udp.h"
#include "packet.h"

static int g_sock = -1;
static struct sockaddr_in *g_host_addr;	//上位机地址


int create_udp_server(void)
{
	struct sockaddr_in addr;

        g_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (g_sock < 0){
                LOGE("socket error.\n");
                return -1;
        }

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(UDP_PORT);
	if (bind(g_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		LOGE("bind socket error.\n");
		close(g_sock);
		g_sock = -1;
		return -1;
	}

	LOGD("create udp server on %s:%d ok.\n",
			inet_ntoa(addr.sin_addr), 
			ntohs(addr.sin_port));
	return 0;
}


void destroy_udp_server(void)
{
	close(g_sock);
	g_sock = -1;
}


void *udp_recv_loop(void* param)
{
	int packet_size, ret, addr_size;
	struct sockaddr_in addr;
	char packet[MAX_DATA_SIZE];

	UNUSED(param);
	LOGD("start udp listener.\n");
	
	addr_size = sizeof(struct sockaddr_in);

	while (g_sock >= 0) {
		memset(packet, 0, sizeof(packet));
		packet_size = recvfrom(g_sock, &packet, sizeof(packet),
					0, (struct sockaddr*)&addr, &addr_size);
		if (packet_size < 0) {
                        continue;
                }

		LOGD("Receive packet from %s:%d\n",
				inet_ntoa(addr.sin_addr),
				ntohs(addr.sin_port));
		packet[packet_size] = '\0';
		LOGD("Data: [%s], size %d\n" , packet, packet_size);

		g_host_addr = &addr;

		handle_packet(packet, packet_size);
	}

	return (void *)NULL;
}


int send_udp_data(char *data, int size)
{
	int count;

	LOGV("%s size %d\n", __func__, size);

	if (g_sock < 0) {
		LOGE("udp closed.\n");
		return -1;
	}

	if (!data) {
		LOGE("no data.\n");
		return -1;
	}

	//TODO: ensure all data sent
        count = sendto(g_sock, data, size, 0,
			(struct sockaddr*)g_host_addr, sizeof(*g_host_addr));
	//LOGD("count %d\n", count);

	return count;
}
