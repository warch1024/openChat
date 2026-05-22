#ifndef _USER_MANAGER_H
#define _USER_MANAGER_H

#include <stdint.h>

/**
 * @brief 用户信息结构体
 */
typedef struct {
    char user_name[32];
    char user_psw[32];
} user_info_t;

/**
 * @brief 初始化用户管理模块
 * @param user_info_file_path 用户信息文件路径
 * @return 成功返回0，失败返回-1
 */
int user_manager_init(const char *user_info_file_path);

/**
 * @brief 清理用户管理模块资源
 */
void user_manager_cleanup(void);

/**
 * @brief 验证用户信息
 * @param user_info 用户信息
 * @return 成功返回1，用户名错误返回-1，密码错误返回-2，其他错误返回0
 */
int user_manager_verify(const user_info_t *user_info);

/**
 * @brief 检查是否为管理员
 * @param user_info 用户信息
 * @return 是管理员返回1，否则返回0
 */
int user_manager_is_admin(const user_info_t *user_info);

/**
 * @brief 添加新用户
 * @param user_info 用户信息
 * @return 成功返回0，失败返回-1
 */
int user_manager_add_user(const user_info_t *user_info);

/**
 * @brief 保存用户信息到文件
 * @return 成功返回0，失败返回-1
 */
int user_manager_save(void);

#endif // _USER_MANAGER_H
