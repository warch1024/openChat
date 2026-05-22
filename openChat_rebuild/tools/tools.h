#ifndef _TOOLS_H
#define _TOOLS_H

#include "lvgl/lvgl.h"

// 全局字体样式
extern lv_style_t *def_text_style;

// 拼音输入法结构体
typedef struct {
    lv_obj_t *cn_kb;
    lv_obj_t *cn_kb_cp;
} cn_kb_cp_ros;

// 双向链表结构
typedef struct Tools_dll_t {
    void *data;
    uint32_t num;
    struct Tools_dll_t *next;
    struct Tools_dll_t *prev;
    struct Tools_dll_t *tail;
} tools_dll_t;

/**
 * @brief 创建字体样式
 * @param style 样式指针
 * @param font_path 字体路径
 * @param font_size 字体大小
 */
void tools_create_font_style(lv_style_t *style, char *font_path, int font_size);

/**
 * @brief 创建拼音输入法
 * @param parent_obj 父对象
 * @param weight 宽度
 * @param height 高度
 * @return 键盘和候选栏的结构体
 */
cn_kb_cp_ros tools_create_pinyin_ime(lv_obj_t *parent_obj, int weight, int height);

/**
 * @brief 显示拼音键盘
 * @param kb 键盘对象
 * @param cp 候选栏对象
 */
void tools_show_pinyin_kb(lv_obj_t *kb, lv_obj_t *cp);

/**
 * @brief 隐藏拼音键盘
 * @param kb 键盘对象
 * @param cp 候选栏对象
 */
void tools_hidden_pinyin_kb(lv_obj_t *kb, lv_obj_t *cp);

/**
 * @brief 创建登录窗口输入框
 * @param parent_o 父对象
 * @param width 宽度
 * @param height 高度
 * @param pht 提示文本
 * @return 输入框对象
 */
lv_obj_t *tools_create_login_ta(lv_obj_t *parent_o, int width, int height, char *pht);

/**
 * @brief 创建通用输入框
 * @param parent_o 父对象
 * @param width 宽度
 * @param height 高度
 * @param pht 提示文本
 * @return 输入框对象
 */
lv_obj_t *tools_create_common_ta(lv_obj_t *parent_o, int width, int height, char *pht);

/**
 * @brief 创建登录按钮标签
 * @param parent_o 父对象
 * @param text 文本内容
 * @return 标签对象
 */
lv_obj_t *tools_create_login_bt_lb(lv_obj_t *parent_o, char *text);

/**
 * @brief 创建按钮标签
 * @param parent_o 父对象
 * @param text 文本内容
 * @return 标签对象
 */
lv_obj_t *tools_add_bt_lb(lv_obj_t *parent_o, char *text);

/**
 * @brief 创建带标签的按钮
 * @param parent_o 父对象
 * @param text 按钮文本
 * @param width 宽度
 * @param height 高度
 * @param color 按钮颜色
 * @return 按钮对象
 */
lv_obj_t *tools_create_lbed_bt(lv_obj_t *parent_o, char *text, int width, int height, lv_color_t color);

/**
 * @brief 创建登录窗口复选框
 * @param parent_o 父对象
 * @param width 宽度
 * @param height 高度
 * @param text 文本内容
 * @return 复选框对象
 */
lv_obj_t *tools_create_login_checkbox(lv_obj_t *parent_o, int width, int height, char *text);

/**
 * @brief 关联输入框和键盘
 * @param ta 输入框对象
 * @param kb 键盘对象
 */
void tools_ta_kb_associate(lv_obj_t *ta, lv_obj_t *kb);

/**
 * @brief 设置透明度样式
 * @param opa_style 样式对象
 * @param obj 目标对象
 * @param opa_value 透明度值
 */
void tools_set_opa_style(lv_style_t *opa_style, lv_obj_t *obj, lv_opa_t opa_value);

/**
 * @brief 设置背景样式
 * @param bg_style 样式对象
 * @param obj 目标对象
 * @param opa_value 透明度值
 * @param fig_path 背景图片路径
 */
void tools_set_bg_style(lv_style_t *bg_style, lv_obj_t *obj, lv_opa_t opa_value, char *fig_path);

/**
 * @brief 初始化链表
 * @return 链表头节点
 */
tools_dll_t *tools_init_dll_list();

/**
 * @brief 添加链表节点
 * @param dll_list 链表头节点
 * @param data 节点数据
 * @return 新添加的节点
 */
tools_dll_t *tools_add_dll_list_node(tools_dll_t *dll_list, void *data);

/**
 * @brief 删除链表节点
 * @param dll_list 链表头节点
 * @param node 要删除的节点
 * @return 链表头节点
 */
tools_dll_t *tools_delete_dll_list_node(tools_dll_t *dll_list, tools_dll_t *node);

/**
 * @brief 查找链表节点
 * @param dll_list 链表头节点
 * @param data 查找的数据
 * @return 找到的节点
 */
tools_dll_t *tools_find_dll_list_node(tools_dll_t *dll_list, void *data);

/**
 * @brief 修改链表节点数据
 * @param dll_list 链表头节点
 * @param old_data 旧数据
 * @param new_data 新数据
 * @return 成功返回1，失败返回0
 */
int tools_modify_dll_list_node(tools_dll_t *dll_list, void *old_data, void *new_data);

/**
 * @brief 根据用户标签查找同级对象
 * @param peer_obj 同级对象
 * @param user_label 用户标签
 * @return 找到的对象
 */
lv_obj_t *tools_get_peer_obj_via_user_label(lv_obj_t *peer_obj, void *user_label);

#endif // _TOOLS_H
