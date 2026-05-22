#include "openchat.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_t client_thread_id;  // 客户端线程ID

/**
 * @brief 初始化基础组件，如字体、样式等
 */
static void init_base_components(void) {
    // 初始化中文字体支持
    if (!def_text_style) {
        def_text_style = (lv_style_t *)malloc(sizeof(lv_style_t));
        if (def_text_style) {
            tools_create_font_style(def_text_style, "/fonts/MSYH.TTC", 18);
        } else {
            perror("内存申请失败");
        }
    }
}

/**
 * @brief 初始化并启动OpenChat应用
 * @return 成功返回0，失败返回-1
 */
int openchat_init(void) {
    // 初始化用户管理
    if (user_manager_init("/IOT/projects/openChat/user_info_recorder.txt") != 0) {
        printf("用户管理初始化失败\n");
        return -1;
    }
    
    // 初始化客户端通信
    if (client_init() != 0) {
        printf("客户端初始化失败\n");
        user_manager_cleanup();
        return -1;
    }
    
    // 初始化基础组件
    init_base_components();
    
    // 启动登录屏幕
    login_screen_init();
    
    return 0;
}

/**
 * @brief 清理OpenChat应用资源
 */
void openchat_cleanup(void) {
    // 清理客户端资源
    client_cleanup();
    
    // 清理用户管理资源
    user_manager_cleanup();
    
    // 清理字体资源
    if (def_text_style) {
        free(def_text_style);
        def_text_style = NULL;
    }
}
