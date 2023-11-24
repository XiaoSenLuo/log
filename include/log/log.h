//
// Created by XIAOSENLUO on 2023/11/24.
//

#ifndef __LOG_H__
#define __LOG_H__
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#include <stdint.h>
#endif

#define     LOG_VERSION        "1.0.1"

#define     LOG_USING_LOCK     1
#define     LOG_BUFFER_SIZE    1024              /**< log输出缓冲大小 */
#define     LOG_USING_COLOR    1                /**< 是否使用颜色 */
#define     LOG_MAX_NUMBER     2                /**< 允许注册的最大log对象数量 */
#define     LOG_AUTO_TAG       1                /**< 是否自动添加TAG */
#define     LOG_END            "\r\n"           /**< log信息结尾 */
#define     LOG_TIME_STAMP     1                /**< 设置获取系统时间戳 */

#ifndef LOG_TAG
#define LOG_TAG            __FUNCTION__     /**< 自定添加的TAG */
#endif

#ifndef     LOG_ENABLE
#define LOG_ENABLE         1                /**< 使能log */
#endif

#if LOG_USING_LOCK == 1
#define     LOG_LOCK(log)           log->lock(log)
#define     LOG_UNLOCK(log)         log->unlock(log)
#else
#define     LOG_LOCK(log)
#define     LOG_UNLOCK(log)
#endif /* LOG_USING_LOCK == 1 */

#define     LOG_ALL_OBJ        ((log_t *)-1)      /**< 所有已注册的log对象 */

/**
 * 终端字体颜色代码
 */
#define     CSI_BLACK           30              /**< 黑色 */
#define     CSI_RED             31              /**< 红色 */
#define     CSI_GREEN           32              /**< 绿色 */
#define     CSI_YELLOW          33              /**< 黄色 */
#define     CSI_BLUE            34              /**< 蓝色 */
#define     CSI_FUCHSIN         35              /**< 品红 */
#define     CSI_CYAN            36              /**< 青色 */
#define     CSI_WHITE           37              /**< 白色 */
#define     CSI_BLACK_L         90              /**< 亮黑 */
#define     CSI_RED_L           91              /**< 亮红 */
#define     CSI_GREEN_L         92              /**< 亮绿 */
#define     CSI_YELLOW_L        93              /**< 亮黄 */
#define     CSI_BLUE_L          94              /**< 亮蓝 */
#define     CSI_FUCHSIN_L       95              /**< 亮品红 */
#define     CSI_CYAN_L          96              /**< 亮青 */
#define     CSI_WHITE_L         97              /**< 亮白 */
#define     CSI_DEFAULT         39              /**< 默认 */

#define     CSI(code)           "\033[" #code "m"   /**< ANSI CSI指令 */

/**
 * log级别字符(包含颜色)
 */
#if LOG_USING_COLOR == 1
#define     ERROR_TEXT          CSI(31) "E(%d) %s:" CSI(39)     /**< 错误标签 */
#define     WARNING_TEXT        CSI(33) "W(%d) %s:" CSI(39)     /**< 警告标签 */
#define     INFO_TEXT           CSI(32) "I(%d) %s:" CSI(39)     /**< 信息标签 */
#define     DEBUG_TEXT          CSI(34) "D(%d) %s:" CSI(39)     /**< 调试标签 */
#define     VERBOSE_TEXT        CSI(36) "V(%d) %s:" CSI(39)     /**< 冗余信息标签 */
#else
#define     ERROR_TEXT          "E(%d) %s:"
#define     WARNING_TEXT        "W(%d) %s:"
#define     INFO_TEXT           "I(%d) %s:"
#define     DEBUG_TEXT          "D(%d) %s:"
#define     VERBOSE_TEXT        "V(%d) %s:"
#endif


/**
 * @brief 日志级别定义
 *
 */
typedef enum
{
    LOG_NONE = 0,                                  /**< 无级别 */
    LOG_ERROR = 1,                                 /**< 错误 */
    LOG_WRANING = 2,                               /**< 警告 */
    LOG_INFO = 3,                                  /**< 消息 */
    LOG_DEBUG = 4,                                 /**< 调试 */
    LOG_VERBOSE = 5,                               /**< 冗余 */
    LOG_ALL = 6,                                   /**< 所有日志 */
} log_level;


/**
 * @brief log对象定义
 *
 */
typedef struct log_def
{
    char active;                                    /**< 是否激活 */
    char flag;
#ifdef __GNUC__
    int16_t w;
#else
    short w;
#endif
    void (*write)(char *, short);                   /**< 写buffer */
    log_level level;                                 /**< 日志级别 */
#if LOG_USING_LOCK == 1
    int (*lock)(struct log_def *);                  /**< log 加锁 */
    int (*unlock)(struct log_def *);                /**< log 解锁 */
#endif /** LOG_USING_LOCK == 1 */

} log_t;



/**
 * @brief log打印(自动换行)
 *
 * @param fmt 格式
 * @param ... 参数
 */
#define log_println(format, ...) \
        log_write(LOG_ALL_OBJ, LOG_NONE, format "\r\n", ##__VA_ARGS__)


/**
 * @brief 日志格式化输出
 *
 * @param text 消息文本
 * @param level 日志级别
 * @param fmt 格式
 * @param ... 参数
 */
#define log_format(text, level, fmt, ...) \
        if (LOG_ENABLE) {\
            log_write(LOG_ALL_OBJ, level, text " " fmt "" LOG_END, \
                LOG_TIME_STAMP, LOG_TAG, ##__VA_ARGS__); }

/**
 * @brief 错误log输出
 *
 * @param fmt 格式
 * @param ... 参数
 */
#define log_error(fmt, ...) \
        log_format(ERROR_TEXT, LOG_ERROR, fmt, ##__VA_ARGS__)

/**
 * @brief 警告log输出
 *
 * @param fmt 格式
 * @param ... 参数
 */
#define log_warning(fmt, ...) \
        log_format(WARNING_TEXT, LOG_WRANING, fmt, ##__VA_ARGS__)

/**
 * @brief 信息log输出
 *
 * @param fmt 格式
 * @param ... 参数
 */
#define log_info(fmt, ...) \
        log_format(INFO_TEXT, LOG_INFO, fmt, ##__VA_ARGS__)

/**
 * @brief 调试log输出
 *
 * @param fmt 格式
 * @param ... 参数
 */
#define log_debug(fmt, ...) \
        log_format(DEBUG_TEXT, LOG_DEBUG, fmt, ##__VA_ARGS__)

/**
 * @brief 冗余log输出
 *
 * @param fmt 格式
 * @param ... 参数
 */
#define log_verbose(fmt, ...) \
        log_format(VERBOSE_TEXT, LOG_VERBOSE, fmt, ##__VA_ARGS__)

/**
 * @brief 断言
 *
 * @param expr 表达式
 * @param action 断言失败操作
 */
#define log_assert(expr, action) \
        if (!(expr)) { \
            log_error("\"" #expr "\" assert failed at file: %s, line: %d", __FILE__, __LINE__); \
            action; \
        }

/**
 * @brief 16进制输出到所有终端
 *
 * @param base 内存基址
 * @param length 长度
 */
#define log_hex_dump_all(base, length) \
        log_hex_dump(LOG_ALL_OBJ, LOG_ALL, base, length)

void log_register(log_t *log);
void log_unregister(log_t *log);
void log_set_level(log_t *log, log_level level);
void log_write(log_t *log, log_level level, const char *fmt, ...);
void log_hex_dump(log_t *log, log_level level, void *base, unsigned int length);


#ifdef __cplusplus
}
#endif
#endif //__LOG_H__
