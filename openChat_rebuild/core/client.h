#ifndef _CLIENT_H
#define _CLIENT_H

#include <stdint.h>
#include <netinet/in.h>

/**
 * @brief 客户端信息结构体
 */
typedef struct clientmsg {
    char ip[16];
    uint16_t port;
    struct clientmsg *next;
} clientmsg_t;

/**
 * @brief 初始化客户端通信模块
 * @return 成功返回0，失败返回-1
 */
int client_init(void);

/**
 * @brief 清理客户端通信模块资源
 */
void client_cleanup(void);

/**
 * @brief 获取在线客户端列表
 * @return 在线客户端链表，失败返回NULL
 */
clientmsg_t *client_get_online_list(void);

/**
 * @brief 发送聊天消息
 * @param dest_client 目标客户端信息
 * @param message 消息内容
 * @return 成功返回0，失败返回-1
 */
int client_send_message(const clientmsg_t *dest_client, const char *message);

/**
 * @brief 发送文件
 * @param dest_client 目标客户端信息
 * @param file_path 文件路径
 * @return 成功返回0，失败返回-1
 */
int client_send_file(const clientmsg_t *dest_client, const char *file_path);

/**
 * @brief 查找客户端
 * @param ip IP地址
 * @param port 端口号
 * @return 找到返回客户端信息，否则返回NULL
 */
clientmsg_t *client_find(const char *ip, uint16_t port);

#endif // _CLIENT_H
