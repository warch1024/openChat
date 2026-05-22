#include "user_manager.h"
#include "tools/tools.h"
#include <stdio.h>
#include <string.h>

static tools_dll_t *user_info_list = NULL;
static char user_info_file_path[256] = {0};

/**
 * @brief 初始化用户管理模块
 * @param user_info_file_path 用户信息文件路径
 * @return 成功返回0，失败返回-1
 */
int user_manager_init(const char *file_path) {
    if (!file_path) {
        return -1;
    }
    
    // 保存文件路径
    strcpy(user_info_file_path, file_path);
    
    // 初始化用户信息链表
    if (!user_info_list) {
        user_info_list = tools_init_dll_list();
        if (!user_info_list) {
            return -1;
        }
    }
    
    // 加载用户信息文件
    FILE *user_info_recorder = fopen(user_info_file_path, "r");
    if (user_info_recorder) {
        char name_pwd_buf[64] = {0};
        while (fgets(name_pwd_buf, sizeof(name_pwd_buf), user_info_recorder)) {
            char *name = strtok(name_pwd_buf, "@");
            char *psw = strtok(NULL, "\n");
            if (name && psw) {
                user_info_t *user_tmp = (user_info_t *)malloc(sizeof(user_info_t));
                if (user_tmp) {
                    strcpy(user_tmp->user_name, name);
                    strcpy(user_tmp->user_psw, psw);
                    tools_add_dll_list_node(user_info_list, user_tmp);
                }
            }
        }
        fclose(user_info_recorder);
    }
    
    return 0;
}

/**
 * @brief 清理用户管理模块资源
 */
void user_manager_cleanup(void) {
    if (user_info_list) {
        // 清理链表节点
        tools_dll_t *tmp_node = user_info_list->next;
        while (tmp_node) {
            tools_dll_t *next = tmp_node->next;
            if (tmp_node->data) {
                free(tmp_node->data);
            }
            free(tmp_node);
            tmp_node = next;
        }
        free(user_info_list);
        user_info_list = NULL;
    }
}

/**
 * @brief 验证用户信息
 * @param user_info 用户信息
 * @return 成功返回1，用户名错误返回-1，密码错误返回-2，其他错误返回0
 */
int user_manager_verify(const user_info_t *user_info) {
    if (!user_info || !user_info_list) {
        return 0;
    }
    
    for (tools_dll_t *tmp_node = user_info_list->next; tmp_node; tmp_node = tmp_node->next) {
        user_info_t *node_data = (user_info_t *)tmp_node->data;
        if (strcmp(node_data->user_name, user_info->user_name) == 0) {
            if (strcmp(node_data->user_psw, user_info->user_psw) == 0) {
                return 1;
            } else {
                return -2;
            }
        }
    }
    
    return -1;
}

/**
 * @brief 检查是否为管理员
 * @param user_info 用户信息
 * @return 是管理员返回1，否则返回0
 */
int user_manager_is_admin(const user_info_t *user_info) {
    if (!user_info) {
        return 0;
    }
    
    if (strcmp("admin", user_info->user_name) == 0 && strcmp("123456", user_info->user_psw) == 0) {
        return 1;
    }
    
    return 0;
}

/**
 * @brief 添加新用户
 * @param user_info 用户信息
 * @return 成功返回0，失败返回-1
 */
int user_manager_add_user(const user_info_t *user_info) {
    if (!user_info || !user_info_list) {
        return -1;
    }
    
    // 检查用户名是否已存在
    for (tools_dll_t *tmp_node = user_info_list->next; tmp_node; tmp_node = tmp_node->next) {
        user_info_t *node_data = (user_info_t *)tmp_node->data;
        if (strcmp(node_data->user_name, user_info->user_name) == 0) {
            return -1; // 用户名已存在
        }
    }
    
    // 创建新用户节点
    user_info_t *new_user = (user_info_t *)malloc(sizeof(user_info_t));
    if (!new_user) {
        return -1;
    }
    
    strcpy(new_user->user_name, user_info->user_name);
    strcpy(new_user->user_psw, user_info->user_psw);
    
    // 添加到链表
    tools_add_dll_list_node(user_info_list, new_user);
    
    return 0;
}

/**
 * @brief 保存用户信息到文件
 * @return 成功返回0，失败返回-1
 */
int user_manager_save(void) {
    if (!user_info_list || !user_info_file_path[0]) {
        return -1;
    }
    
    FILE *user_info_recorder = fopen(user_info_file_path, "w");
    if (!user_info_recorder) {
        return -1;
    }
    
    for (tools_dll_t *tmp_node = user_info_list->next; tmp_node; tmp_node = tmp_node->next) {
        user_info_t *node_data = (user_info_t *)tmp_node->data;
        fprintf(user_info_recorder, "%s@%s\n", node_data->user_name, node_data->user_psw);
    }
    
    fclose(user_info_recorder);
    return 0;
}
