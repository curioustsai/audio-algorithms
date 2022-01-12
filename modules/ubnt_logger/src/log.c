/*
 *  Copyright (C) 2014, Ubiquiti Networks, Inc,
 *  Anton Obukhov (anton@ubnt.com, anton.obukhov@gmail.com)
 */

#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <search.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include "ubnt_logger/ubnt_logger.h"

static const char *LogColors[] = {
	"\033[01;31m", /* LOG_EMERG -> FATAL_COLOR */
	"\033[22;37m", /* LOG_ALERT -> FINE_COLOR */
	"\033[01;31m", /* LOG_CRIT -> FATAL_COLOR */
	"\033[22;31m", /* LOG_ERR -> ERROR_COLOR */
	"\033[01;33m", /* LOG_WARNING -> WARNING_COLOR */
	"\033[22;37m", /* LOG_NOTICE -> FINEST_COLOR */
	"\033[22;36m", /* LOG_INFO -> INFO_COLOR */
	"\033[01;37m" /* LOG_DEBUG -> DEBUG_COLOR */
};
static const char *LogColorNormal = "\033[0m";
#define LogColorStrLen                  8
#define LogColorNormalStrLen            4

#define _syslog(stdfd, pri, fmt, ...) \
	do { \
		if (g_LogFpSts) { \
			struct timeval tv; \
			gettimeofday(&tv, NULL); \
			if (g_LogFpColorSts) { \
				char *tmp1 = NULL, *tmp2 = NULL; \
				int dummy = asprintf(&tmp1, fmt,##__VA_ARGS__); \
				dummy = asprintf(&tmp2, "%s%s%s", LogColors[pri & LOG_PRIMASK], tmp1, LogColorNormal); \
				dummy = dummy; \
				fprintf((stdfd), "%ld.%06ld ", tv.tv_sec, tv.tv_usec); \
				fprintf((stdfd), "%s", tmp2); \
				free(tmp2); \
				free(tmp1); \
			} else { \
				fprintf((stdfd), "%ld.%06ld ", tv.tv_sec, tv.tv_usec); \
				fprintf((stdfd), fmt,##__VA_ARGS__); \
			} \
		} \
		if (g_LogSyslogSts) { \
			syslog(pri, fmt,##__VA_ARGS__); \
		} \
	} while (0)

FILE *g_LogFp = NULL;
int g_LogFpSts = 1;
int g_LogFpColorSts = 1;
int g_LogSyslogSts = 1;

void setLogFpFp(FILE *pFile) {
	g_LogFp = pFile;
}

void setLogFpSts(int on) {
	g_LogFpSts = on;
}

void setLogFpColorSts(int on) {
	g_LogFpColorSts = on;
}

void setLogSyslogSts(int on) {
	g_LogSyslogSts = on;
}

const char *logShortenFileName(const char *filename) {
	return strrchr(filename, '/') ? strrchr(filename, '/') + 1 : filename;
}

static void logInternalScheme0(
		const char *pBuffer,
		const char *pModuleName,
		const char *pFileName,
		const char *pFunctionName,
		int line,
		int level,
		int posixErrno) {
	if (posixErrno) {
		_syslog(g_LogFp ? g_LogFp : stdout, level, "%s (%d %s) [%s:%s:%s:%d]\n", pBuffer, posixErrno, strerror(posixErrno), pModuleName, logShortenFileName(pFileName), pFunctionName, line);
	} else {
		_syslog(g_LogFp ? g_LogFp : stdout, level, "%s [%s:%s:%s:%d]\n", pBuffer, pModuleName, logShortenFileName(pFileName), pFunctionName, line);
	}
}

//differs from 0 only by stderr

static void logInternalScheme2(
		const char *pBuffer,
		const char *pModuleName,
		const char *pFileName,
		const char *pFunctionName,
		int line,
		int level,
		int posixErrno) {
	if (posixErrno) {
		_syslog(g_LogFp ? g_LogFp : stderr, level, "%s (%d %s) [%s:%s:%s:%d]\n", pBuffer, posixErrno, strerror(posixErrno), pModuleName, logShortenFileName(pFileName), pFunctionName, line);
	} else {
		_syslog(g_LogFp ? g_LogFp : stderr, level, "%s [%s:%s:%s:%d]\n", pBuffer, pModuleName, logShortenFileName(pFileName), pFunctionName, line);
	}
}

static void logInternalScheme1(
		const char *pBuffer,
		const char *pModuleName,
		const char *pFileName,
		const char *pFunctionName,
		int line,
		int level,
		int posixErrno) {
	if (posixErrno) {
		_syslog(g_LogFp ? g_LogFp : stdout, level, "%s:%4d %s (%d %s)\n", pFileName, line, pBuffer, posixErrno, strerror(posixErrno));
	} else {
		_syslog(g_LogFp ? g_LogFp : stdout, level, "%s:%4d %s\n", pFileName, line, pBuffer);
	}
}

typedef void (*logInternalScheme)(
		const char *pBuffer,
		const char *pModuleName,
		const char *pFileName,
		const char *pFunctionName,
		int line,
		int level,
		int posixErrno);

static logInternalScheme logSchemes[] = {
	&logInternalScheme0,
	&logInternalScheme1,
	&logInternalScheme2
};

/*
 * Simplistic module filtering
 */
struct single_module_logging_t {
	/*
	 * the status of the single module logging:
	 * == 0 - _moduleName variable not yet read
	 * == 1 - _moduleName variable was read but it was empty/NULL. Logging will be
	 *     done normally, no filtering performed
	 * >= 2 - _moduleName variable was read and it was not empty/NULL. We only log
	 * the specified module name
	 */
	int _status;

#define MAX_MODULE_NAME 256
	/*
	 * The name of the module to filter on, when _status>=2
	 */
	char _moduleName[MAX_MODULE_NAME];
};

static struct single_module_logging_t singleMouleLogging = {
	._status = 0
};

/**
 * called as part of the canLog, when single_module_logging_t._status is 0 (not
 * initialized). It guarantees to set it to a value !=0, so it will be executed
 * only once
 */
static void initializeModuleFilter() {
	//try to open the file
	int fd = open("/etc/persistent/single_module_logging.conf", O_RDONLY);
	if (fd < 0) {
		singleMouleLogging._moduleName[0] = 0;
		singleMouleLogging._status = 1;
		return;
	}

	//read the file
	singleMouleLogging._moduleName[MAX_MODULE_NAME - 1] = 0;
	ssize_t totalRead = read(fd, singleMouleLogging._moduleName, MAX_MODULE_NAME - 1);
	if (totalRead <= 0) {
		singleMouleLogging._moduleName[0] = 0;
		singleMouleLogging._status = 1;
	} else {
		singleMouleLogging._moduleName[totalRead] = 0;
		//try to remove any ending \r\n
		int i = 0;
		for (i = 0; i < MAX_MODULE_NAME; i++) {
			if ((singleMouleLogging._moduleName[i] == '\r')
					|| (singleMouleLogging._moduleName[i] == '\n')
					|| (singleMouleLogging._moduleName[i] == 0)
					) {
				singleMouleLogging._moduleName[i] = 0;
				break;
			}
		}
		singleMouleLogging._status = (singleMouleLogging._moduleName[0] == 0) ? 1 : 2;
	}

	//close the file
	close(fd);
}

/**
 * This function is called for all log messages. It allows or disallows the logging
 * based on the module name. The provided module name is compared with the value
 * found in /etc/persistent/single_module_logging.txt. Only the first line from
 * that file is considered, and it is case sensitive.
 * @param pModuleName the module name to be evaluated. It is the value of
 * _UBNT_MODULE_NAME define which is set in the cmake files for all libs/apps
 * @return 0 if the loging should bot be performed or different from 0 if logging
 * should be performed
 */
static int canLog(const char *pModuleName) {
	//we need to initialize first, by reading the file on /tmp
	if (singleMouleLogging._status == 0)
		initializeModuleFilter();

	//if we did not enable module filtering, simply bail out by logging everything
	if (singleMouleLogging._status == 1)
		return 1;

	//we don't log if the provided module name does not match the module name
	//specified in the activation file
	return ((pModuleName == NULL) || (strncmp(pModuleName, singleMouleLogging._moduleName, MAX_MODULE_NAME - 1) != 0)) ? 0 : 1;
}

static pid_t getProcessId() {
	static pid_t s_pid = 0;
	if (s_pid == 0) {
		s_pid = getpid();
	}
	return s_pid;
}

static pid_t getThreadId() {
	static __thread pid_t s_tid = 0;
	if (s_tid == 0) {
		s_tid = syscall(SYS_gettid);
	}
	return s_tid;
}

void logInternal(
		const char *pModuleName,
		const char *pFileName,
		const char *pFunctionName,
		int line,
		int level,
		int posixErrno,
		unsigned logScheme,
		const char *pFormatString, ...) {
	//see if we filter on module name
	if (canLog(pModuleName) == 0)
		return;

	char *pBuffer = NULL;
	va_list arguments;

	if (logScheme >= sizeof (logSchemes) / sizeof (logSchemes[0])) {
		_syslog(stderr, LOG_EMERG, "Invalid logscheme [%s:%s:%s:%d]\n", pModuleName, pFileName, pFunctionName, line);
		exit(-1);
	}

	va_start(arguments, pFormatString);

	// for non-main thread let's amend format and insert the thread id
	pid_t tid = getThreadId();
	char* pfmt = NULL;
	if (getProcessId() != tid) {
		int maxFmtLen = strlen(pFormatString) + 16;
		pfmt = malloc(maxFmtLen);
		if (pfmt == NULL) {
			_syslog(stderr, LOG_EMERG, "malloc failed to allocate %d bytes\n", maxFmtLen);
			exit(-1);
		}
		snprintf(pfmt, maxFmtLen, "TID[%d] %s", tid, pFormatString);
		pFormatString = pfmt;
	}

	if (vasprintf(&pBuffer, pFormatString, arguments) < 0) {
		_syslog(stderr, LOG_EMERG, "vasprintf failed while reporting error [%s:%s:%s:%d]\n", pModuleName, pFileName, pFunctionName, line);
		free(pfmt);
		exit(-1);
	}

	va_end(arguments);
	logSchemes[logScheme](pBuffer, pModuleName, pFileName, pFunctionName, line, level, posixErrno);

	free(pfmt);
	free(pBuffer);
}

int logFilter(int level) {
	int logmask = setlogmask(LOG_UPTO(level));
	int prevLevel = (logmask & LOG_MASK(LOG_DEBUG)) ? LOG_DEBUG :
					(logmask & LOG_MASK(LOG_INFO)) ? LOG_INFO :
					(logmask & LOG_MASK(LOG_NOTICE)) ? LOG_NOTICE :
					(logmask & LOG_MASK(LOG_WARNING)) ? LOG_WARNING :
					(logmask & LOG_MASK(LOG_ERR)) ? LOG_ERR :
					(logmask & LOG_MASK(LOG_CRIT)) ? LOG_CRIT :
					(logmask & LOG_MASK(LOG_ALERT)) ? LOG_ALERT :
					(logmask & LOG_MASK(LOG_EMERG)) ? LOG_EMERG :
					0;
	return prevLevel;
}

size_t logHexStr(void *ptr, size_t sz, char *out, size_t outLen) {
	int i;
	uint8_t *_ptr = (uint8_t *) ptr;
	char *_out = out;
	int srcLen = (int) (outLen + 1) / 3;
	srcLen = srcLen > (int) sz ? (int) sz : srcLen;

	for (i = 0; i < srcLen; i++) {
		int hi = (*_ptr >> 4) & 0xF;
		int lo = *_ptr & 0xF;
		_ptr++;
		*_out++ = (char) ((hi < 10) ? ('0' + hi) : ('A' + hi - 10));
		*_out++ = (char) ((lo < 10) ? ('0' + lo) : ('A' + lo - 10));

		if (i != srcLen - 1) {
			*_out++ = ' ';
		}
	}

	return _out - out;
}

void forceCrash() {
	//if assert(0) doesn't do anything (is dependent on NDEBUG define), than call abort()
	//if abort() doesn't do anything either, than call exit(-1) as a last resort
	assert(0);
	abort();
	exit(-1);
}

typedef struct {
	char const *logLevelName;
	int logLevel;
} LogLevelItem ;

static int compareLogLevelName(void const *key, void const *value) {
	char const *logLevelName = (char const *)key;
	LogLevelItem *item = (LogLevelItem *)value;
	return strcmp(logLevelName, item->logLevelName);
}

int toLogLevel(char const *logLevelName, int defaultLevel) {
	static const LogLevelItem s_logLevelMap[] = {
		{"DEBUG", LOG_DEBUG},
		{"LOG_DEBUG", LOG_DEBUG},
		{"INFO", LOG_INFO},
		{"LOG_INFO", LOG_INFO},
		{"NOTICE", LOG_NOTICE},
		{"LOG_NOTICE", LOG_NOTICE},
		{"WARN", LOG_WARNING},
		{"WARNING", LOG_WARNING},
		{"LOG_WARNING", LOG_WARNING},
		{"ERR", LOG_ERR},
		{"ERROR", LOG_ERR},
		{"LOG_ERR", LOG_ERR},
		{"CRIT", LOG_CRIT},
		{"CRITICAL", LOG_CRIT},
		{"LOG_CRIT", LOG_CRIT},
		{"ALERT", LOG_ALERT},
		{"LOG_ALERT", LOG_ALERT},
		{"EMERG", LOG_EMERG},
		{"EMERGENCY", LOG_EMERG},
		{"LOG_EMERG", LOG_EMERG}
	};

	size_t nmemb = sizeof(s_logLevelMap) / sizeof(s_logLevelMap[0]);
	void *found = logLevelName == NULL
					? NULL
					: lfind(logLevelName, s_logLevelMap, &nmemb,
						sizeof(s_logLevelMap[0]), compareLogLevelName);

	return found != NULL ? ((LogLevelItem*)found)->logLevel : defaultLevel;
}
