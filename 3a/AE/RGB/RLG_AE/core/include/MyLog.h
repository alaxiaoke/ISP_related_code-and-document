#ifndef _MY_LOG_H_
#define _MY_LOG_H_

#include <cstdio>
#include <cstring>
#include <ctime>
#include <direct.h>
#include <cstdarg>

//#include <iostream>
using namespace std;

typedef enum LogLevel_em
{
	LogLevel_OFF	= 0,

	LogLevel_Trace	= 1 << 0,
	LogLevel_Debug	= 1 << 1,
	LogLevel_Info	= 1 << 2,
	LogLevel_Warn	= 1 << 3,
	LogLevel_Error	= 1 << 4,
	LogLevel_Fatal	= 1 << 5,

	LogLevel_end
};

#define PER_LOG_LINE_SIZE 512
#define LOG_PATH "AElog"




//static enum LogLevel_em w_emCurLogLevel = LogLevel_OFF;       //log level switch  -->add new func to update loglevel
//static int g_log_terminal = 1;
//static int g_log_to_file = 0;


extern enum LogLevel_em w_emCurLogLevel;       //log level switch  -->add new func to update loglevel
extern int g_log_terminal;
extern int g_log_to_file;
extern char* logbuffer;

static char g_log_root[64] = { 0 };


static time_t timep;
static struct tm* p;

void printf_log(
	enum LogLevel_em level, // 日志级别
	const char* file,  //打印日志所在的文件
	const char* function,//打印日志的函数名
	int line, 		//打印日志的行号
	const char* format,//日志格式char* w_Msg, uint32_t u32MsgLen
	...
);

void write_log_2_file(
	const char* file_name,//需要保存的日志文件名
	enum LogLevel_em level, // 日志级别
	const char* file,  //打印日志所在的文件
	const char* function,//打印日志的函数名
	int   line, //打印日志的行号
	const char* msg,
	...
);


void printf_log(
	enum LogLevel_em level, // 日志级别
	const char* file,  //打印日志所在的文件
	const char* function,//打印日志的函数名
	int line, 		//打印日志的行号
	const char* format,//日志格式char* w_Msg, uint32_t u32MsgLen
	...
);

static void write_log_2_file(
	const char* file_name,//需要保存的日志文件名
	enum LogLevel_em level, // 日志级别
	const char* file,  //打印日志所在的文件
	const char* function,//打印日志的函数名
	int   line, //打印日志的行号
	const char* msg,
	...
);

static void saveFileLog();


#define LOG_TRACE(format, ...)  printf_log(log_str_2_level("TRACE"),__FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define LOG_DEBUG(format, ...) printf_log(log_str_2_level("DEBUG"),__FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define LOG_INFO(format, ...)  printf_log(log_str_2_level("INFO"),__FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  printf_log(log_str_2_level("WARNING"),__FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) printf_log(log_str_2_level("ERROR"),__FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) printf_log(log_str_2_level("FATAL"),__FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LOG_SAVE saveFileLog()



static void log_terminal()
{
	g_log_terminal = TRUE;
}

static void log_shutdown_terminal()
{
	g_log_terminal = FALSE;
}

static void log_shutdown_file()
{
	g_log_to_file = FALSE;
}

static void log_to_file()
{
	g_log_to_file = TRUE;
}

static char* log_level_2_str(enum LogLevel_em level)
{
	switch (level)
	{
	case LogLevel_Trace:
		return (char*)"TRACE";
	case LogLevel_Debug:
		return (char*)"DEBUG";
	case LogLevel_Info:
		return (char*)"INFO";
	case LogLevel_Warn:
		return (char*)"WARN";
	case LogLevel_Error:
		return (char*)"ERROR";
	case LogLevel_Fatal:
		return (char*)"FATAL";
	case LogLevel_end:
		return (char*)"OFF";
	default:
		return (char*)"UNKNOW";
	}

	return (char*)"UNKNOW";
}

static enum LogLevel_em log_str_2_level(const char* level)
{
	if (!level)
	{
		return LogLevel_Debug;
	}

	if (strcmp("TRACE", level) == 0)
	{
		return LogLevel_Trace;
	}

	if (strcmp("DEBUG", level) == 0)
	{
		return LogLevel_Debug;
	}

	if (strcmp("INFO", level) == 0)
	{
		return LogLevel_Info;
	}

	if (strcmp("WARNING", level) == 0)
	{
		return LogLevel_Warn;
	}

	if (strcmp("ERROR", level) == 0)
	{
		return LogLevel_Error;
	}

	if (strcmp("FATAL", level) == 0)
	{
		return LogLevel_Fatal;
	}

	return LogLevel_Debug;
}


static void set_log_level(enum LogLevel_em newLevel)
{
	if (newLevel < LogLevel_OFF || newLevel >= LogLevel_end)
	{
		LOG_ERROR("please input right log level");
		return;
	}
	w_emCurLogLevel = newLevel;
}

static void set_log_path(const char* path)
{
	if (path)
	{
		memcpy(g_log_root, path, strlen(path));
	}
}

//static void saveFileLog(int log2flieFlag)
static void saveFileLog()
{
	//printf("\naelog %d   %d\n", g_log_to_file, log2flieFlag);
	printf("\naelog %d ", g_log_to_file);
	//if (log2flieFlag)
	if (g_log_to_file)
	{
		time(&timep);
		p = localtime(&timep);
		if (!strlen(g_log_root))
		{
			//生成日志文件的名字
			memcpy(g_log_root, LOG_PATH, strlen(LOG_PATH));
			//assert(ret == 0);
		}
		_mkdir(g_log_root);
		char folder[160] = { 0 };
		sprintf(folder, "%s/%d-%d", g_log_root, 1 + p->tm_mon, p->tm_mday);
		//sprintf(folder, "%s", g_log_root);
		_mkdir(folder);
		////开始运行时间再创建一个目录
		//memset(folder, 0, sizeof(folder));
		//sprintf(folder, "%s//%d-%d/hour_%d", g_log_root, 1 + p->tm_mon, p->tm_mday, p->tm_hour);
		//_mkdir(folder);
		//文件名
		char threadId[256] = { 0 };
		//sprintf(threadId, "%s/%d_%d.log", folder, p->tm_hour, p->tm_min);
		sprintf(threadId, "%s/%2d.log", folder, p->tm_hour);
		//sprintf(threadId, "%s/2.log", folder);
		FILE* fp = fopen(threadId, "a+");
		if (!fp)
		{
			return;
		}
		//fwrite(logbuffer, sizeof(char), strlen(logbuffer), fp);
		//fwrite("\n", sizeof(char), 1, fp);
		fprintf(fp, "%s", logbuffer);
		fclose(fp);
	}
	delete[] logbuffer;
	logbuffer = nullptr;
}

static void printf_log(enum LogLevel_em level, const char* file, const char* function, int line, const char* format, ...)
{
	//printf("\n debug loglevel %d", w_emCurLogLevel);
	if (w_emCurLogLevel & level)
	{
		time(&timep);
		p = localtime(&timep);
		char msg_buf[1024];
		va_list ap;
		va_start(ap, format);
		//sprintf(msg_buf, "[AEC Proc]%02d-%02d %02d:%02d:%02d In %s @%s Line:%u:[%s] ", 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, file, function, line, log_level_2_str(level));
		sprintf(msg_buf, "\n[AEC Proc]%02d-%02d %02d:%02d:%02d @[%s] Line:%u:[%s] ", 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, function, line, log_level_2_str(level));
		//sprintf(msg_buf, "[AEC Proc] @[%s] Line:%u:[%s] ", function, line, log_level_2_str(level));
		//vspritf 存在越界风险，不截断buf --> vsnprintf会按buf长度截断打印
		//__file__在vc编译和gcc编译时输出路径不完全一致，一个是相对路径，一个是绝对路径，可以通过cmake重写函数实现输出相对路径
		vsprintf(msg_buf + strlen(msg_buf), format, ap) - 1;
		va_end(ap);
		//logbuffer = logbuffer + std::string(msg_buf,8);
		int m = 0;
		if (logbuffer != NULL)
		{
			m = strlen(logbuffer);
		}

		int n = strlen(msg_buf);
		char* str = new char[m + n + 1];
		if (logbuffer != NULL)
		{
			sprintf(str, "%s%s", logbuffer, msg_buf);
			delete[] logbuffer;
		}
		else
		{
			sprintf(str, "%s", msg_buf);
		}

		logbuffer = str;

		if (g_log_terminal)
		{
			fprintf(stdout, " %s\n", msg_buf);
		}

	}
	return;
}

static void write_log_2_file(const char* file_name, enum LogLevel_em level, const char* file, const char* function, int line, const char* msg, ...)
{
	FILE* fp = fopen(file_name, "a+");

	if (!fp)
	{
		return;
	}
	if (w_emCurLogLevel & level)
	{
		time(&timep);
		p = localtime(&timep);
		char msg_buf[1024];
		va_list ap;
		va_start(ap, msg);
		sprintf(msg_buf, "%d-%d %d:%d:%d In %s - %s Line %u: [%s] ", 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, file, function, line, log_level_2_str(level));
		vsprintf(msg_buf, msg, ap) - 1;

		if (g_log_to_file)
		{
			fwrite(msg_buf, sizeof(char), strlen(msg_buf), fp);
		}
		fclose(fp);

		if (g_log_terminal)
		{
			fprintf(stdout, msg_buf);
		}
		va_end(ap);
	}
}

#endif // !_MY_LOG_H_