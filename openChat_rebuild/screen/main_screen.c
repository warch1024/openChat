#include "main_screen.h"
#include "../core/client.h"
#include "../tools/tools.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

// 函数声明
static void add_open_chat_ol_client_to_list(lv_obj_t *open_chat_ol_list);
static void open_chat_ol_client_btn_cb(lv_event_t *e);
static void oc_send_msg_btn_cb(lv_event_t *e);
static void oc_send_file_btn_cb(lv_event_t *e);
static void oc_send_to_file_btn_cb(lv_event_t *e);
static void oc_send_emotion_btn_cb(lv_event_t *e);
static void add_emotion_to_preview_flex_cont(lv_obj_t *oc_flex_cont, char *emotion_dir, void *user_data);
static void oc_send_emotion_click_cb(lv_event_t *e);

// 主屏幕对象
lv_obj_t *main_screen_o = NULL;
lv_obj_t *open_chat_main_interface_o = NULL;
lv_obj_t *oc_main_interface_to_client = NULL;

// 组件对象
static lv_obj_t *main_screen_title_bg = NULL;
static lv_obj_t *main_screen_title_lb = NULL;
static lv_obj_t *open_chat_ol_list = NULL;
static lv_obj_t *open_chat_input_interface_o = NULL;
static lv_obj_t *open_chat_input_ta = NULL;
static lv_obj_t *open_chat_send_msg_btn = NULL;
static lv_obj_t *open_chat_send_file_btn = NULL;
static lv_obj_t *open_chat_send_emoti_btn = NULL;

// 键盘
static cn_kb_cp_ros open_chat_screen_cn_kb_pair = {.cn_kb = NULL, .cn_kb_cp = NULL};

// 当前聊天客户端
static clientmsg_t *oc_current_chat_client = NULL;

/**
 * @brief 隐藏键盘回调
 */
static void oc_hidden_kb_cb(lv_event_t *e) {
    tools_hidden_pinyin_kb(open_chat_screen_cn_kb_pair.cn_kb, open_chat_screen_cn_kb_pair.cn_kb_cp);
    move_input_interface_default(open_chat_input_interface_o, open_chat_main_interface_o);
}

/**
 * @brief 输入框关联键盘回调
 */
static void oc_ta_kb_associate_cb(lv_event_t *e) {
    tools_show_pinyin_kb(open_chat_screen_cn_kb_pair.cn_kb, open_chat_screen_cn_kb_pair.cn_kb_cp);
    tools_ta_kb_associate(lv_event_get_target(e), open_chat_screen_cn_kb_pair.cn_kb);
    move_input_interface_above_kb(open_chat_input_interface_o, open_chat_screen_cn_kb_pair.cn_kb);
}

/**
 * @brief 移动输入框到键盘上方
 */
void move_input_interface_above_kb(lv_obj_t *input_interface_o, lv_obj_t *kb_o) {
    lv_obj_align_to(input_interface_o, kb_o, LV_ALIGN_OUT_TOP_MID, 0, 0);
}

/**
 * @brief 移动输入框到默认位置
 */
void move_input_interface_default(lv_obj_t *input_interface_o, lv_obj_t *main_interface_o) {
    lv_obj_align_to(input_interface_o, main_interface_o, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
}

/**
 * @brief 显示消息
 */
void oc_display_recv_send_message(const char *message, lv_obj_t *main_interface_o, char *msg_type, char *img_path) {
    if (!message || !main_interface_o) {
        printf("显示消息失败，消息或主界面对象为空\n");
        return;
    }

    // 创建消息气泡容器
    lv_obj_t *bubble_bar_o = lv_obj_create(main_interface_o);
    lv_obj_set_size(bubble_bar_o, lv_obj_get_width(main_interface_o), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(bubble_bar_o, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(bubble_bar_o, 0, LV_PART_MAIN);
    lv_obj_align_to(bubble_bar_o, main_interface_o, LV_ALIGN_BOTTOM_MID, 0, 0);

    // 创建消息气泡
    lv_obj_t *bubble = lv_obj_create(bubble_bar_o);
    lv_obj_set_size(bubble, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(bubble, 25, LV_PART_MAIN);
    lv_obj_set_style_pad_all(bubble, 12, LV_PART_MAIN);
    lv_obj_set_style_border_width(bubble, 0, LV_PART_MAIN);

    if (strcmp(msg_type, "recv") == 0) {
        lv_obj_set_style_bg_color(bubble, lv_color_hex(0xFFFACD), LV_PART_MAIN);
        lv_obj_align(bubble, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    } else if (strcmp(msg_type, "send") == 0) {
        lv_obj_set_style_bg_color(bubble, lv_color_hex(0x90EE90), LV_PART_MAIN);
        lv_obj_align(bubble, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    }

    // 创建图片显示（如果有）
    lv_obj_t *img_cont = NULL;
    if (img_path) {
        img_cont = lv_obj_create(bubble);
        lv_obj_set_size(img_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(img_cont, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(img_cont, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(img_cont, 0, LV_PART_MAIN);
        lv_obj_align(img_cont, LV_ALIGN_TOP_LEFT, 0, 0);

        lv_obj_t *img = lv_img_create(img_cont);
        char img_path_format[256] = {0};
        sprintf(img_path_format, "S:%s", img_path);
        lv_img_set_src(img, img_path_format);
        lv_obj_set_size(img, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_border_width(img, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(img, 0, LV_PART_MAIN);
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    }

    // 创建消息文本
    lv_obj_t *message_lb = lv_label_create(bubble);
    lv_obj_add_style(message_lb, def_text_style, 0);

    if (strlen(message) > 20) {
        lv_obj_set_width(message_lb, lv_obj_get_width(main_interface_o) / 2);
    } else {
        lv_obj_set_width(message_lb, LV_SIZE_CONTENT);
    }

    lv_label_set_long_mode(message_lb, LV_LABEL_LONG_WRAP);
    lv_label_set_text(message_lb, message);
    lv_obj_align(message_lb, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_align(message_lb, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

    // 如果有图片，将文本放在图片下方
    if (img_cont) {
        lv_obj_align_to(message_lb, img_cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    }

    // 创建气泡间隔
    lv_obj_t *bubble_gap = lv_obj_create(main_interface_o);
    lv_obj_set_size(bubble_gap, lv_obj_get_width(bubble_bar_o), 5);
    lv_obj_set_style_bg_opa(bubble_gap, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(bubble_gap, 0, LV_PART_MAIN);
    lv_obj_align_to(bubble_gap, bubble_bar_o, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    // 滚动到新消息
    lv_obj_scroll_to_y(main_interface_o, LV_COORD_MAX, LV_ANIM_ON);
}

/**
 * @brief 加载主聊天界面
 */
static void load_open_chat_main_interface(lv_obj_t *screen_o) {
    lv_coord_t main_screen_h = lv_obj_get_height(screen_o);
    lv_coord_t main_screen_w = lv_obj_get_width(screen_o);

    // 创建在线客户端列表容器
    lv_obj_t *open_chat_ol_list_o = lv_obj_create(screen_o);
    lv_obj_set_size(open_chat_ol_list_o, 255, main_screen_h - 2);
    lv_obj_align_to(open_chat_ol_list_o, screen_o, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(open_chat_ol_list_o, LV_OBJ_FLAG_SCROLLABLE);

    lv_coord_t ol_list_h = lv_obj_get_height(open_chat_ol_list_o);
    lv_coord_t ol_list_w = lv_obj_get_width(open_chat_ol_list_o);

    // 创建在线客户端列表
    open_chat_ol_list = lv_list_create(open_chat_ol_list_o);
    lv_obj_set_size(open_chat_ol_list, ol_list_w, ol_list_h);
    lv_obj_align_to(open_chat_ol_list, open_chat_ol_list_o, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(open_chat_ol_list, oc_hidden_kb_cb, LV_EVENT_CLICKED, NULL);

    // 创建主聊天界面容器
    open_chat_main_interface_o = lv_obj_create(screen_o);
    oc_main_interface_to_client = open_chat_main_interface_o;
    lv_obj_set_size(open_chat_main_interface_o, main_screen_w - ol_list_w, main_screen_h - 120);
    lv_obj_align_to(open_chat_main_interface_o, screen_o, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_scroll_dir(open_chat_main_interface_o, LV_DIR_VER);
    lv_obj_add_event_cb(open_chat_main_interface_o, oc_hidden_kb_cb, LV_EVENT_CLICKED, NULL);

    // 创建输入框容器
    open_chat_input_interface_o = lv_obj_create(screen_o);
    lv_obj_set_size(open_chat_input_interface_o, main_screen_w - ol_list_w, 120);
    lv_obj_align_to(open_chat_input_interface_o, open_chat_main_interface_o, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_style_pad_all(open_chat_input_interface_o, 0, LV_PART_MAIN);
    lv_obj_clear_flag(open_chat_input_interface_o, LV_OBJ_FLAG_SCROLLABLE);

    // 创建输入框
    open_chat_input_ta = tools_create_common_ta(open_chat_input_interface_o, 
        lv_obj_get_width(open_chat_input_interface_o), 
        lv_obj_get_height(open_chat_input_interface_o), 
        "请输入消息");
    lv_obj_align_to(open_chat_input_ta, open_chat_input_interface_o, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_event_cb(open_chat_input_ta, oc_ta_kb_associate_cb, LV_EVENT_FOCUSED, NULL);

    // 创建按钮
    lv_coord_t btn_w = 110, btn_h = 43;

    // 发送表情按钮
    open_chat_send_emoti_btn = tools_create_lbed_bt(open_chat_input_interface_o, 
        "(●'◡'●)", btn_w, btn_h, lv_color_hex(0x4CAF50));
    lv_obj_add_state(open_chat_send_emoti_btn, LV_STATE_DISABLED);
    lv_obj_align_to(open_chat_send_emoti_btn, open_chat_input_interface_o, LV_ALIGN_BOTTOM_LEFT, 5, 1);
    lv_obj_add_event_cb(open_chat_send_emoti_btn, oc_send_emotion_btn_cb, LV_EVENT_CLICKED, NULL);

    // 发送文件按钮
    open_chat_send_file_btn = tools_create_lbed_bt(open_chat_input_interface_o, 
        "发送文件", btn_w, btn_h, lv_color_hex(0xFF8C00));
    lv_obj_add_state(open_chat_send_file_btn, LV_STATE_DISABLED);
    lv_obj_align_to(open_chat_send_file_btn, open_chat_send_emoti_btn, LV_ALIGN_OUT_RIGHT_MID, 5, 1);
    lv_obj_add_event_cb(open_chat_send_file_btn, oc_send_file_btn_cb, LV_EVENT_CLICKED, NULL);

    // 发送消息按钮
    open_chat_send_msg_btn = tools_create_lbed_bt(open_chat_input_interface_o, 
        "发送消息", btn_w, btn_h, lv_color_hex(0x2196F3));
    lv_obj_add_state(open_chat_send_msg_btn, LV_STATE_DISABLED);
    lv_obj_align_to(open_chat_send_msg_btn, open_chat_input_interface_o, LV_ALIGN_BOTTOM_RIGHT, -5, 1);
    lv_obj_add_event_cb(open_chat_send_msg_btn, oc_send_msg_btn_cb, LV_EVENT_CLICKED, NULL);

    // 加载在线用户列表
    add_open_chat_ol_client_to_list(open_chat_ol_list);
}

/**
 * @brief 添加在线客户端到列表
 */
static void add_open_chat_ol_client_to_list(lv_obj_t *ol_list) {
    clientmsg_t *online_clients = client_get_online_list();
    if (!online_clients) {
        return;
    }

    lv_obj_clean(ol_list);

    clientmsg_t *current = online_clients->next;
    while (current) {
        char ol_client_info[30] = {0};
        sprintf(ol_client_info, "%s@%hu", current->ip, current->port);

        lv_obj_t *ol_client_btn = lv_list_add_btn(ol_list, NULL, ol_client_info);
        lv_obj_set_user_data(ol_client_btn, current);
        lv_obj_add_style(ol_client_btn, def_text_style, 0);
        lv_obj_add_event_cb(ol_client_btn, open_chat_ol_client_btn_cb, LV_EVENT_CLICKED, current);

        current = current->next;
    }
}

/**
 * @brief 在线客户端列表按钮回调
 */
static void open_chat_ol_client_btn_cb(lv_event_t *e) {
    clientmsg_t *client = (clientmsg_t *)lv_event_get_user_data(e);
    if (!client) {
        return;
    }

    printf("debug: 点击了在线用户 %s@%hu\n", client->ip, client->port);

    // 更新标题
    char ol_client_info[30] = {0};
    sprintf(ol_client_info, "%s@%hu", client->ip, client->port);
    lv_obj_set_size(main_screen_title_bg, 250, 28);
    lv_label_set_text(main_screen_title_lb, ol_client_info);

    // 保存当前聊天客户端
    oc_current_chat_client = client;

    // 启用按钮
    lv_obj_clear_state(open_chat_send_file_btn, LV_STATE_DISABLED);
    lv_obj_clear_state(open_chat_send_emoti_btn, LV_STATE_DISABLED);
    lv_obj_clear_state(open_chat_send_msg_btn, LV_STATE_DISABLED);
}

/**
 * @brief 发送消息按钮回调
 */
static void oc_send_msg_btn_cb(lv_event_t *e) {
    if (!oc_current_chat_client) {
        return;
    }

    char msg[1024] = {0};
    strcpy(msg, lv_textarea_get_text(open_chat_input_ta));

    if (client_send_message(oc_current_chat_client, msg) == 0) {
        oc_display_recv_send_message(msg, open_chat_main_interface_o, "send", NULL);
        lv_textarea_set_text(open_chat_input_ta, "");
    }
}

/**
 * @brief 发送文件按钮回调
 */
static void oc_send_file_btn_cb(lv_event_t *e) {
    if (!oc_current_chat_client) {
        return;
    }

    lv_obj_t *oc_input_ta = lv_obj_get_child(open_chat_input_interface_o, 0);
    lv_obj_t *oc_send_msg_btn = lv_obj_get_child(open_chat_input_interface_o, 3);

    if (!oc_input_ta || !oc_send_msg_btn) {
        return;
    }

    // 禁用并隐藏发送消息按钮
    lv_obj_add_state(oc_send_msg_btn, LV_STATE_DISABLED);
    lv_obj_add_flag(oc_send_msg_btn, LV_OBJ_FLAG_HIDDEN);

    // 创建确认发送按钮
    lv_obj_t *oc_send_to_file_btn = tools_create_lbed_bt(open_chat_input_interface_o, 
        "确认发送", 110, 43, lv_color_hex(0xFF8C00));
    lv_obj_align_to(oc_send_to_file_btn, oc_send_msg_btn, LV_ALIGN_CENTER, 0, 0);

    // 设置输入框提示
    lv_textarea_set_placeholder_text(oc_input_ta, "请输入文件名");

    // 传递参数
    static struct {
        lv_obj_t *main_interface;
        lv_obj_t *input_interface;
        clientmsg_t *current_client;
    } file_send_info = {0};

    file_send_info.main_interface = open_chat_main_interface_o;
    file_send_info.input_interface = open_chat_input_interface_o;
    file_send_info.current_client = oc_current_chat_client;

    lv_obj_add_event_cb(oc_send_to_file_btn, oc_send_to_file_btn_cb, LV_EVENT_CLICKED, &file_send_info);
}

/**
 * @brief 确认发送文件回调
 */
static void oc_send_to_file_btn_cb(lv_event_t *e) {
    struct {
        lv_obj_t *main_interface;
        lv_obj_t *input_interface;
        clientmsg_t *current_client;
    } *file_send_info = (void *)lv_event_get_user_data(e);

    if (!file_send_info || !file_send_info->current_client) {
        return;
    }

    lv_obj_t *oc_input_ta = lv_obj_get_child(file_send_info->input_interface, 0);
    lv_obj_t *oc_send_msg_btn = lv_obj_get_child(file_send_info->input_interface, 3);
    lv_obj_t *oc_send_to_file_btn = lv_obj_get_child(file_send_info->input_interface, 4);

    if (!oc_input_ta || !oc_send_msg_btn || !oc_send_to_file_btn) {
        return;
    }

    char file_name[64] = {0};
    strcpy(file_name, lv_textarea_get_text(oc_input_ta));

    if (strlen(file_name) <= 0) {
        return;
    }

    // 检查文件名
    if (strpbrk(file_name, "/\\?%*:|\"<>") != NULL) {
        printf("发送文件失败，文件名包含非法字符\n");
        return;
    }

    // 构建文件路径
    char abs_file_path[256] = {0};
    char current_dir[256] = {0};
    getcwd(current_dir, sizeof(current_dir));
    sprintf(abs_file_path, "%s/%s", current_dir, file_name);

    // 发送文件
    if (client_send_file(file_send_info->current_client, abs_file_path) == 0) {
        // 显示发送成功消息
        char send_success_msg[128] = {0};
        sprintf(send_success_msg, "发送文件%s成功！", file_name);
        oc_display_recv_send_message(send_success_msg, file_send_info->main_interface, "send", NULL);
    }

    // 恢复界面
    lv_obj_clear_state(oc_send_msg_btn, LV_STATE_DISABLED);
    lv_obj_clear_flag(oc_send_msg_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_del(oc_send_to_file_btn);
}

/**
 * @brief 发送表情按钮回调
 */
static void oc_send_emotion_btn_cb(lv_event_t *e) {
    if (!oc_current_chat_client) {
        return;
    }

    lv_obj_t *oc_input_ta = lv_obj_get_child(open_chat_input_interface_o, 0);
    lv_obj_t *oc_send_file_btn = lv_obj_get_child(open_chat_input_interface_o, 2);
    lv_obj_t *oc_send_msg_btn = lv_obj_get_child(open_chat_input_interface_o, 3);

    if (!oc_input_ta || !oc_send_file_btn || !oc_send_msg_btn) {
        return;
    }

    // 禁用相关组件
    lv_obj_add_state(oc_input_ta, LV_STATE_DISABLED);
    lv_obj_add_state(oc_send_file_btn, LV_STATE_DISABLED);
    lv_obj_add_state(oc_send_msg_btn, LV_STATE_DISABLED);

    // 构建表情路径
    char emotion_path[128] = {0};
    char current_path[64] = {0};
    getcwd(current_path, sizeof(current_path));
    sprintf(emotion_path, "%s/emotion", current_path);

    // 创建表情预览容器
    static lv_obj_t *oc_flex_cont = NULL;
    oc_flex_cont = lv_obj_create(main_screen_o);
    lv_obj_set_size(oc_flex_cont, 400, 250);
    lv_obj_align_to(oc_flex_cont, open_chat_input_interface_o, LV_ALIGN_OUT_TOP_MID, 0, 0);

    // 传递参数
    static struct {
        lv_obj_t *main_interface;
        lv_obj_t *input_interface;
        clientmsg_t *current_client;
        lv_obj_t **flex_cont;
    } emotion_send_info = {0};

    emotion_send_info.main_interface = open_chat_main_interface_o;
    emotion_send_info.input_interface = open_chat_input_interface_o;
    emotion_send_info.current_client = oc_current_chat_client;
    emotion_send_info.flex_cont = &oc_flex_cont;

    // 添加表情到容器
    add_emotion_to_preview_flex_cont(oc_flex_cont, emotion_path, &emotion_send_info);
}

/**
 * @brief 添加表情到预览容器
 */
static void add_emotion_to_preview_flex_cont(lv_obj_t *oc_flex_cont, char *emotion_dir, void *user_data) {
    struct {
        lv_obj_t *main_interface;
        lv_obj_t *input_interface;
        clientmsg_t *current_client;
        lv_obj_t **flex_cont;
    } *emotion_send_info = (void *)user_data;

    DIR *dir = opendir(emotion_dir);
    if (!dir) {
        printf("打开表情包文件夹失败\n");
        return;
    }

    // 设置Flex布局
    lv_obj_set_layout(oc_flex_cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(oc_flex_cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_all(oc_flex_cont, 0, LV_PART_MAIN);

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        char emotion_path[128] = {0};
        sprintf(emotion_path, "%s/%s", emotion_dir, ent->d_name);

        // 创建表情预览
        lv_obj_t *img_preview_o = lv_obj_create(oc_flex_cont);
        lv_obj_set_size(img_preview_o, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        // 保存图片路径
        lv_obj_t *img_preview_label = lv_label_create(img_preview_o);
        lv_obj_set_size(img_preview_label, 1, 1);
        lv_label_set_text(img_preview_label, emotion_path);
        lv_obj_add_flag(img_preview_label, LV_OBJ_FLAG_HIDDEN);

        // 创建图片
        lv_obj_t *img_preview = lv_img_create(img_preview_o);
        char img_path[128] = {0};
        sprintf(img_path, "S:%s", emotion_path);
        lv_img_set_src(img_preview, img_path);
        lv_obj_set_size(img_preview, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(img_preview, 20, LV_PART_MAIN);
        lv_obj_set_style_clip_corner(img_preview, true, LV_PART_MAIN);
        lv_obj_set_style_border_width(img_preview, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(img_preview, 0, LV_PART_MAIN);

        // 添加点击事件
        lv_obj_add_event_cb(img_preview_o, oc_send_emotion_click_cb, LV_EVENT_CLICKED, emotion_send_info);
    }

    closedir(dir);
}

/**
 * @brief 点击表情回调
 */
static void oc_send_emotion_click_cb(lv_event_t *e) {
    struct {
        lv_obj_t *main_interface;
        lv_obj_t *input_interface;
        clientmsg_t *current_client;
        lv_obj_t **flex_cont;
    } *emotion_send_info = (void *)lv_event_get_user_data(e);

    if (!emotion_send_info || !emotion_send_info->current_client) {
        return;
    }

    lv_obj_t *img_preview_o = lv_event_get_target(e);
    lv_obj_t *img_preview_label = lv_obj_get_child(img_preview_o, 0);

    if (!img_preview_label) {
        return;
    }

    char *emotion_img_path = (char *)lv_label_get_text(img_preview_label);
    if (!emotion_img_path) {
        return;
    }

    // 发送表情
    if (client_send_file(emotion_send_info->current_client, emotion_img_path) == 0) {
        // 显示发送成功消息
        char send_success_msg[128] = {0};
        sprintf(send_success_msg, "发送表情成功！");
        oc_display_recv_send_message(send_success_msg, emotion_send_info->main_interface, "send", emotion_img_path);
    }

    // 恢复界面
    lv_obj_t *oc_input_ta = lv_obj_get_child(emotion_send_info->input_interface, 0);
    lv_obj_t *oc_send_file_btn = lv_obj_get_child(emotion_send_info->input_interface, 2);
    lv_obj_t *oc_send_msg_btn = lv_obj_get_child(emotion_send_info->input_interface, 3);

    if (oc_input_ta) lv_obj_clear_state(oc_input_ta, LV_STATE_DISABLED);
    if (oc_send_file_btn) lv_obj_clear_state(oc_send_file_btn, LV_STATE_DISABLED);
    if (oc_send_msg_btn) lv_obj_clear_state(oc_send_msg_btn, LV_STATE_DISABLED);

    // 删除表情容器
    if (*emotion_send_info->flex_cont) {
        lv_obj_del(*emotion_send_info->flex_cont);
        *emotion_send_info->flex_cont = NULL;
    }
}

/**
 * @brief 显示管理员标识
 */
void main_screen_show_admin(void) {
    if (main_screen_o) {
        lv_obj_t *main_screen_admin_lb = lv_label_create(main_screen_o);
        lv_obj_set_align(main_screen_admin_lb, LV_ALIGN_TOP_RIGHT);
        lv_label_set_text(main_screen_admin_lb, "管理员");
        lv_obj_add_style(main_screen_admin_lb, def_text_style, 1);
    }
}

/**
 * @brief 初始化主屏幕
 */
void main_screen_init(void) {
    main_screen();
}

/**
 * @brief 显示主屏幕
 */
void main_screen(void) {
    // 创建主屏幕
    main_screen_o = lv_obj_create(NULL);
    lv_obj_set_size(main_screen_o, 800, 480);
    lv_obj_set_style_bg_color(main_screen_o, lv_color_hex(0xd4e9f7), LV_PART_MAIN);

    // 添加主聊天界面
    load_open_chat_main_interface(main_screen_o);

    // 添加键盘
    open_chat_screen_cn_kb_pair = tools_create_pinyin_ime(main_screen_o, 500, 200);

    // 添加标题
    main_screen_title_bg = lv_obj_create(main_screen_o);
    lv_obj_set_size(main_screen_title_bg, 150, 25);
    lv_obj_set_align(main_screen_title_bg, LV_ALIGN_TOP_MID);
    lv_obj_set_style_border_width(main_screen_title_bg, 0, LV_PART_MAIN);

    main_screen_title_lb = lv_label_create(main_screen_o);
    lv_obj_set_align(main_screen_title_lb, LV_ALIGN_TOP_MID);
    lv_label_set_text(main_screen_title_lb, "openChat");
    lv_obj_add_style(main_screen_title_lb, def_text_style, 0);

    // 加载屏幕
    lv_scr_load_anim(main_screen_o, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, false);
}
