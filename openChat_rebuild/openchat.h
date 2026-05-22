#ifndef _OPENCHAT_H
#define _OPENCHAT_H

#include "screen/login_screen.h"
#include "screen/main_screen.h"
#include "core/user_manager.h"
#include "core/client.h"
#include "tools/tools.h"

/**
 * @brief 初始化并启动OpenChat应用
 * @return 成功返回0，失败返回-1
 */
int openchat_init(void);

/**
 * @brief 清理OpenChat应用资源
 */
void openchat_cleanup(void);

#endif // _OPENCHAT_H
