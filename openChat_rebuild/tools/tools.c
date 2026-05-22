#include "tools.h"
#include <stdio.h>
#include <string.h>

// 全局字体样式
lv_style_t *def_text_style = NULL;

/**
 * @brief 创建字体样式
 */
void tools_create_font_style(lv_style_t *style, char *font_path, int font_size) {
    static lv_ft_info_t info;
    memset(&info, 0, sizeof(info));
    
    info.name = font_path;
    info.weight = font_size;
    info.style = FT_FONT_STYLE_NORMAL;
    info.mem = NULL;
    
    if (!lv_ft_font_init(&info)) {
        LV_LOG_ERROR("create failed.");
        return;
    }
    
    lv_style_init(style);
    lv_style_set_text_font(style, info.font);
    lv_style_set_text_align(style, LV_TEXT_ALIGN_CENTER);
}

/**
 * @brief 给键盘添加拼音插件
 */
static lv_obj_t *add_pinyin_plugin(lv_obj_t *kb, lv_obj_t *parent_obj, int kb_width) {
    if (!parent_obj || !kb) {
        printf("debug: 父对象为空，或键盘为空，无法创建拼音输入法插件\n");
        return NULL;
    }
    
    lv_obj_t *pinyin_ime = lv_ime_pinyin_create(parent_obj);
    if (!pinyin_ime) {
        printf("debug: 创建拼音输入法插件失败\n");
        return NULL;
    }
    
    lv_obj_set_size(pinyin_ime, 1, 1);
    lv_obj_set_pos(pinyin_ime, 0, 0);
    lv_obj_add_style(pinyin_ime, def_text_style, 0);
    lv_ime_pinyin_set_mode(pinyin_ime, LV_IME_PINYIN_MODE_K26);
    
    lv_obj_t *cand_pannel = lv_ime_pinyin_get_cand_panel(pinyin_ime);
    if (!cand_pannel) {
        printf("debug: 获取拼音候选栏对象失败\n");
        return NULL;
    }
    
    lv_obj_set_width(cand_pannel, kb_width);
    lv_ime_pinyin_set_keyboard(pinyin_ime, kb);
    
    lv_obj_move_foreground(pinyin_ime);
    lv_obj_move_foreground(cand_pannel);
    
    return cand_pannel;
}

/**
 * @brief 创建拼音输入法
 */
cn_kb_cp_ros tools_create_pinyin_ime(lv_obj_t *parent_obj, int weight, int height) {
    if (!parent_obj) {
        printf("debug: 父对象为空，无法创建拼音输入法\n");
        return (cn_kb_cp_ros){NULL, NULL};
    }
    
    lv_obj_t *cn_kb = lv_keyboard_create(parent_obj);
    lv_obj_set_size(cn_kb, weight, height);
    lv_obj_set_pos(cn_kb, 0, 0);
    
    lv_obj_t *cn_kb_cp = add_pinyin_plugin(cn_kb, parent_obj, weight);
    
    if (cn_kb && cn_kb_cp) {
        tools_hidden_pinyin_kb(cn_kb, cn_kb_cp);
        return (cn_kb_cp_ros){cn_kb, cn_kb_cp};
    } else {
        printf("debug: 创建拼音输入法失败，键盘%p, 候选栏%p\n", cn_kb, cn_kb_cp);
        return (cn_kb_cp_ros){NULL, NULL};
    }
}

/**
 * @brief 隐藏拼音键盘
 */
void tools_hidden_pinyin_kb(lv_obj_t *kb, lv_obj_t *cp) {
    if (kb && cp) {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cp, LV_OBJ_FLAG_HIDDEN);
        printf("debug: 隐藏拼音键盘键盘%p, 候选栏%p\n", kb, cp);
    }
}

/**
 * @brief 显示拼音键盘
 */
void tools_show_pinyin_kb(lv_obj_t *kb, lv_obj_t *cp) {
    if (kb && cp) {
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(cp, LV_OBJ_FLAG_HIDDEN);
        printf("debug: 显示拼音键盘键盘%p, 候选栏%p\n", kb, cp);
        
        lv_obj_move_foreground(kb);
        lv_obj_move_foreground(cp);
    }
}

/**
 * @brief 创建登录窗口输入框
 */
lv_obj_t *tools_create_login_ta(lv_obj_t *parent_o, int width, int height, char *pht) {
    lv_obj_t *ta_obj = lv_textarea_create(parent_o);
    lv_obj_set_size(ta_obj, width, height);
    lv_textarea_set_placeholder_text(ta_obj, pht);
    lv_obj_add_style(ta_obj, def_text_style, 0);
    lv_textarea_set_one_line(ta_obj, true);
    lv_textarea_set_max_length(ta_obj, 8);
    return ta_obj;
}

/**
 * @brief 创建通用输入框
 */
lv_obj_t *tools_create_common_ta(lv_obj_t *parent_o, int width, int height, char *pht) {
    lv_obj_t *ta_obj = lv_textarea_create(parent_o);
    lv_obj_set_size(ta_obj, width, height);
    lv_textarea_set_placeholder_text(ta_obj, pht);
    lv_textarea_set_align(ta_obj, LV_TEXT_ALIGN_LEFT);
    lv_obj_add_style(ta_obj, def_text_style, 0);
    return ta_obj;
}

/**
 * @brief 创建登录按钮标签
 */
lv_obj_t *tools_create_login_bt_lb(lv_obj_t *parent_o, char *text) {
    lv_obj_t *bt_lb_obj = lv_label_create(parent_o);
    lv_obj_align_to(bt_lb_obj, parent_o, LV_ALIGN_CENTER, -5, -2);
    lv_obj_add_style(bt_lb_obj, def_text_style, 0);
    lv_label_set_text(bt_lb_obj, text);
    return bt_lb_obj;
}

/**
 * @brief 创建按钮标签
 */
lv_obj_t *tools_add_bt_lb(lv_obj_t *parent_o, char *text) {
    lv_obj_t *bt_lb_obj = lv_label_create(parent_o);
    lv_obj_align_to(bt_lb_obj, parent_o, LV_ALIGN_CENTER, 0, 0);
    lv_obj_center(bt_lb_obj);
    lv_obj_add_style(bt_lb_obj, def_text_style, 0);
    lv_label_set_text(bt_lb_obj, text);
    return bt_lb_obj;
}

/**
 * @brief 创建带标签的按钮
 */
lv_obj_t *tools_create_lbed_bt(lv_obj_t *parent_o, char *text, int width, int height, lv_color_t color) {
    lv_obj_t *bt_obj = lv_btn_create(parent_o);
    lv_obj_set_size(bt_obj, width, height);
    lv_obj_set_style_bg_color(bt_obj, color, LV_PART_MAIN);
    tools_add_bt_lb(bt_obj, text);
    return bt_obj;
}

/**
 * @brief 创建登录窗口复选框
 */
lv_obj_t *tools_create_login_checkbox(lv_obj_t *parent_o, int width, int height, char *text) {
    lv_obj_t *checkbox = lv_checkbox_create(parent_o);
    
    if (width != -1 && height != -1) {
        lv_obj_set_size(checkbox, width, height);
    }
    
    lv_checkbox_set_text(checkbox, text);
    lv_obj_add_style(checkbox, def_text_style, 0);
    
    return checkbox;
}

/**
 * @brief 关联输入框和键盘
 */
void tools_ta_kb_associate(lv_obj_t *ta, lv_obj_t *kb) {
    if (ta && kb) {
        lv_keyboard_set_textarea(kb, ta);
    }
}

/**
 * @brief 设置透明度样式
 */
void tools_set_opa_style(lv_style_t *opa_style, lv_obj_t *obj, lv_opa_t opa_value) {
    if (opa_style) {
        lv_style_init(opa_style);
        lv_style_set_bg_opa(opa_style, opa_value);
        lv_obj_add_style(obj, opa_style, 0);
    }
}

/**
 * @brief 设置背景样式
 */
void tools_set_bg_style(lv_style_t *bg_style, lv_obj_t *obj, lv_opa_t opa_value, char *fig_path) {
    if (bg_style && obj) {
        lv_obj_remove_style(obj, bg_style, LV_PART_ANY | LV_STATE_ANY);
        lv_style_init(bg_style);
        
        lv_style_set_bg_img_src(bg_style, fig_path);
        lv_style_set_bg_img_opa(bg_style, opa_value);
        
        lv_obj_add_style(obj, bg_style, 0);
    }
}

/**
 * @brief 初始化链表
 */
tools_dll_t *tools_init_dll_list() {
    tools_dll_t *head = (tools_dll_t *)malloc(sizeof(tools_dll_t));
    if (head) {
        head->data = NULL;
        head->num = 0;
        head->next = NULL;
        head->prev = NULL;
        head->tail = head;
    }
    return head;
}

/**
 * @brief 添加链表节点
 */
tools_dll_t *tools_add_dll_list_node(tools_dll_t *dll_list, void *data) {
    if (dll_list) {
        tools_dll_t *tmp = (tools_dll_t *)malloc(sizeof(tools_dll_t));
        if (tmp) {
            tmp->data = data;
            tmp->num = 1;
            tmp->prev = dll_list->tail;
            tmp->next = NULL;
            dll_list->tail->next = tmp;
            dll_list->tail = tmp;
            dll_list->num++;
        }
        return tmp;
    }
    return NULL;
}

/**
 * @brief 删除链表节点
 */
tools_dll_t *tools_delete_dll_list_node(tools_dll_t *dll_list, tools_dll_t *node) {
    if (dll_list && dll_list->next != NULL) {
        for (tools_dll_t *tmp_node = dll_list->next; tmp_node != NULL; tmp_node = tmp_node->next) {
            if (tmp_node == node) {
                tmp_node->prev->next = tmp_node->next;
                if (tmp_node == dll_list->tail) {
                    dll_list->tail = tmp_node->prev;
                } else {
                    tmp_node->next->prev = tmp_node->prev;
                }
                free(tmp_node);
                dll_list->num--;
                break;
            }
        }
    }
    return dll_list;
}

/**
 * @brief 查找链表节点
 */
tools_dll_t *tools_find_dll_list_node(tools_dll_t *dll_list, void *data) {
    if (dll_list) {
        for (tools_dll_t *tmp_node = dll_list->next; tmp_node != NULL; tmp_node = tmp_node->next) {
            if (tmp_node->data == data) {
                return tmp_node;
            }
        }
    }
    return NULL;
}

/**
 * @brief 修改链表节点数据
 */
int tools_modify_dll_list_node(tools_dll_t *dll_list, void *old_data, void *new_data) {
    if (dll_list) {
        tools_dll_t *modify_node = tools_find_dll_list_node(dll_list, old_data);
        if (modify_node) {
            modify_node->data = new_data;
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 根据用户标签查找同级对象
 */
lv_obj_t *tools_get_peer_obj_via_user_label(lv_obj_t *peer_obj, void *user_label) {
    lv_obj_t *item_window = lv_obj_get_parent(peer_obj);
    if (!item_window) return NULL;
    
    for (int i = 0; i < lv_obj_get_child_cnt(item_window); i++) {
        lv_obj_t *target_child = lv_obj_get_child(item_window, i);
        if (lv_obj_get_user_data(target_child) == user_label) {
            return target_child;
        }
    }
    return NULL;
}
