#pragma once

#include <stdint.h> //uint32_t ?
#include <string>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
namespace wuyiServer {

// 事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
private:
    // 文件名
    const char* m_file = nullptr;
    // 行号
    int32_t m_line = 0;
    // 程序启动累计时间
    uint32_t m_elapse = 0;
    // 线程id
    uint32_t m_threadId = 0;
    // 协程id
    uint32_t fiberId = 0;
    // 线程名称
    std::string m_threadName;
    // 线程消息流
    std::stringstream m_strStream;
    // 目标日志器
    std::shared_ptr<Logger> m_logger;
    // 日志级别
    LogLevel::level m_level;
};

// 日志级别
class LogLevel {
public:
    enum level {
        DEBUG = 1,
        INFO,
        WARN,
        ERROR,
        FATAL
    };
};

/*
日志格式器，执行日志格式化，负责日志格式的初始化。
解析日志格式，将用户自定义的日志格式，解析为对应的FormatItem。
日志格式举例：%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
格式解析：
%d{%Y-%m-%d %H:%M:%S} : %d 标识输出的是时间 {%Y-%m-%d %H:%M:%S}为时间格式，可选 DateTimeFormatItem
%T : Tab[\t]            TabFormatItem
%t : 线程id             ThreadIdFormatItem
%N : 线程名称           ThreadNameFormatItem
%F : 协程id             FiberIdFormatItem
%p : 日志级别           LevelFormatItem       
%c : 日志名称           NameFormatItem
%f : 文件名             FilenameFormatItem
%l : 行号               LineFormatItem
%m : 日志内容           MessageFormatItem
%n : 换行符[\r\n]       NewLineFormatItem

具体日志：
2019-06-17 00:28:45     9368    main    6       [INFO]  [system]   
sylar/tcp_server.cc:64  server bind success: [Socket sock=9 is_connected=0 family=2 type=1 protocol=0 local_address=0.0.0.0:8020]
*/
// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;

    std::string format(std::shared_ptr<Logger> logger, LogLevel::level level, LogEvent::ptr event);
    // 具体日志格式
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        // 将对应日志格式内容写入到 ostream
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::level level, LogEvent::ptr event) = 0;
        
    };
    void init();
    const std::string getPattern() const { return m_pattern;}
private:
    // 日志格式
    std::string m_pattern;
    //通过日志格式解析出来的FormatItem，支持扩展
    std::vector<FormatItem::ptr> m_items;
    
};

// 日志目的地
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;

    // 基类虚析构
    virtual ~LogAppender() {}
    // 日志输出到对应的位置  抽象基类 必须override
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::level level, LogEvent::ptr event) = 0;

    LogLevel::level getLevel() { return m_level;}
    LogFormatter::ptr getFormatter() { return m_formatter;}
    void setLevel(LogLevel::level lev) { m_level = lev;}

protected:
    LogLevel::level m_level = LogLevel::DEBUG;
    
    //日志格式器
    LogFormatter::ptr m_formatter;
};

//日志器
class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;
    logger(const std::string& name = "root");

    //~logger() = default;
    // 指定级别的写日志
    void log(LogLevel::level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
    // 添加appender
    void addAppender(LogAppender::ptr appender);
    // 删除appender
    void delAppender(LogAppender::ptr appender);
    // 清空appener
    void clearAppender();

    const std::string& getName() const { return m_name;}
    LogLevel::level getLevel() const { return m_level;}

private:
    std::string m_name;
    //低于该级别就不输出
    LogLevel::level m_level;
    //appender集合
    std::list<LogAppender::ptr> m_appenders;
    //日志格式
    LogFormatter::ptr m_formatter;
};

// 派生类 输出到控制台
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::level level, LogEvent::ptr event) override;
};

// 派生类 输出到文件
class FileLogAppender : public LogAppender {
public:    
    typedef std::shared_ptr<FileLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::level level, LogEvent::ptr event) override;


    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;

};

    
}


