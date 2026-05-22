#include "myhead.h"  //我自定义的头文件，里面包含了所有常用的头文件
//开发板作为tcp客户端
/*
	设置套接字的属性--》套接字允许端口复用
*/   
static pthread_mutex_t mutex_recv_ol_client;
static pthread_t recv_handler_id;	//接收消息线程的id
static int client_sock;
//char * file_name = "/IOT/projects/26-3-4/1.txt";

static struct clientmsg *online_client_ll; //全局变量，指向存储的所有连接到此服务器的客户端的信息
static struct sockaddr_in client_addr;	//保存此客户端的信息
//函数声明
void recv_all_online_clients_handler(int client_sock);


int recv_file_handler(int self_sock, char msg[SYNC_MSG_SIZE]){	//接收文件的处理函数
	//msg的内容是10240$1.txt@192.1.1.1@10001
	clear_socket_buffer(self_sock);	//清空接收缓存套接字
	char sync_file_info[SYNC_MSG_SIZE] = {0};	//要发送的同步消息
	bzero(sync_file_info, sizeof(sync_file_info));
	sprintf(sync_file_info, "send$file$#%s$", msg);	//要发送的同步消息

	char file_path[200] = {0};	//要保存的文件的存放路径
	char *file_size_s = strtok(msg, "$");	//获取文件名
	char * recv_file_name = strtok(NULL, "@");	//获取接收的文件的名字
	char * des_ip = strtok(NULL, "@");	//获取目标客户端的ip
	char * des_port = strtok(NULL, "@");	//获取目标客户端的端口

	long file_size = strtol(file_size_s, NULL, 0);	//获取要接受多少kb数据
	bzero(file_path, sizeof(file_path));
	//获取当前文件的绝对路径
	char abs_path[200] = {0};
	bzero(abs_path, sizeof(abs_path));
	getcwd(abs_path, sizeof(abs_path));
	sprintf(file_path, "%s/%s",abs_path, recv_file_name);	//组合文件存放路径
	//debug
	printf("debug 客户端要接受的字节数：%ld\n",file_size);
	//debug
	printf("debug 客户端要存储的文件名：%s\n",file_path);
	int fd = open(file_path, O_CREAT|O_TRUNC|O_WRONLY, 0777);	//打开文件并写入
	//debug
	printf("debug 客户端打开的文件描述符：%d\n",fd);
	//获取目标客户端的信息
	//写入文件
	int write_k = (file_size + SYNC_MSG_SIZE-1) / SYNC_MSG_SIZE;
	write_k--;
	char wbuf[SYNC_MSG_SIZE];
	while(write_k--){	//转发收到的文件数据
		bzero(wbuf, sizeof(wbuf));
		int ret = recv(self_sock, wbuf,sizeof(wbuf),0);	//接收数据
		//debug
		printf("debug rf客户端接收到的数据：%s\n",wbuf);
		int size = write(fd, wbuf, sizeof(wbuf));	//写入文件
		//debug
		printf("debug rf客户端转发：%d字节\n",size);
	}
	//最后一次写入数据
	{
	recv(self_sock, wbuf,sizeof(wbuf),0);
	write(fd, wbuf, file_size % SYNC_MSG_SIZE);	//写入最后一次数据
	}
	close(fd);
	printf("debug 文件写入完成\n");
}
//接收单独聊天发来的信息
void c2c_chat_recv_handler(char rbuf[SYNC_MSG_SIZE]){
	//消息格式：源ip@源port@msg
	char *source_ip_s = strtok(rbuf, "@");	//得到源ip
	char *source_port_s = strtok(NULL, "@");	//得到源port
	char *msg_s = strtok(NULL, "@");	//得到消息本体
	char *des_ip_s = strtok(NULL, "@");	//获得目标ip
	char *des_port_s = strtok(NULL, "@");	//获得目标端口
	printf("%s@%s发来的消息：%s\n", source_ip_s, source_port_s, msg_s);
}
//线程任务函数：接收服务器发送过来的信息
void *recv_servermsg(void *arg)
{
	char rbuf[SYNC_MSG_SIZE];	//最高支持发送1k
	int ret;
	int client_sock = *(int *)arg;	//传入本机的sock
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	//设置立即取消此线程
	while(1)
	{
		bzero(rbuf,sizeof(rbuf)); 
		//接收服务器发送过来的信息
		ret=recv(client_sock,rbuf,sizeof(rbuf),0);
		if(ret==0)
		{
			printf("对方(服务器端)断开连接了!\n");	
			exit(0);
		}
		//判断收到的信息是不是quit
		if(strcmp(rbuf,"quit")==0)
			exit(0);

		//接收文件处理
		char * head_cmd_buf = strtok(rbuf, "#");
		char msg_buf[SYNC_MSG_SIZE] = {0};
		bzero(msg_buf, sizeof(msg_buf));
		strcpy(msg_buf, strtok(NULL, "#"));	//保存转发的消息

		//消息格式：c2c$chat$#源ip@源port@msg
		if(strcmp(head_cmd_buf, "c2c$chat$")== 0){	//单独聊天
			printf("接收到单独聊天消息：%s\n", msg_buf);
			//消息格式：源ip@源port@msg
			c2c_chat_recv_handler(msg_buf);	//接收转发来的消息
		}
		else if(strcmp(head_cmd_buf, "send$file$")== 0){	//接收文件
			
			recv_file_handler(client_sock, msg_buf);	//接收文件

		}
		else if(strcmp(head_cmd_buf, "emoticon")== 0){	//发文件

		}
		
		//正式接收信息
		printf("服务器给我发送的信息是: %s\n",rbuf);
	}
}

//客户端给客户端发送文件处理函数，输入 1.txt@192.1.1.1@10001，将本地1.txt文件发送给目标客户端
void send_file_handler(int sock){
	char file_path[100] = {0};
	bzero(file_path, 100);
	//发送交互
	recv_all_online_clients_handler(sock);//接收所有在线客户端信息
	clear_socket_buffer(sock);
	struct clientmsg * des_client = online_client_ll;
	char *file_name = NULL;
	char send_file_buf[100] = {0};
	label_file_no_exist:
	do{	//循环输入，直到输入的目标地址在线/正确
		printf("请输入要发送的文件名@目标客户端ip@端口号：\n");
		bzero(send_file_buf, sizeof(send_file_buf));
		fgets(send_file_buf, sizeof(send_file_buf), stdin);	//获取用户输入
		send_file_buf[strcspn(send_file_buf, "\n")] = '\0';  // 去除末尾的换行符
		//debug
		printf("debug 输入的字符串：%s\n",send_file_buf);
		//获取发送信息
		file_name = strtok(send_file_buf, "@");	//文件名
		char *des_ip = strtok(NULL, "@");	//获取目标ip
		char *des_port_s = strtok(NULL, "@");	//获取目标端口号
		uint16_t des_port = atoi(des_port_s);	//将端口号转换为整数
		des_client = find_client(des_ip, des_port, online_client_ll);
		//debug
		printf("debug 目标客户端ip：%s，端口：%hu\n",des_ip, des_port);
	}while(des_client == online_client_ll);	//如果目标客户端不存在，继续输入

	sprintf(file_path, "./%s", file_name);	//文件路径
	//debug
	printf("debug 要发送的文件名：%s\n",file_name);
	//获取文件大小
	struct stat f_stat;	//获取文件的大小
	if(stat(file_path, &f_stat) == -1){	//文件不存在
		printf("文件不存在，请重新输入\n");
		goto label_file_no_exist;
	}
	stat(file_path, &f_stat);	//获取文件的状态属性
	long file_size = f_stat.st_size;
	//debug
	printf("debug 文件大小：%ld\n",file_size);
	
	int fd = open(file_path, O_RDONLY);	//打开文件，准备读取以发送

	char read_buf[SYNC_MSG_SIZE] = {0};
	//先发一串命令给服务器
	char cmd_buf[SYNC_MSG_SIZE] = {0};
	bzero(cmd_buf, sizeof(cmd_buf));


	//发送文件命令send$file$#10240$1.txt@192.1.1.1@10001;
	// send$file$#1048$send$file$#1048$$@192.168.11.88@43601
	sprintf(cmd_buf, "send$file$#%ld$%s@%s@%hu", file_size, file_name, des_client->ip, des_client->port);	//发送给客户端文件的属性
	//debug
	printf("debug 发送给服务器的命令：%s\n",cmd_buf);
	send(sock, cmd_buf, sizeof(cmd_buf), 0);	//先发给客户端一条命令，告知文件大小等信息，使其准备接受
	int send_k = (file_size + SYNC_MSG_SIZE - 1) / SYNC_MSG_SIZE;	//计算发送文件的块数
	//发送文件内容
	for(int i=0; i<send_k; i++){

		bzero(read_buf, sizeof(read_buf));	//清空缓存
		int read_size = read(fd, read_buf, sizeof(read_buf));//读取文件
		//debug
		printf("debug 读取了%d字节文件：%s\n",read_size, read_buf);
		int send_size = send(sock, read_buf, sizeof(read_buf), 0);	//发送读取的一段数据
		//debug
		printf("debug 发送了%d字节文件内容\n",send_size);
	}
	close(fd);
	printf("debug 发送文件完成\n");
}


//获取所有服务器在线客户端信息
void recv_all_online_clients_handler(int client_sock){	
	//先发一串命令给服务器
	char clients_info_buf[SYNC_MSG_SIZE] = {0};//用于接收服务器要发送客户端信息的属性
	clear_socket_buffer(client_sock);	//清空接收缓存套接字
	// 取消接收线程
	pthread_cancel(recv_handler_id);
	char self_client_addr[50] = {0};	//本机的ip和端口号
	bzero(self_client_addr, sizeof(self_client_addr));
	//合成本机ip和端口号192.1.1.1@10001
	sprintf(self_client_addr, "%s@%hu", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));	//合成本机ip@port
	
	bzero(clients_info_buf, sizeof(clients_info_buf));
	//合成发送命令get$all$online$clients$#192.1.1.1@10001
	sprintf(clients_info_buf, "get$all$online$clients$#%s", self_client_addr);
	//debug
	printf("debug——客户端合成的发送命令：%s\n", clients_info_buf);
	send(client_sock, clients_info_buf, sizeof(clients_info_buf), 0);	//发送给服务器命令用于获取已连接客户端信息
	//接收已连接客户端的数量信息
	
	usleep(10);	//延时10ms以同步
	bzero(clients_info_buf, sizeof(clients_info_buf));
	//接收服务器发来的前置信息count$10$size$1024
	int ret = recv(client_sock, clients_info_buf, sizeof(clients_info_buf), 0);	//接收客户端信息前置属性 count$10$size$1024
	//debug
	printf("debug——接收到的客户端信息是1：%s\n", clients_info_buf);
	//解析发来的同步头消息count$10$size$1024
	char * clients_count_s = strtok(clients_info_buf, "$");
	clients_count_s = strtok(NULL, "$");	//获取客户端数量
	char * clients_size_s = strtok(NULL, "$");
	clients_size_s = strtok(NULL, "$");	//获取总共发送的字节数
	int client_count = atoi(clients_count_s);	//客户端数量
	long client_size = strtol(clients_size_s, NULL, 0);	//获取总字节数

	empty_list(online_client_ll);	//先清空旧的链表
	//正式接收服务器发来的客户端信息
	usleep(5);
	for(int i = 0; i < client_count; i++){	//接收一个客户端的信息， client@192.1.1.1@10000
		bzero(clients_info_buf, sizeof(clients_info_buf));
		ret = recv(client_sock, clients_info_buf, sizeof(clients_info_buf), 0); 
		//debug
		printf("debug——接收到的客户端地址信息是：%s\n", clients_info_buf);
		//解析发来的信息：client@192.1.1.1@10000
		char *client_ip_s = strtok(clients_info_buf, "@");
		client_ip_s = strtok(NULL, "@");	//获取客户端的ip
		char *client_port_s = strtok(NULL, "@");	//获取此客户端的port
		//准备新的节点,用于保存到链表中
		
		struct clientmsg *newnode = malloc(sizeof(struct clientmsg));
		newnode->sock = -1;  //不存储套接字
		strcpy(newnode->ip, client_ip_s); //存放ip地址
		newnode->port = (uint16_t)atoi(client_port_s); //存放端口号
		newnode->next=NULL;
		insert_tail(newnode,online_client_ll);//一个客户端的信息尾插到链表
		
	}
	//重新新建接收数据线程
	pthread_create(&recv_handler_id,NULL,recv_servermsg,&client_sock);
	//debug
	printf("debug——接收完成s发来的在线c打印链表\n");
	print_all_client(online_client_ll);	//打印所有的已连接客户端的信息
}

//单独聊天
void c2c_chat_send_handler(int sock){
	char des_client_addr[50] = {0};
	char msbuf[900] = {0};
	struct clientmsg * des_client_info = online_client_ll;
	do{
		recv_all_online_clients_handler(sock);	//先同步已连接服务器的客户端
		printf("请输入你要跟哪个客户端聊天，输入这个客户端ip@端口号!\n");
		bzero(des_client_addr, sizeof(des_client_addr));
		//获取目标ip和端口号
		fgets(des_client_addr, sizeof(des_client_addr), stdin);
		msbuf[strcspn(des_client_addr, "\n")] = '\0';  // 去除末尾的换行符
		char *des_ip = strtok(des_client_addr, "@");
		uint16_t des_client = (uint16_t)atoi(strtok(NULL, "@"));
		//debug
		printf("debug——输入的信息是：%s@%hu\n", des_ip, des_client);
		des_client_info = find_client(des_ip,des_client, online_client_ll);	//查找输入的目标是否存在
		//debug
		printf("debug——查找到的客户端信息ll：%pdes ll%p：%s@%hu\n",online_client_ll, des_client_info, des_client_info->ip, des_client_info->port);
	}while(des_client_info == online_client_ll);	//不能等于头节点
	printf("请输入要给这个客户端发送的信息!\n");
	bzero(msbuf, sizeof(msbuf));
	fgets(msbuf, sizeof(msbuf), stdin);
	msbuf[strcspn(msbuf, "\n")] = '\0';  // 去除末尾的换行符
	char sbuf[SYNC_MSG_SIZE] = {0};
	//c2c$chat$@本机ip@port@msg@目的ip@port
	char source_ip[INET_ADDRSTRLEN];
	sprintf(sbuf, "c2c$chat$#%s@%hu@%s@%s@%hu",
		inet_ntop(AF_INET, &client_addr.sin_addr, source_ip, sizeof(source_ip)), ntohs(client_addr.sin_port), msbuf, des_client_info->ip, des_client_info->port);	//c2c$chat$@本机ip@port@msg@目的ip@port
	send(client_sock, sbuf, sizeof(sbuf), 0);
	printf("发送成功%s\n",sbuf);
	//创建线程用于处理别的客户端发来的聊天信息

}
//打印交功能选择互列表
void print_func_list(){
	printf("****\t1 获取所有在线客户端\t****\n");
	printf("****\t2 单独聊天\t\t****\n");
	printf("****\t3 发送文件\t\t****\n");
	printf("****\t1 发送表情包\t\t****\n");
}
// 客户端交互服务函数
void client_interaction_handler(int self_sock){
	int ret;
	char select_buf[10];
	clear_socket_buffer(self_sock);	//先清空缓存
	print_func_list();	//打印菜单	1获取所有在线client，2单独和某client聊天，3发文件 ，4发表情包（图片）
	
	bzero(select_buf,sizeof(select_buf));
	fgets(select_buf, sizeof(select_buf), stdin);	//获取一行输入
	select_buf[strcspn(select_buf, "\n")] = '\0';  // 去除末尾的换行符
	if(strcmp(select_buf, "1")== 0){	//获取在线客户端
		recv_all_online_clients_handler(self_sock);
	}
	else if(strcmp(select_buf, "2")== 0){	//单独聊天
		c2c_chat_send_handler(self_sock);
	}
	else if(strcmp(select_buf, "3")== 0){	//发文件
		send_file_handler(self_sock);	//客户端给客户端发送文件
	}
	else if(strcmp(select_buf, "4")== 0){	//发表情包

	}

}
int main()
{
	int ret;
	char sbuf[100];

	//定义ipv4地址结构体变量存放客户端需要绑定的ip和端口号
	
	bzero(&client_addr,sizeof(client_addr));
	client_addr.sin_family=AF_INET;  //地址协议，IPV4
	client_addr.sin_addr.s_addr=htonl(INADDR_ANY);    //绑定客户端自己的ip地址
	client_addr.sin_port=htons(0);  //端口设为0，由系统自动分配可用端口
	
	//定义ipv4地址结构体变量存放服务器的ip和端口号
	struct sockaddr_in serveraddr;
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;  //地址协议，IPV4
	serveraddr.sin_addr.s_addr=inet_addr("192.168.11.87");    //服务器的ip地址
	serveraddr.sin_port=htons(30000);  //服务器的端口号
	
	//创建tcp套接字--》买手机
	client_sock=socket(AF_INET,SOCK_STREAM,0);
	if(client_sock==-1)
	{
		perror("买手机失败(创建tcp套接字)失败!\n");
		return -1;
	}
	
	//设置端口复用
	int on=1; //非0即可,是个开关，开启端口复用功能
	setsockopt(client_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	
	//绑定ip地址和端口号
	ret=bind(client_sock,(struct sockaddr *)&client_addr,sizeof(client_addr));
	if(ret==-1)
	{
		perror("绑定ip和端口号失败了!\n");
		return -1;
	}
	
	//连接服务器
	ret=connect(client_sock,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
	if(ret==-1)
	{
		perror("连接服务器失败了!\n");
		return -1;
	}
	//获取本机的实际ip，写入到client_addr,应该在连接服务器后
	socklen_t addr_len = sizeof(client_addr);
	getsockname(client_sock, (struct sockaddr*)&client_addr, &addr_len);
	//初始化在线成员的存储链表
	online_client_ll=client_init();
	
	//创建线程用于接收信息
	pthread_create(&recv_handler_id,NULL,recv_servermsg,&client_sock);
	while(1)	//发送信息
	{
		client_interaction_handler(client_sock);	//打开交互界面
	}
	//挂机
	close(client_sock);
	return 0;
}