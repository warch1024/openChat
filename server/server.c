#include "myhead.h"  //我自定义的头文件，里面包含了所有常用的头文件

//开发板作为tcp客户端
/*
	多个客户端连接服务器
*/   
pthread_t send_msg_thread_id;	//发消息的线程id
pthread_mutex_t mutex;  //互斥锁
pthread_cond_t cond;    //条件变量
static int connect_flag = 0;
static int reconnect_create_thread_flag = 0;
//定义结构体存放连接成功的客户端信息


static struct clientmsg *all_ol_clients_ll; //全局变量，指向存储的所有连接到此服务器的客户端的信息
// 清空套接字接收缓冲区的函数


void send_file_handler(int sock, char* rec_cmd_buf){
	char file_path[100] = {0};
	bzero(file_path, 100);
	char *file_name = strtok(rec_cmd_buf, " ");
	file_name = strtok(NULL, " ");	//获取要求发送的文件名

	sprintf(file_path, "./%s", file_name);	//发送服务器所在目录的文件
	//debug
	printf("debug 文件名：%s\n",file_path);
	struct stat f_stat;	//获取文件的大小
	stat(file_path, &f_stat);	//获取文件的状态属性
	long file_size = f_stat.st_size;
	//debug
	printf("debug 文件大小：%ld\n",file_size);
	
	int fd = open(file_path, O_RDONLY);	//打开文件，准备读取以发送

	char read_buf[128] = {0};
	//先发一串命令
	char cmd_buf[100] = {0};
	bzero(cmd_buf, sizeof(cmd_buf));
	sprintf(cmd_buf, "file$%ld$%s$", file_size, file_name);	//发送给客户端文件的属性
	//debug
	printf("debug 发送给客户端的命：%s\n",cmd_buf);
	send(sock, cmd_buf, sizeof(read_buf), 0);	//先发给客户端一条命令，告知文件大小等信息，使其准备接受
	int send_k = (file_size + 127) /128;	//计算发送文件的块数
	for(int i=0; i<send_k; i++){

		bzero(read_buf, 128);	//清空缓存
		int read_size = read(fd, read_buf, 128);//读取文件
		//debug
		printf("debug 读取了%d字节文件：%s\n",read_size, read_buf);
		int send_size = send(sock, read_buf, sizeof(read_buf), 0);	//发送读取的一段数据
		//debug
		printf("debug 发送了%d字节文件内容\n",send_size);
	}
	close(fd);
}

int forward_file_handler(int self_sock, char msg[SYNC_MSG_SIZE]){	//接收文件的处理函数
	//msg的内容是源ip@源port@文件名@文件大小@目的ip@目的port
	clear_socket_buffer(self_sock);	//清空接收缓存套接字
	char sync_file_info[SYNC_MSG_SIZE] = {0};	//要发送的同步消息
	bzero(sync_file_info, sizeof(sync_file_info));
	sprintf(sync_file_info, "send$file#%s", msg);	//要发送的同步消息
	//debug
	printf("debug 转发文件信息命令：%s\n",sync_file_info);
	char *src_ip_s = strtok(msg, "@");	//获取源ip
	char *src_port_s = strtok(NULL, "@");	//获取源port
	char * src_file_name_s = strtok(NULL, "@");	//获取转发的文件的名字
	char *file_size_s = strtok(NULL, "@");	//获取文件名
	char * des_ip_s = strtok(NULL, "@");	//获取目标客户端的ip
	char * des_port_s = strtok(NULL, "@");	//获取目标客户端的端口
	unsigned short des_port_u = atoi(des_port_s);	//将端口号转换为无符号短整数
	long file_size = strtol(file_size_s, NULL, 0);	//获取要接受多少kb数据
	//debug
	printf("debug 转发的文件大小：%ld\n",file_size);	
	//debug
	printf("debug 转发的文件名：%s\n",src_file_name_s);	
	//获取当前文件的绝对路径

	//获取目标客户端的信息
	struct clientmsg * des_client = find_client(des_ip_s, des_port_u, all_ol_clients_ll);
	if(des_client == all_ol_clients_ll){	//目标客户端不存在
		printf("目标客户端已下线\n");
		return -1;
	}
	int des_client_sock = des_client->sock;	//获取目标客户端的套接字
	//转发命令
	send(des_client_sock, sync_file_info, sizeof(sync_file_info), 0);	//先发给客户端一条命令，告知文件大小等信息，使其准备接受
	usleep(5);	//等待客户端准备接受
	//之后将接收的数据转发出去
	char sbuf[SYNC_MSG_SIZE];
	int write_k = file_size  / sizeof(sbuf) + 1, count = 0;
	long real_send_size = 0;
	while(write_k--){	//转发收到的文件数据
		bzero(sbuf, sizeof(sbuf));
		int ret = recv(self_sock, sbuf,sizeof(sbuf),0);	//接收数据
		//debug
		int size = send(des_client_sock, sbuf, sizeof(sbuf), 0);	//写入文件
		printf("debug rf客户端转发：%d字节\n",size);
		//debug
		real_send_size += size;
		usleep(5);	//等待客户端准备接受,开发板需要等待
		//debug
		count ++;
		printf("debug rf客户端转发：%d次\n",count);
	}
	printf("debug rf客户端转发完成,共转发%d次，%ld字节\n",count, real_send_size);
	usleep(1000);	//等待10ms,开发板需要等待
	clear_socket_buffer(self_sock);	//清空接收缓存套接字
}
//发送所有已连接客户端信息给客户端,配合client_requests_handler()
int send_all_clients_info_handler(int des_client_sock){
	
	char client_info_buf[SYNC_MSG_SIZE] = {0};
	//先发一串命令
	char cmd_buf[SYNC_MSG_SIZE] = {0};
	bzero(cmd_buf, sizeof(cmd_buf));
	//每个客户端的信息固定长度clients$info#10@10240
	int send_k = get_client_count(all_ol_clients_ll);	//几个客户端就发几次
	sprintf(cmd_buf, "clients$info#%d@%ld", send_k,
			 (long)send_k*sizeof(client_info_buf));	//发送给客户端文件的属性
	//debug
	printf("debug——发送给客户端的命：%s\n",cmd_buf);
	//发送命令：clients$info#10@10240
	send(des_client_sock, cmd_buf, sizeof(cmd_buf), 0);	//先发给客户端一条命令，告知文件大小等信息，使其准备接受
	usleep(5);	//等待客户端准备接受
	//开始发客户端信息
	if(all_ol_clients_ll && all_ol_clients_ll->next){
		struct clientmsg * tmp = all_ol_clients_ll->next;
		for(int i=0; i<send_k && tmp != NULL; i++){

			bzero(client_info_buf, sizeof(client_info_buf));	//清空缓存
			sprintf(client_info_buf, "client#%s@%hu", tmp->ip, tmp->port);
			//debug
			printf("debug——发送给客户端的信息：%s\n",client_info_buf);
			//发送的信息为client#ip@port
			int send_size = send(des_client_sock, client_info_buf, sizeof(client_info_buf), 0);	//发送读取的一段数据
			usleep(5);	//等待客户端准备接受,开发板需要等待
			//debug
			printf("debug——发送了%d字节文件内容\n",send_size);
			tmp = tmp->next;	//发送下一个客户端信息
		}
	}
	else{
		perror("链表为空，无法发送给客户端已连接信息");
		return -1;
	}
}
//单聊服务器服务函数转发
void c2c_chat_forward_handler(char rbuf[SYNC_MSG_SIZE], int sock){
	//消息：源ip@port@msg@目的ip@port
	//debug
	printf("debug 服务器收到的消息：%s\n", rbuf);
	char *source_ip_s = strtok(rbuf, "@");	//得到源ip
	char *source_port_s = strtok(NULL, "@");	//得到源port
	char *msg_s = strtok(NULL, "@");	//得到消息本体
	char *des_ip_s = strtok(NULL, "@");	//获得目标ip
	char *des_port_s = strtok(NULL, "@");	//获得目标端口
	//找到目标地址的socket
	struct clientmsg * des_client = find_client(des_ip_s, (uint16_t)atoi(des_port_s), all_ol_clients_ll);
	//debug
	printf("debug 服务器找到的目标客户端ip：%s  端口号：%hu\n", des_client->ip, des_client->port);
	if(des_client){	//找到了客户端
		char sbuf[SYNC_MSG_SIZE] = {0};
		bzero(sbuf, sizeof(sbuf));
		sprintf(sbuf, "c2c$chat#%s@%s@%s", source_ip_s, source_port_s, msg_s);	//无需发送目标ip和port
		send(des_client->sock, sbuf, sizeof(sbuf), 0);	//将发来的信息进行转发
		//debug
		printf("服务器转发的消息：%s\n", sbuf);
	}
	
}
//实际的客户端请求处理函数处理4类请求命令
int client_requests_handler(char rbuf[SYNC_MSG_SIZE], struct clientmsg *client_info){	//处理4个请求命令,传入接收缓存和客户端信息
	//处理客户端发送过来的命令
	//debug
	printf("debug 服务器收到的消息：%s\n", rbuf);
	char *head_cmd_buf = strtok(rbuf, "#");	//获取头部指令
	//获取客户端发来的消息实体
	char msg_buf[SYNC_MSG_SIZE] = {0};
	bzero(msg_buf, sizeof(msg_buf));
	strcpy(msg_buf, strtok(NULL, "#"));	//获取剩余消息
	//判断通信功能
	if(strcmp(head_cmd_buf, "get$all$online$clients") == 0){	//如果服务器要发送文件 file$1024$filename.txt$@客户端ip@port
		
		send_all_clients_info_handler(client_info->sock);	//发送所有的客户端给客户端
		return 1;
	}
	else if(strcmp(head_cmd_buf,"c2c$chat") == 0){
		printf("转发消息%s\n", msg_buf);
		c2c_chat_forward_handler(msg_buf, client_info->sock);	//单独聊天处理函数
	}
	else if(strcmp(head_cmd_buf,"send$file") == 0){
		//msg的内容是源ip@源port@文件名@文件大小@目的ip@目的port
		printf("转发文件\n");
		forward_file_handler(client_info->sock, msg_buf);	//文件转发处理函数
	}
	else if(strcmp(head_cmd_buf,"send$emoticon$") == 0){
		printf("发表情包\n");
	}
		
}

//服务器用于接收某个连接成功的客户端发来的数据处理函数
void *recv_client_msg(void *arg){
	int ret;
	struct clientmsg *client_info=(struct clientmsg *)arg;	//传入参数为客户端tcp的地址等信息
	char rbuf[SYNC_MSG_SIZE];	//接收缓存
	clear_socket_buffer(client_info->sock);	//清空此客户端的sock缓存
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	//设置立即取消此线程
	while(1)
	{
		bzero(rbuf,sizeof(rbuf));
		//不断接收此客户端发来的消息
		ret=recv(client_info->sock,rbuf,sizeof(rbuf),0);	//每次接收固定长度信息
		if(ret==0){
			printf("客户端ip %s  端口号%hu断开了\n",client_info->ip,client_info->port);
			//从单链表中删除这个断开的客户端
			list_delete(client_info->ip,client_info->port, all_ol_clients_ll);
			//退出当前线程
			pthread_exit(NULL);
		}
		else if(ret< 0){
			printf("服务器接收客户端ip %s  端口号%hu消息失败\n",client_info->ip,client_info->port);
			continue;
		}
		else if(ret < SYNC_MSG_SIZE){
			printf("服务器接收客户端ip %s  端口号%hu消息不完整\n",client_info->ip,client_info->port);
			continue;
		}
		else if(strcmp(rbuf,"quit")==0){
			exit(0);	//结束进程
		}
		else{
			int recv_type = client_requests_handler(rbuf, client_info);	//处理4类请求
			if(recv_type == 0){	//普通消息

				printf("客户端ip %s  端口号%hu给我发送的信息是： %s\n",client_info->ip,client_info->port,rbuf);
			}
		}
		
	}
}



//服务器发送消息给客户端，单独线程
void *send_msgto_client(void *arg)
{
	char ipbuf[20];
	char msbuf[100];
	unsigned short someport;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	//设置立即取消此线程
	while(1)
	{
		bzero(ipbuf,20);
		bzero(msbuf,100);
		//遍历当前这个链表，把目前在线的客户端信息打印一遍，方便我选择究竟要跟谁聊天
		print_all_client(all_ol_clients_ll);	//打印所有已连接客户端

		while(all_ol_clients_ll->next == NULL){	//插入新客户端会解除此处的阻塞
			if(!connect_flag){
				printf("等待客户端连接...\n");
				connect_flag = 1;
			}
		}
		printf("请输入你要跟哪个客户端聊天，输入这个客户端ip和端口号!\n");
		scanf("%s",ipbuf);
		scanf("%hu",&someport);
		printf("请输入要给这个客户端发生的信息!\n");
		fgets(msbuf, sizeof(msbuf), stdin);
		msbuf[strcspn(msbuf, "\n")] = '\0';  // 去除末尾的换行符
		//scanf("%s",msbuf);
		//把这个信息发送给对应的客户端

		//遍历链表找到你选择的那个客户端
		struct clientmsg *p = find_client(ipbuf, someport, all_ol_clients_ll);	//获取已连接客户端tcp信息
		
		send(p->sock,msbuf,strlen(msbuf),0);	//发送信息给客户端
	}
}
			
int main()
{
	int ret;
	pthread_t new_client_id;

	int server_sock;
	int newsock; 
	char rbuf[100];
	
	//服务器tcp对象
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family=AF_INET;  //地址协议，IPV4
	server_addr.sin_addr.s_addr=inet_addr("192.168.11.88");    //绑定服务器自己的ip地址
	server_addr.sin_port=htons(30000);  //服务器要绑定的端口号
	
	
	//定义线程属性变量并初始化
	pthread_attr_t myattr;
	pthread_attr_init(&myattr);
	
	//调用设置分离属性的函数
	pthread_attr_setdetachstate(&myattr,PTHREAD_CREATE_DETACHED); //创建的所有线程可分离，无需手动回收
	

	//设置服务器的tcp相关对象
	server_sock = socket(AF_INET,SOCK_STREAM,0);
	if(server_sock==-1)
	{
		perror("买手机失败(创建tcp套接字)失败!\n");
		return -1;
	}
	
	//设置端口复用
	int on=1; //非0即可,是个开关，开启端口复用功能
	setsockopt(server_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	
	//绑定服务器ip地址和端口号
	ret=bind(server_sock,(struct sockaddr *)&server_addr,sizeof(server_addr));
	if(ret==-1)
	{
		perror("绑定ip和端口号失败了!\n");
		return -1;
	}
	
	//监听服务器的服务端口
	ret=listen(server_sock,5);
	if(ret==-1)
	{
		perror("监听失败了!\n");
		return -1;
	}
	
	system("clear");

	//初始化保存客户端信息的链表的头结点
	all_ol_clients_ll=client_init();

	//创建一个线程：用于从键盘输入内容发送给对应的客户端
	pthread_create(&send_msg_thread_id, NULL,send_msgto_client,NULL);

	//定义ipv4地址结构体变量保存连接成功的那个客户端信息
	struct sockaddr_in clientaddr;
	int size = sizeof(clientaddr);
	
	while(1)
	{
		//接收客户端的连接请求，产生新的套接字
		newsock=accept(server_sock,(struct sockaddr *)&clientaddr,&size);
		//printf("目前连接成功的那个客户端ip是: %s  端口号是: %hu\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
		if(newsock != -1){	//客户端连接成功
			//准备新的节点,用于保存到链表中
			struct clientmsg *newnode=malloc(sizeof(struct clientmsg));
			newnode->sock=newsock;  //存放套接字
			bzero(newnode->ip, sizeof(newnode->ip));
			strcpy(newnode->ip,inet_ntoa(clientaddr.sin_addr)); //存放ip地址
			newnode->port=ntohs(clientaddr.sin_port); //存放端口号
			newnode->next=NULL;
			
			//客户端的信息尾插到链表
			insert_tail(newnode,all_ol_clients_ll);
			
			//立马新建一个线程(分离的)，该线程专门负责接收这个客户端发送过来的信息，
			pthread_create(&new_client_id,&myattr,recv_client_msg, newnode);
			
			//新连接后打印所有已连接的客户端信息
			print_all_client(all_ol_clients_ll);
			printf("请输入你要跟哪个客户端聊天，输入这个客户端ip和端口号!\n");
		}
	}
	//挂机
	close(server_sock);
	// close(newsock);
	return 0;
}