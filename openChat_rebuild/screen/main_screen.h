#ifndef _MAIN_SCREEN_H
#define _MAIN_SCREEN_H

#include "lvgl/lvgl.h"

/**
 * @brief 初始化主屏幕
 */
void main_screen_init(void);

/**
 * @brief 显示主屏幕
 */
void main_screen(void);

/**
 * @brief 显示管理员标识
 */
void main_screen_show_admin(void);

/**
 * @brief 显示消息
 * @param message 消息内容
 * @param main_interface_o 主界面对象
 * @param msg_type 消息类型（"recv" 或 "send"）
 * @param img_path 图片路径（可选）
 */
void oc_display_recv_send_message(const char *message, lv_obj_t *main_interface_o, char *msg_type, char *img_path);

/**
 * @brief 移动输入框到键盘上方
 * @param input_interface_o 输入界面对象
 * @param kb_o 键盘对象
 */
void move_input_interface_above_kb(lv_obj_t *input_interface_o, lv_obj_t *kb_o);

/**
 * @brief 移动输入框到默认位置
 * @param input_interface_o 输入界面对象
 * @param main_interface_o 主界面对象
 */
void move_input_interface_default(lv_obj_t *input_interface_o, lv_obj_t *main_interface_o);

// 全局对象声明
extern lv_obj_t *open_chat_main_interface_o;
extern lv_obj_t *oc_main_interface_to_client;

#endif // _MAIN_SCREEN_H
