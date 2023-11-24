//
// Created by XIAOSENLUO on 2023/11/24.
//

#include <stdarg.h>
#include "log/log.h"
#include "printf.h"

#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

#if LOG_USING_COLOR == 1
#define memPrintHead CSI(31) \
    "    Offset: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F" \
    CSI(39) \
    "\r\n"
#define memPrintAddr CSI(31)"0x%08x: "CSI(39)
#else
#define memPrintHead "    Offset: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n"
#define memPrintAddr "0x%08x: "
#endif

log_t *logList[LOG_MAX_NUMBER] = {0};
static char logBuffer[LOG_BUFFER_SIZE];

struct llog_def {
    log_t *log;
    log_level level;
};

#if LOG_USING_LOCK == 1
/**
 * @brief   上锁log对象
 * @param   log log对象
 */
static void log_lock(log_t *log)
{
    if (log == LOG_ALL_OBJ)
    {
        for (short i = 0; i < LOG_MAX_NUMBER; i++)
        {
            if (logList[i] && logList[i]->active)
            {
                if (logList[i]->lock)
                {
                    LOG_LOCK(logList[i]);
                }
            }
        }
    }
    else if (log->lock)
    {
        LOG_LOCK(log);
    }
}

/**
 * @brief   解锁log对象
 * @param   log log对象
 */
static void log_unlock(log_t *log)
{
    if (log == LOG_ALL_OBJ)
    {
        for (short i = 0; i < LOG_MAX_NUMBER; i++)
        {
            if (logList[i] && logList[i]->active)
            {
                if (logList[i]->unlock)
                {
                    LOG_UNLOCK(logList[i]);
                }
            }
        }
    }
    else if (log->unlock)
    {
        LOG_UNLOCK(log);
    }
}
#endif /* LOG_USING_LOCK == 1 */

/**
 * @brief 注册log对象
 *
 * @param log log对象
 */
void log_register(log_t *log)
{
    for (short i = 0; i < LOG_MAX_NUMBER; i++)
    {
        if (logList[i] == 0)
        {
            logList[i] = log;
            return;
        }
    }
}


/**
 * @brief 注销log对象
 *
 * @param log log对象
 */
void log_unregister(log_t *log)
{
    for (short i = 0; i < LOG_MAX_NUMBER; i++)
    {
        if (logList[i] == log)
        {
            logList[i] = 0;
            return;
        }
    }
}


/**
 * @brief 设置log日志级别
 *
 * @param log log对象
 * @param level 日志级别
 */
void log_set_level(log_t *log, log_level level)
{
    log_assert(log, return);
    log->level = level;
}


/**
 * @brief log写buffer
 *
 * @param log log对象
 * @param level 日志级别
 * @param buffer buffer
 * @param len buffer长度
 */
static void log_write_buffer(log_t *log, log_level level, char *buffer, short len)
{
    if(logList[0] == NULL) return;
//#if LOG_USING_LOCK == 1
//    log_lock(log);
//#endif /* LOG_USING_LOCK == 1 */
    if (log == LOG_ALL_OBJ)
    {
        for (short i = 0; i < LOG_MAX_NUMBER; i++)
        {
            if (logList[i]
                && logList[i]->active
                && logList[i]->level >= level)
            {
                logList[i]->write(logBuffer, len);
            }
        }
    }
    else if (log && log->active && log->level >= level)
    {
        log->write(logBuffer, len);
    }
//#if LOG_USING_LOCK == 1
//    log_unlock(log);
//#endif /* LOG_USING_LOCK == 1 */
}

static void log_write_out_fn(char ch, void *arg){
    struct llog_def *ll = (struct llog_def *)arg;

    if((ll) || (logList[0] == NULL) || (ll->log == NULL)) return;

    logBuffer[ll->log->w++] = ch;

    if((ll->log->w >= (LOG_BUFFER_SIZE - 2)) || (ch == '\n')){
        if(ll->log->flag){
            for (short i = 0; i < LOG_MAX_NUMBER; i++)
            {
                if (logList[i]
                    && logList[i]->active
                    && logList[i]->level >= ll->level)
                {
                    logList[i]->write(logBuffer, ll->log->w);
                }
            }
            ll->log->w = 0;
        }else if(ll->log && ll->log->active && ll->log->level >= ll->level){
            ll->log->write(logBuffer, ll->log->w);
            ll->log->w = 0;
        }
    }
}

/**
 * @brief log格式化写入数据
 *
 * @param log log对象
 * @param level log级别
 * @param fmt 格式
 * @param ... 参数
 */
void log_write(log_t *log, log_level level, const char *fmt, ...)
{
    va_list vargs;
    int len;

    if(logList[0] == NULL) return;
#if LOG_USING_LOCK == 1
    log_lock(log);
#endif /* LOG_USING_LOCK == 1 */
#if(0)
    va_start(vargs, fmt);
    len = vsnprintf_(logBuffer, LOG_BUFFER_SIZE - 1, fmt, vargs);
    va_end(vargs);

    if (len > LOG_BUFFER_SIZE)
    {
        len = LOG_BUFFER_SIZE;
    }

    log_write_buffer(log, level, logBuffer, len);
#else
    struct llog_def llog = {.log = log, .level = level};
    va_start(vargs, fmt);
    if(log == LOG_ALL_OBJ){
        int w = logList[0]->w;
        logList[0]->flag = 1;
        llog.log = logList[0];
        llog.level = level;
        len = vfctprintf(log_write_out_fn, &llog, fmt, vargs);
        (void)len;
        logList[0]->w = (short)w;
        logList[0]->flag = 0;
    }else{
        len = vfctprintf(log_write_out_fn, &llog, fmt, vargs);
        (void)len;
    }

    va_end(vargs);
#endif

#if LOG_USING_LOCK == 1
    log_unlock(log);
#endif /* LOG_USING_LOCK == 1 */
}

/**
 * @brief 16进制输出
 *
 * @param log log对象
 * @param level 日志级别
 * @param base 内存基址
 * @param length 长度
 */
void log_hex_dump(log_t *log, log_level level, void *base, unsigned int length)
{
    unsigned char *address;
    unsigned int len;
    unsigned int printLen = 0;

    if (length == 0 || (log != LOG_ALL_OBJ && log->level < level))
    {
        return;
    }
#if LOG_USING_LOCK == 1
    log_lock(log);
#endif /* LOG_USING_LOCK == 1 */
    len = snprintf_(logBuffer, LOG_BUFFER_SIZE - 1, "memory of 0x%08x, size: %d:\r\n%s",
                   (unsigned int)base, length, memPrintHead);
    log_write_buffer(log, level, logBuffer, len);

    len = length;

    address = (unsigned char *)((unsigned int)base & (~0x0000000F));
    length += (unsigned int)base - (unsigned int)address;
    length = (length + 15) & (~0x0000000F);

    while (length)
    {
        printLen += sprintf_(logBuffer + printLen, memPrintAddr, (unsigned int)address);
        for (int i = 0; i < 16; i++)
        {
            if ((unsigned int)(address + i) < (unsigned int)base
                || (unsigned int)(address + i) >= (unsigned int)base + len)
            {
                logBuffer[printLen ++] = ' ';
                logBuffer[printLen ++] = ' ';
                logBuffer[printLen ++] = ' ';
            }
            else
            {
                printLen += sprintf_(logBuffer + printLen, "%02x ", *(address + i));
            }
        }
        logBuffer[printLen ++] = '|';
        logBuffer[printLen ++] = ' ';
        for (int i = 0; i < 16; i++)
        {
            if ((unsigned int)(address + i) < (unsigned int)base
                || (unsigned int)(address + i) >= (unsigned int)base + len)
            {
                logBuffer[printLen ++] = ' ';
            }
            else
            {
                if (*(address + i) >= 32 && *(address + i) <= 126)
                {
                    printLen += sprintf_(logBuffer + printLen, "%c", *(address + i));
                }
                else
                {
                    logBuffer[printLen ++] = '.';
                }
            }
        }
        logBuffer[printLen ++] = ' ';
        logBuffer[printLen ++] = '|';
        logBuffer[printLen ++] = '\r';
        logBuffer[printLen ++] = '\n';
        log_write_buffer(log, level, logBuffer, printLen);
        address += 16;
        length -= 16;
        printLen = 0;
    }
#if LOG_USING_LOCK == 1
    log_unlock(log);
#endif /* LOG_USING_LOCK == 1 */
}

#ifdef __cplusplus
}
#endif
