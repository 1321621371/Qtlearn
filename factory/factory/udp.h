#ifndef UDP_H_
#define UDP_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UDP_PORT	(60008)
#define MAX_DATA_SIZE   (512)

int create_udp_server(void);
void destroy_udp_server(void);
void *udp_recv_loop(void* param);
int send_udp_data(char *data, int size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
