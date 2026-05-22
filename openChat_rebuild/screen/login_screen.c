#include "login_screen.h"
#include "main_screen.h"
#include "../core/user_manager.h"
#include "../tools/tools.h"
#include <stdio.h>
#include <string.h>

// 函数声明
static void logup_btn_cb(lv_event_t *e);
static void logup_window(void);
static void sm_logup_btn_cb(lv_event_t *e);
static void bk_logup_btn_cb(lv_event_t *e);

// 登录屏幕组件
static lv_obj_t *login_screen_o = NULL;
static lv_obj_t *login_window_o = NULL;
static lv_obj_t *user_ta = NULL, *psw_ta = NULL, *cf_psw_ta = NULL;
static lv_obj_t *login_bt = NULL, *logup_bt = NULL, *sm_logup_bt = NULL, *bk_logup_bt = NULL;
static lv_obj_t *login_check_box1 = NULL, *login_check_box2 = NULL;

// 样式
static lv_style_t *login_bg_style = NULL;
static lv_style_t *login_window_opa_style = NULL;

// 键盘
static cn_kb_cp_ros login_cn_kb_pair = {.cn_kb = NULL, .cn_kb_cp = NULL};

// 默认主题路径
static const char *def_login_theme_path = "S:/IOT/projects/openChat/resources/login_bg2.jpg";

/**
 * @brief 隐藏键盘回调
 */
static void hidden_kb_cb(lv_event_t *e) {
    tools_hidden_pinyin_kb(login_cn_kb_pair.cn_kb, login_cn_kb_pair.cn_kb_cp);
}

/**
 * @brief 输入框关联键盘回调
 */
static void ta_kb_associate_cb(lv_event_t *e) {
    tools_show_pinyin_kb(login_cn_kb_pair.cn_kb, login_cn_kb_pair.cn_kb_cp);
    tools_ta_kb_associate(lv_event_get_target(e), login_cn_kb_pair.cn_kb);
}

/**
 * @brief 登录按钮回调
 */
static void login_btn_cb(lv_event_t *e) {
    char *name = lv_textarea_get_text(user_ta);
    char *passwd = lv_textarea_get_text(psw_ta);

    user_info_t user_info = {0};
    strcpy(user_info.user_name, name);
    strcpy(user_info.user_psw, passwd);

    // 检查是否为管理员
    if (user_manager_is_admin(&user_info)) {
        printf("###################管理员#################\n");
        // 显示管理员标识
        main_screen_show_admin();
    }

    // 验证用户
    if (user_manager_verify(&user_info) == 1) {
        // 登录成功，跳转到主屏幕
        main_screen_init();
    } else {
        printf("用户名或密码错误\n");
    }
}

/**
 * @brief 主题切换回调
 */
static void login_change_theme_checkbox_cb(lv_event_t *e) {
    if (login_bg_style) {
        lv_obj_remove_style(login_screen_o, login_bg_style, LV_PART_ANY | LV_STATE_ANY);

        if (lv_event_get_target(e) == login_check_box1) {
            lv_obj_clear_state(login_check_box2, LV_STATE_CHECKED);
            lv_style_set_bg_img_src(login_bg_style, "S:/IOT/projects/openChat/resources/login_bg1.jpg");
            lv_obj_set_style_text_color(login_check_box1, lv_color_hex(0xffffff), LV_PART_MAIN);
            lv_obj_set_style_text_color(login_check_box2, lv_color_hex(0xffffff), LV_PART_MAIN);
        } else if (lv_event_get_target(e) == login_check_box2) {
            lv_obj_clear_state(login_check_box1, LV_STATE_CHECKED);
            lv_style_set_bg_img_src(login_bg_style, "S:/IOT/projects/openChat/resources/login_bg2.jpg");
            lv_obj_set_style_text_color(login_check_box1, lv_color_hex(0x000000), LV_PART_MAIN);
            lv_obj_set_style_text_color(login_check_box2, lv_color_hex(0x000000), LV_PART_MAIN);
        }

        lv_style_set_bg_img_opa(login_bg_style, LV_OPA_COVER);
        lv_obj_add_style(login_screen_o, login_bg_style, 0);
    }
}

/**
 * @brief 加载默认主题
 */
static void load_def_login_theme(lv_obj_t *screen_o, const char *def_theme_path) {
    if (!login_bg_style) {
        login_bg_style = (lv_style_t *)malloc(sizeof(lv_style_t));
    }
    lv_style_init(login_bg_style);
    lv_obj_add_state(login_check_box2, LV_STATE_CHECKED);

    lv_style_set_bg_img_src(login_bg_style, def_theme_path);
    lv_style_set_bg_img_opa(login_bg_style, LV_OPA_COVER);
    lv_obj_add_style(screen_o, login_bg_style, 0);
}

/**
 * @brief 添加主题切换复选框
 */
static void add_switch_full_screen_theme_checkbox(lv_obj_t *full_screen_o) {
    login_check_box1 = tools_create_login_checkbox(full_screen_o, -1, -1, "CYBER");
    lv_obj_align_to(login_check_box1, full_screen_o, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_event_cb(login_check_box1, login_change_theme_checkbox_cb, LV_EVENT_VALUE_CHANGED, NULL);

    login_check_box2 = tools_create_login_checkbox(full_screen_o, -1, -1, "默认主题");
    lv_obj_align_to(login_check_box2, login_check_box1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);
    lv_obj_add_event_cb(login_check_box2, login_change_theme_checkbox_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

/**
 * @brief 登录窗口
 */
static void login_window(void) {
    if (!login_window_o) {
        login_window_o = lv_obj_create(login_screen_o);
    }

    lv_obj_set_size(login_window_o, 300, 200);
    lv_obj_align(login_window_o, LV_ALIGN_TOP_MID, 0, 0);

    if (!login_window_opa_style) {
        login_window_opa_style = (lv_style_t *)malloc(sizeof(lv_style_t));
    }
    tools_set_opa_style(login_window_opa_style, login_window_o, LV_OPA_50);

    // 用户名输入框
    user_ta = tools_create_login_ta(login_window_o, 200, 40, "输入用户名");
    lv_obj_align_to(user_ta, login_window_o, LV_ALIGN_TOP_MID, 0, -10);
    lv_obj_add_event_cb(user_ta, ta_kb_associate_cb, LV_EVENT_FOCUSED, NULL);

    // 密码输入框
    psw_ta = tools_create_login_ta(login_window_o, 200, 40, "输入密码");
    lv_obj_align_to(psw_ta, user_ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_textarea_set_accepted_chars(psw_ta, "0123456789");
    lv_textarea_set_password_mode(psw_ta, true);
    lv_obj_add_event_cb(psw_ta, ta_kb_associate_cb, LV_EVENT_FOCUSED, NULL);

    // 登录按钮
    login_bt = lv_btn_create(login_window_o);
    lv_obj_set_size(login_bt, 80, 40);
    lv_obj_align_to(login_bt, psw_ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_add_event_cb(login_bt, login_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *login_bt_lb = tools_create_login_bt_lb(login_bt, "登录");

    // 注册按钮
    logup_bt = lv_btn_create(login_window_o);
    lv_obj_set_size(logup_bt, 80, 40);
    lv_obj_align_to(logup_bt, psw_ta, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 4);
    lv_obj_add_event_cb(logup_bt, logup_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *logup_bt_lb = tools_create_login_bt_lb(logup_bt, "注册");
}

/**
 * @brief 注册按钮回调
 */
static void logup_btn_cb(lv_event_t *e) {
    logup_window();
}

/**
 * @brief 注册窗口
 */
static void logup_window(void) {
    // 删除登录和注册按钮
    if (login_bt && logup_bt) {
        lv_obj_del(login_bt);
        login_bt = NULL;
        lv_obj_del(logup_bt);
        logup_bt = NULL;
    }

    // 调整输入框
    lv_textarea_set_text(user_ta, "");
    lv_textarea_set_placeholder_text(user_ta, "设置用户名");

    lv_textarea_set_text(psw_ta, "");
    lv_textarea_set_placeholder_text(psw_ta, "设置密码(小于9位)");
    lv_textarea_set_password_mode(psw_ta, false);

    // 确认密码框
    cf_psw_ta = tools_create_login_ta(login_window_o, 200, 40, "重复设置密码");
    lv_obj_align_to(cf_psw_ta, psw_ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_textarea_set_accepted_chars(cf_psw_ta, "0123456789");
    lv_textarea_set_password_mode(cf_psw_ta, false);
    lv_obj_add_event_cb(cf_psw_ta, ta_kb_associate_cb, LV_EVENT_FOCUSED, NULL);

    // 提交按钮
    sm_logup_bt = lv_btn_create(login_window_o);
    lv_obj_set_size(sm_logup_bt, 80, 40);
    lv_obj_align_to(sm_logup_bt, cf_psw_ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_add_event_cb(sm_logup_bt, sm_logup_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *sm_logup_bt_lb = tools_create_login_bt_lb(sm_logup_bt, "提交");

    // 返回按钮
    bk_logup_bt = lv_btn_create(login_window_o);
    lv_obj_set_size(bk_logup_bt, 80, 40);
    lv_obj_align_to(bk_logup_bt, cf_psw_ta, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 4);
    lv_obj_add_event_cb(bk_logup_bt, bk_logup_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *bk_logup_bt_lb = tools_create_login_bt_lb(bk_logup_bt, "返回");
}

/**
 * @brief 提交注册回调
 */
static void sm_logup_btn_cb(lv_event_t *e) {
    if (user_ta && psw_ta && cf_psw_ta) {
        char *logup_name = lv_textarea_get_text(user_ta);
        char *logup_passwd = lv_textarea_get_text(psw_ta);
        char *logup_cf_passwd = lv_textarea_get_text(cf_psw_ta);

        if (strcmp(logup_passwd, logup_cf_passwd) == 0) {
            user_info_t user_info = {0};
            strcpy(user_info.user_name, logup_name);
            strcpy(user_info.user_psw, logup_passwd);

            if (user_manager_add_user(&user_info) == 0) {
                if (user_manager_save() == 0) {
                    printf("注册成功\n");
                    // 返回登录界面
                    lv_obj_del(login_window_o);
                    login_window_o = NULL;
                    login_window();
                } else {
                    printf("保存用户信息失败\n");
                }
            } else {
                printf("用户名已存在\n");
            }
        } else {
            printf("密码设置错误\n");
            lv_textarea_set_text(psw_ta, "");
            lv_textarea_set_text(cf_psw_ta, "");
        }
    }
}

/**
 * @brief 返回登录界面回调
 */
static void bk_logup_btn_cb(lv_event_t *e) {
    lv_obj_del(login_window_o);
    login_window_o = NULL;
    login_window();
}

/**
 * @brief 初始化登录屏幕
 */
void login_screen_init(void) {
    login_screen();
}

/**
 * @brief 显示登录屏幕
 */
void login_screen(void) {
    // 创建登录屏幕
    if (!login_screen_o) {
        login_screen_o = lv_obj_create(NULL);
        lv_obj_set_size(login_screen_o, 800, 480);

        // 添加键盘
        login_cn_kb_pair = tools_create_pinyin_ime(login_screen_o, 500, 200);
        lv_obj_add_event_cb(login_screen_o, hidden_kb_cb, LV_EVENT_CLICKED, NULL);

        // 添加主题切换复选框
        add_switch_full_screen_theme_checkbox(login_screen_o);

        // 加载默认主题
        load_def_login_theme(login_screen_o, def_login_theme_path);

        // 加载登录窗口
        login_window();

        // 加载屏幕
        lv_scr_load(login_screen_o);
    }
}
