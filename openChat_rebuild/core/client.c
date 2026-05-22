#include "client.h"
#include "../screen/main_screen.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SYNC_MSG_SIZE 1024

static int oc_client_sock = -1; // 客户端套接字
static pthread_t oc_recv_thread_id = 0; // 接收线程ID
static clientmsg_t *oc_online_client_ll = NULL; // 在线客户端链表头节点
static struct sockaddr_in oc_client_addr = {0}; // 客户端地址结构体

/**
 * @brief 清空套接字缓冲区
 */
static void clear_socket_buffer(int sock) {
    char buf[SYNC_MSG_SIZE];
    int ret;
    while ((ret = recv(sock, buf, sizeof(buf), MSG_DONTWAIT)) > 0);
}

/**
 * @brief 从路径中提取文件名
 */
static char *get_filename_from_path(const char *path) {
    static char filename[64];
    const char *last_slash = strrchr(path, '/');
    if (last_slash) {
        strcpy(filename, last_slash + 1);
    } else {
        strcpy(filename, path);
    }
    return filename;
}

/**
 * @brief 初始化客户端链表
 */
static clientmsg_t *client_list_init(void) {
    clientmsg_t *head = (clientmsg_t *)malloc(sizeof(clientmsg_t));
    if (head) {
        memset(head, 0, sizeof(clientmsg_t));
    }
    return head;
}

/**
 * @brief 清空客户端链表
 */
static void client_list_empty(clientmsg_t *head) {
    if (head) {
        clientmsg_t *tmp = head->next;
        while (tmp) {
            clientmsg_t *next = tmp->next;
            free(tmp);
            tmp = next;
        }
        head->next = NULL;
    }
}

/**
 * @brief 向链表尾部插入客户端信息
 */
static void client_list_insert_tail(clientmsg_t *head, clientmsg_t *client) {
    if (head && client) {
        clientmsg_t *tmp = head;
        while (tmp->next) {
            tmp = tmp->next;
        }
        tmp->next = client;
    }
}

/**
 * @brief 打印所有客户端信息
 */
static void client_list_print(clientmsg_t *head) {
    if (head) {
        clientmsg_t *tmp = head->next;
        while (tmp) {
            printf("客户端信息：%s@%d\n", tmp->ip, tmp->port);
            tmp = tmp->next;
        }
    }
}

/**
 * @brief 发送获取所有在线客户端命令
 */
static void send_get_all_online_clients_cmd(void) {
    if (oc_client_sock == -1) {
        return;
    }
    
    char sbuf[SYNC_MSG_SIZE] = {0};
    char source_ip[INET_ADDRSTRLEN];
    sprintf(sbuf, "get$all$online$clients#%s@%hu",
        inet_ntop(AF_INET, &oc_client_addr.sin_addr, source_ip, sizeof(source_ip)),
        ntohs(oc_client_addr.sin_port));
    
    send(oc_client_sock, sbuf, sizeof(sbuf), 0);
    printf("获取在线客户端发送成功%s\n", sbuf);
}

/**
 * @brief 发送聊天消息
 */
int client_send_message(const clientmsg_t *dest_client, const char *message) {
    if (oc_client_sock == -1 || !dest_client || !message) {
        return -1;
    }
    
    char sbuf[SYNC_MSG_SIZE] = {0};
    char source_ip[INET_ADDRSTRLEN];
    sprintf(sbuf, "c2c$chat#%s@%hu@%s@%s@%hu",
        inet_ntop(AF_INET, &oc_client_addr.sin_addr, source_ip, sizeof(source_ip)),
        ntohs(oc_client_addr.sin_port),
        message,
        dest_client->ip,
        dest_client->port);
    
    send(oc_client_sock, sbuf, sizeof(sbuf), 0);
    printf("单独聊天发送成功%s\n", sbuf);
    return 0;
}

/**
 * @brief 发送文件
 */
int client_send_file(const clientmsg_t *dest_client, const char *file_path) {
    if (oc_client_sock == -1 || !dest_client || !file_path) {
        return -1;
    }
    
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1) {
        perror("stat");
        return -1;
    }
    
    long file_size = file_stat.st_size;
    char cmd_sbuf[SYNC_MSG_SIZE] = {0};
    char source_ip[INET_ADDRSTRLEN];
    char *file_name = get_filename_from_path(file_path);
    
    sprintf(cmd_sbuf, "send$file#%s@%hu@%s@%ld@%s@%hu",
        inet_ntop(AF_INET, &oc_client_addr.sin_addr, source_ip, sizeof(source_ip)),
        ntohs(oc_client_addr.sin_port),
        file_name,
        file_size,
        dest_client->ip,
        dest_client->port);
    
    send(oc_client_sock, cmd_sbuf, sizeof(cmd_sbuf), 0);
    printf("发送发文件命令%s\n", cmd_sbuf);
    
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("fopen");
        return -1;
    }
    
    char file_buf[SYNC_MSG_SIZE];
    int send_k = file_size / sizeof(file_buf) + 1;
    while (send_k--) {
        memset(file_buf, 0, sizeof(file_buf));
        int ret = fread(file_buf, 1, sizeof(file_buf), file);
        if (ret == 0) {
            break;
        }
        send(oc_client_sock, file_buf, sizeof(file_buf), 0);
        usleep(5);
    }
    
    fclose(file);
    printf("发送文件数据发送成功，文件大小%ld字节\n", file_size);
    return 0;
}

/**
 * @brief 接收客户端信息
 */
static void recv_clients_info(char *msg_buf) {
    if (oc_client_sock == -1 || !oc_online_client_ll) {
        return;
    }
    
    clear_socket_buffer(oc_client_sock);
    
    char *client_num_str = strtok(msg_buf, "@");
    int client_num = atoi(client_num_str);
    printf("当前在线客户端数量：%d\n", client_num);
    
    client_list_empty(oc_online_client_ll);
    
    char rbuf[SYNC_MSG_SIZE] = {0};
    char *tmp = NULL, **pos = &tmp;
    
    for (int i = 0; i < client_num; i++) {
        memset(rbuf, 0, sizeof(rbuf));
        int ret = recv(oc_client_sock, rbuf, sizeof(rbuf), 0);
        if (ret <= 0) {
            printf("接收客户端信息失败\n");
            break;
        }
        
        strtok_r(rbuf, "#", pos);
        clientmsg_t *client_info = (clientmsg_t *)malloc(sizeof(clientmsg_t));
        if (client_info) {
            memset(client_info, 0, sizeof(clientmsg_t));
            strcpy(client_info->ip, strtok_r(NULL, "@", pos));
            client_info->port = (uint16_t)atoi(strtok_r(NULL, "@", pos));
            client_list_insert_tail(oc_online_client_ll, client_info);
            printf("客户端信息：%s@%d\n", client_info->ip, client_info->port);
        }
    }
    
    client_list_print(oc_online_client_ll);
    clear_socket_buffer(oc_client_sock);
}

/**
 * @brief 接收聊天消息
 */
static void recv_chat_message(char *msg_buf) {
    char *source_ip_s = strtok(msg_buf, "@");
    char *source_port_s = strtok(NULL, "@");
    char *msg_s = strtok(NULL, "@");
    
    if (source_ip_s && source_port_s && msg_s) {
        printf("%s@%s发来的消息：%s\n", source_ip_s, source_port_s, msg_s);
        // 显示消息到主屏幕
        extern void oc_display_recv_send_message(const char *message, lv_obj_t *main_interface_o, char *msg_type, char *img_path);
        extern lv_obj_t *open_chat_main_interface_o;
        if (open_chat_main_interface_o) {
            oc_display_recv_send_message(msg_s, open_chat_main_interface_o, "recv", NULL);
        }
    }
}

/**
 * @brief 接收文件
 */
static void recv_file(char *msg_buf) {
    if (oc_client_sock == -1) {
        return;
    }
    
    clear_socket_buffer(oc_client_sock);
    
    char *source_ip_s = strtok(msg_buf, "@");
    char *source_port_s = strtok(NULL, "@");
    char *file_name_s = strtok(NULL, "@");
    char *file_size_s = strtok(NULL, "@");
    
    if (!source_ip_s || !source_port_s || !file_name_s || !file_size_s) {
        return;
    }
    
    long file_size = strtol(file_size_s, NULL, 0);
    char file_path[200] = {0};
    char abs_path[200] = {0};
    
    getcwd(abs_path, sizeof(abs_path));
    sprintf(file_path, "%s/%s", abs_path, file_name_s);
    
    printf("客户端要接受的字节数：%ld\n", file_size);
    printf("客户端要存储的文件路径：%s\n", file_path);
    
    int fd = open(file_path, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (fd == -1) {
        perror("open");
        return;
    }
    
    char recv_buf[SYNC_MSG_SIZE];
    int write_k = file_size / sizeof(recv_buf);
    
    while (write_k--) {
        memset(recv_buf, 0, sizeof(recv_buf));
        int ret = recv(oc_client_sock, recv_buf, sizeof(recv_buf), 0);
        if (ret <= 0) {
            break;
        }
        write(fd, recv_buf, sizeof(recv_buf));
    }
    
    // 处理最后一次数据
    memset(recv_buf, 0, sizeof(recv_buf));
    int ret = recv(oc_client_sock, recv_buf, sizeof(recv_buf), 0);
    if (ret > 0) {
        write(fd, recv_buf, file_size % sizeof(recv_buf));
    }
    
    close(fd);
    usleep(1000);
    clear_socket_buffer(oc_client_sock);
    
    // 显示接收成功消息
    extern void oc_display_recv_send_message(const char *message, lv_obj_t *main_interface_o, char *msg_type, char *img_path);
    extern lv_obj_t *oc_main_interface_to_client;
    
    char send_success_msg[128] = {0};
    char *extension = strrchr(file_name_s, '.');
    
    if (extension && (strcmp(extension, ".jpg") == 0 || strcmp(extension, ".png") == 0)) {
        sprintf(send_success_msg, "接收表情包%s成功！", file_name_s);
        if (oc_main_interface_to_client) {
            oc_display_recv_send_message(send_success_msg, oc_main_interface_to_client, "recv", file_path);
        }
    } else {
        sprintf(send_success_msg, "接收文件%s成功！", file_name_s);
        if (oc_main_interface_to_client) {
            oc_display_recv_send_message(send_success_msg, oc_main_interface_to_client, "recv", NULL);
        }
    }
    
    printf("debug 发送成功的提示信息： %s\n", send_success_msg);
}

/**
 * @brief 接收服务器消息线程
 */
static void *recv_servermsg(void *arg) {
    char rbuf[SYNC_MSG_SIZE];
    int client_sock = *(int *)arg;
    
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    while (1) {
        memset(rbuf, 0, sizeof(rbuf));
        int ret = recv(client_sock, rbuf, sizeof(rbuf), 0);
        
        if (ret == 0) {
            printf("对方(服务器端)断开连接了!\n");
            break;
        } else if (ret < 0) {
            printf("接收服务器消息失败\n");
            continue;
        } else if (ret < SYNC_MSG_SIZE) {
            printf("接收服务器消息不完整\n");
            continue;
        } else if (strcmp(rbuf, "quit") == 0) {
            break;
        }
        
        char *head_cmd_buf = strtok(rbuf, "#");
        char *msg_buf_p = strtok(NULL, "#");
        
        if (!msg_buf_p) {
            printf("bug 消息格式错误%s\n", rbuf);
            continue;
        }
        
        char msg_buf[SYNC_MSG_SIZE] = {0};
        strcpy(msg_buf, msg_buf_p);
        
        if (strcmp(head_cmd_buf, "clients$info") == 0) {
            recv_clients_info(msg_buf);
        } else if (strcmp(head_cmd_buf, "c2c$chat") == 0) {
            recv_chat_message(msg_buf);
        } else if (strcmp(head_cmd_buf, "send$file") == 0) {
            recv_file(msg_buf);
        } else {
            printf("未知消息格式：%s\n", rbuf);
        }
        
        clear_socket_buffer(client_sock);
    }
    
    return NULL;
}

/**
 * @brief 初始化客户端通信模块
 */
int client_init(void) {
    // 初始化客户端地址
    memset(&oc_client_addr, 0, sizeof(oc_client_addr));
    oc_client_addr.sin_family = AF_INET;
    oc_client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    oc_client_addr.sin_port = htons(10000);
    
    // 创建套接字
    oc_client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (oc_client_sock == -1) {
        perror("创建tcp套接字失败!");
        return -1;
    }
    
    // 设置端口复用
    int on = 1;
    setsockopt(oc_client_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
    // 绑定地址
    if (bind(oc_client_sock, (struct sockaddr *)&oc_client_addr, sizeof(oc_client_addr)) == -1) {
        perror("绑定ip和端口号失败了!");
        close(oc_client_sock);
        oc_client_sock = -1;
        return -1;
    }
    
    // 连接服务器
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("192.168.11.88");
    serveraddr.sin_port = htons(30000);
    
    if (connect(oc_client_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("连接服务器失败了!");
        close(oc_client_sock);
        oc_client_sock = -1;
        return -1;
    }
    
    // 获取实际地址
    socklen_t addr_len = sizeof(oc_client_addr);
    getsockname(oc_client_sock, (struct sockaddr *)&oc_client_addr, &addr_len);
    
    // 初始化在线客户端列表
    oc_online_client_ll = client_list_init();
    if (!oc_online_client_ll) {
        close(oc_client_sock);
        oc_client_sock = -1;
        return -1;
    }
    
    // 创建接收线程
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    if (pthread_create(&oc_recv_thread_id, &attr, recv_servermsg, &oc_client_sock) != 0) {
        perror("创建接收线程失败");
        client_list_empty(oc_online_client_ll);
        free(oc_online_client_ll);
        oc_online_client_ll = NULL;
        close(oc_client_sock);
        oc_client_sock = -1;
        return -1;
    }
    
    pthread_attr_destroy(&attr);
    
    // 等待线程启动
    sleep(1);
    
    // 同步在线客户端信息
    send_get_all_online_clients_cmd();
    
    return 0;
}

/**
 * @brief 清理客户端通信模块资源
 */
void client_cleanup(void) {
    if (oc_client_sock != -1) {
        close(oc_client_sock);
        oc_client_sock = -1;
    }
    
    if (oc_online_client_ll) {
        client_list_empty(oc_online_client_ll);
        free(oc_online_client_ll);
        oc_online_client_ll = NULL;
    }
}

/**
 * @brief 获取在线客户端列表
 */
clientmsg_t *client_get_online_list(void) {
    return oc_online_client_ll;
}

/**
 * @brief 查找客户端
 */
clientmsg_t *client_find(const char *ip, uint16_t port) {
    if (!oc_online_client_ll || !ip) {
        return NULL;
    }
    
    clientmsg_t *tmp = oc_online_client_ll->next;
    while (tmp) {
        if (strcmp(tmp->ip, ip) == 0 && tmp->port == port) {
            return tmp;
        }
        tmp = tmp->next;
    }
    
    return NULL;
}
