/*
 *  Copyright (C) 2014, Ubiquiti Networks, Inc,
 *  Anton Obukhov (anton@ubnt.com, anton.obukhov@gmail.com)
 */

#ifndef LOG_H_
#define LOG_H_

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#define _STRINGIZE(x)   #x
#define STRINGIZE(x)    _STRINGIZE(x)

#define UBNT_MODULE_NAME STRINGIZE(_UBNT_MODULE_NAME)

#define CONCAT(x, y)    x ## y

#ifdef __cplusplus
extern "C" {
#endif
	// return the previous log level
	int logFilter(int level);
	void logInternal(const char *pModuleName, const char *pFileName, const char *pFunctionName, \
			int line, int level, int posixErrno, unsigned logScheme, const char *pFormatString, ...);
	size_t logHexStr(void *ptr, size_t sz, char *out, size_t outLen);
	void forceCrash();
	void setLogFpFp(FILE *pFile);
	void setLogFpSts(int on);
	void setLogFpColorSts(int on);
	void setLogSyslogSts(int on);

	// Convert log level by its name to the corresponding log level value like this:
	//		"DEBUG" or "LOG_DEBUG -> LOG_DEBUG
	//		"INFO" or "LOG_INFO -> LOG_INFO
	//		"WARNIG" or "LOG_WARNING -> LOG_WARNING
	//		...
	// Fallback to the specified "defaultLevel" if the logLevelName is null or not valid
	int toLogLevel(char const* logLevelName, int defaultLevel);
#ifdef __cplusplus
}
#endif

#ifndef UBNT_GLOBAL_LOG_SCHEME
#define UBNT_GLOBAL_LOG_SCHEME 0
#endif

#ifdef GEN2
#define THREAD_LOG(level, format, ...) do { \
	char thName[32]={0}; \
	prctl(PR_GET_NAME, thName, 0, 0, 0); \
	pid_t tid = syscall(SYS_gettid); \
	logInternal(STRINGIZE(_UBNT_MODULE_NAME), __FILE__, __FUNCTION__, __LINE__, level, 0, UBNT_GLOBAL_LOG_SCHEME, \
		"%s(%d) - " format,thName,tid, ##__VA_ARGS__); \
} while(0)
#else
#define THREAD_LOG(level, format, ...) do { \
	static thread_local char const* thName = [] { \
        thread_local static char thName[32]; \
        prctl(PR_GET_NAME, thName, 0, 0, 0); \
        return thName; }(); \
	static thread_local pid_t tid = syscall(SYS_gettid); \
	logInternal(STRINGIZE(_UBNT_MODULE_NAME), __FILE__, __FUNCTION__, __LINE__, level, 0, UBNT_GLOBAL_LOG_SCHEME, \
		"%s(%d) - " format,thName,tid, ##__VA_ARGS__); \
} while(0)
#endif

#define LOG(level, format, ...) \
	logInternal(STRINGIZE(_UBNT_MODULE_NAME), __FILE__, __FUNCTION__, __LINE__, level, 0, \
			UBNT_GLOBAL_LOG_SCHEME, format, ##__VA_ARGS__)
#define LOGE(level, posixErrno, format, ...) \
	logInternal(STRINGIZE(_UBNT_MODULE_NAME), __FILE__, __FUNCTION__, __LINE__, level, posixErrno, \
			UBNT_GLOBAL_LOG_SCHEME, format, ##__VA_ARGS__)
#define LOG_HEX(ptr, sz, format, ...) \
	do { \
		char buf[1024]; \
		size_t hexLen = logHexStr(ptr, sz, buf, sizeof(buf)-1); \
		snprintf(buf + hexLen, sizeof(buf) - hexLen, " " format, ##__VA_ARGS__); \
		logInternal(STRINGIZE(_UBNT_MODULE_NAME), __FILE__, __FUNCTION__, __LINE__, LOG_DEBUG, 0, \
				UBNT_GLOBAL_LOG_SCHEME, "%s", buf); \
	} while (0)

#define ASSERT_LOG(level, val, format, ...) \
	do { if (!(val)) { LOG(level, format, ##__VA_ARGS__); } } while (0)
#define ASSERT_LOGE(level, val, format, ...) \
	do { if (!(val)) { LOGE(level, errno, format, ##__VA_ARGS__); } } while (0)
#define ASSERT_LOG_RET(val, ret, format, ...) \
	do { if (!(val)) { LOG(LOG_ERR, format, ##__VA_ARGS__); return ret; } } while (0)
#define ASSERT_LOGE_RET(val, ret, format, ...) \
	do { if (!(val)) { LOGE(LOG_ERR, errno, format, ##__VA_ARGS__); return ret; } } while (0)
#define ASSERT_LOG_EXIT(val, format, ...) \
	do { if (!(val)) { LOG(LOG_EMERG, "TERMINATING, " format, ##__VA_ARGS__);  forceCrash(); } } while (0)
#define ASSERT_LOGE_EXIT(val, format, ...) \
	do { if (!(val)) { LOGE(LOG_EMERG, errno, "TERMINATING, " format, ##__VA_ARGS__); forceCrash(); } } while (0)
#define ASSERT_LOG_ACT(val, act, ret, format, ...) \
	do { if (!(val)) { LOG(LOG_ERR, format, ##__VA_ARGS__); act; return ret; } } while (0)
#define ASSERT_LOGE_ACT(val, act, ret, format, ...) \
	do { if (!(val)) { LOGE(LOG_ERR, errno, format, ##__VA_ARGS__); act; return ret; } } while (0)
#define ASSERT_RET(val, ret) \
	do { if (!(val)) { return ret; } } while (0)

#define FATAL_ERRNO(...) \
	LOGE(LOG_ERR, errno, __VA_ARGS__)
#define FATAL(...) \
	LOG(LOG_ERR, __VA_ARGS__)

#define WARN_ERRNO(...) \
	LOGE(LOG_WARNING, errno, __VA_ARGS__)
#define WARN(...) \
	LOG(LOG_WARNING, __VA_ARGS__)

#define INFO(...) \
	LOG(LOG_INFO, __VA_ARGS__)

#define FINEST(...) \
	LOG(LOG_DEBUG, __VA_ARGS__)
#define ASSERT_ERRNO(...) \
	do { FATAL_ERRNO(__VA_ARGS__);  forceCrash(); } while (0)
#define ASSERT(...) \
	do { FATAL(__VA_ARGS__);  forceCrash(); } while (0)

#define NYI \
	WARN("Function %s not yet implemented", __func__)
#define NYIR \
	do { WARN("Function %s not yet implemented", __func__); return false; } while (0)
#define NYIA \
	do { WARN("Function %s not yet implemented", __func__); forceCrash(); } while (0)

#define THREAD_DEBUG(...) THREAD_LOG(LOG_DEBUG, __VA_ARGS__)
#define THREAD_INFO(...) THREAD_LOG(LOG_INFO, __VA_ARGS__)
#define THREAD_WARN(...) THREAD_LOG(LOG_WARNING, __VA_ARGS__)
#define THREAD_FATAL(...) THREAD_LOG(LOG_ERR, __VA_ARGS__)

#endif  // LOG_H_
