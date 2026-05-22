#ifndef _MYHEAD_H_
#define _MYHEAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/input.h>  //��������ϵͳģ���йص�ͷ�ļ�
#include <sys/mman.h>  //mmap��ͷ�ļ�
#include <errno.h>  //perror��ͷ�ļ�
#include <dirent.h>  //opendir��ͷ�ļ�
#include <sys/wait.h> //wait��ͷ�ļ�
#include <pthread.h>  //�̴߳����йص�ͷ�ļ�
#include <sys/socket.h>  //�������йص��ļ�
#include <netinet/in.h>
#include <arpa/inet.h>
#define SYNC_MSG_SIZE 1024

struct clientmsg
{
	//数据域
	int sock; //目前连接成功客户端套接字
	char ip[20]; //客户端ip地址
	unsigned short port; //客户端端口号
	
	//指针域
	struct clientmsg *next;
};
void clear_socket_buffer(int sock);
struct clientmsg *client_init();
int insert_tail(struct clientmsg *node,struct clientmsg *head);
int list_delete(char ipbuf[20],unsigned short delport,struct clientmsg *head);
struct clientmsg * find_client(char ip[20], unsigned short port, struct clientmsg *head);
void print_all_client(struct clientmsg * myhead);
void *send_msgto_client(void *arg);
int get_client_count(struct clientmsg * myhead);
void empty_list(struct clientmsg * head);

void send_get_all_online_clients_cmd(int *oc_client_sock_p, struct sockaddr_in * oc_client_addr);
void *recv_servermsg(void *arg);
void recv_clients_info(int *oc_client_sock_p,struct clientmsg * online_clients_ll, char msg_buf[SYNC_MSG_SIZE]);
void c2c_chat_recv_handler(char rbuf[SYNC_MSG_SIZE]);
void c2c_chat_send_handler_debug(int *oc_client_sock_p, struct clientmsg * online_clients);
#endif