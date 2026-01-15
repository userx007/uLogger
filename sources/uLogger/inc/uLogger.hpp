#ifndef ULOGGER_H
#define ULOGGER_H

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <mutex>
#include <memory>
#include <atomic>

/**
 * @brief Enumeration for log levels.
 */
enum class LogLevel {
    EC_VERBOSE,        /**< Verbose log level. */
    EC_DEBUG,          /**< Debug log level. */
    EC_INFO,           /**< Info log level. */
    EC_WARNING,        /**< Warning log level. */
    EC_ERROR,          /**< Error log level. */
    EC_FATAL,          /**< Fatal log level. */
    EC_FIXED           /**< Fixed log level. */
};

inline constexpr auto LOG_VERBOSE = LogLevel::EC_VERBOSE;
inline constexpr auto LOG_DEBUG   = LogLevel::EC_DEBUG;
inline constexpr auto LOG_INFO    = LogLevel::EC_INFO;
inline constexpr auto LOG_WARNING = LogLevel::EC_WARNING;
inline constexpr auto LOG_ERROR   = LogLevel::EC_ERROR;
inline constexpr auto LOG_FATAL   = LogLevel::EC_FATAL;
inline constexpr auto LOG_FIXED   = LogLevel::EC_FIXED;

using ConsoleLogLevel = LogLevel;
using FileLogLevel    = LogLevel;

/**
 * @brief Converts a log level to a string.
 */
inline const char* toString(LogLevel level)
{
    switch (level) {
        case LOG_VERBOSE: return "VERBOSE";
        case LOG_DEBUG:   return "  DEBUG";
        case LOG_INFO:    return "   INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR:   return "  ERROR";
        case LOG_FATAL:   return "  FATAL";
        case LOG_FIXED:   return "  FIXED";
        default:          return "UNKNOWN";
    }
}

/**
 * @brief Gets the color code for a log level.
 */
inline const char* getColor(LogLevel level)
{
    switch (level) {
        case LOG_VERBOSE: return "\033[90m"; // Bright Black (Gray)
        case LOG_DEBUG:   return "\033[36m"; // Cyan
        case LOG_INFO:    return "\033[32m"; // Green
        case LOG_WARNING: return "\033[33m"; // Yellow
        case LOG_ERROR:   return "\033[31m"; // Red
        case LOG_FATAL:   return "\033[91m"; // Bright Red
        case LOG_FIXED:   return "\033[97m"; // Bright White
        default:          return "\033[0m";  // Reset
    }
}

/**
 * @brief Configuration for log flushing behavior.
 */
enum class FlushPolicy {
    ALWAYS,           /**< Flush after every log message. */
    ERROR_AND_ABOVE,  /**< Flush only for ERROR, FATAL, and FIXED levels. */
    NEVER             /**< Never auto-flush (manual flush only). */
};

/**
 * @brief Structure for log buffer with improved performance and thread safety.
 */
struct LogBuffer
{
    static constexpr size_t BUFFER_SIZE = 4096;     /**< Increased buffer size. */
    char buffer[BUFFER_SIZE] {};
    size_t size = 0;
    LogLevel currentLevel = LOG_INFO;

    LogLevel consoleThreshold = LOG_VERBOSE;
    LogLevel fileThreshold = LOG_VERBOSE;

    std::ofstream logFile;
    mutable std::mutex logMutex;  // Made mutable for const methods

    bool fileLoggingEnabled = false;
    bool useColors = true;
    bool includeDate = true;
    bool truncated = false;  // Flag to track if message was truncated

    FlushPolicy flushPolicy = FlushPolicy::ERROR_AND_ABOVE;

    // Timestamp caching for performance
    mutable std::string cachedTimestamp;
    mutable std::chrono::system_clock::time_point lastTimestampUpdate;
    mutable std::mutex timestampMutex;

    /**
     * @brief Resets the log buffer.
     */
    void reset()
    {
        size = 0;
        buffer[0] = '\0';
        currentLevel = LOG_INFO;
        truncated = false;
    }

    /**
     * @brief Safely appends to buffer with overflow protection.
     * @return Number of bytes actually written.
     */
    size_t safeAppend(int written)
    {
        if (written > 0) {
            size_t available = BUFFER_SIZE - size - 1;
            size_t toWrite = std::min(static_cast<size_t>(written), available);
            
            if (static_cast<size_t>(written) > available) {
                truncated = true;
            }
            
            size += toWrite;
            return toWrite;
        }
        return 0;
    }

    /**
     * @brief Appends a single character to the log buffer.
     */
    void append(char c)
    {
        if (size >= BUFFER_SIZE - 2) {
            truncated = true;
            return;
        }
        int written = std::snprintf(buffer + size, BUFFER_SIZE - size, "%c ", c);
        safeAppend(written);
    }

    /**
     * @brief Appends a text message to the log buffer.
     */
    void append(const char* text)
    {
        if (nullptr == text || size >= BUFFER_SIZE - 1) {
            if (size >= BUFFER_SIZE - 1) truncated = true;
            return;
        }
        int written = std::snprintf(buffer + size, BUFFER_SIZE - size, "%s ", text);
        safeAppend(written);
    }

    /**
     * @brief Appends a string message to the log buffer.
     */
    void append(const std::string& text)
    {
        if (!text.empty()) {
            append(text.c_str());
        }
    }

    /**
     * @brief Appends a string_view message to the log buffer.
     */
    void append(const std::string_view& text_view)
    {
        if (!text_view.empty()) {
            // Avoid creating temporary string for small views
            if (text_view.size() < BUFFER_SIZE - size - 1) {
                int written = std::snprintf(buffer + size, BUFFER_SIZE - size, "%.*s ", 
                                          static_cast<int>(text_view.size()), text_view.data());
                safeAppend(written);
            } else {
                truncated = true;
            }
        }
    }

    /**
     * @brief Appends a boolean value to the buffer.
     */
    void append(bool value)
    {
        if (size >= BUFFER_SIZE - 10) {
            truncated = true;
            return;
        }
        int written = std::snprintf(buffer + size, BUFFER_SIZE - size, "%s ", value ? "true" : "false");
        safeAppend(written);
    }

    /**
     * @brief Appends an integral value to the log buffer.
     */
    template<typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type
    append(T value)
    {
        if (size >= BUFFER_SIZE - 25) {
            truncated = true;
            return;
        }
        
        constexpr const char* format =
            std::is_same<T, size_t>::value   ? "%zu " :
            std::is_signed<T>::value         ? "%lld " :
                                               "%llu ";
        
        int written = std::snprintf(buffer + size, BUFFER_SIZE - size, format, 
                                   static_cast<long long>(value));
        safeAppend(written);
    }

    /**
     * @brief Appends an integral value in hexadecimal format.
     */
    template<typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type
    appendHex(T value)
    {
        if (size >= BUFFER_SIZE - 25) {
            truncated = true;
            return;
        }
        
        constexpr const char* format =
            std::is_same<T, size_t>::value   ? "0x%zX " :
                                               "0x%llX ";
        
        int written = std::snprintf(buffer + size, BUFFER_SIZE - size, format, 
                                   static_cast<unsigned long long>(value));
        safeAppend(written);
    }

    /**
     * @brief Appends a floating-point value to the log buffer.
     */
    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value>::type
    append(T value)
    {
        if (size >= BUFFER_SIZE - 30) {
            truncated = true;
            return;
        }
        int written = std::snprintf(buffer + size, BUFFER_SIZE - size, "%.8f ", 
                                   static_cast<double>(value));
        safeAppend(written);
    }

    /**
     * @brief Appends a pointer to the log buffer.
     */
    template<typename T>
    typename std::enable_if<std::is_pointer<T>::value>::type
    append(T ptr)
    {
        if (size >= BUFFER_SIZE - 20) {
            truncated = true;
            return;
        }
        int written = std::snprintf(buffer + size, BUFFER_SIZE - size, "%p ", 
                                   static_cast<const void*>(ptr));
        safeAppend(written);
    }

    /**
     * @brief Gets the current timestamp with caching for performance.
     */
    std::string getTimestamp() const
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        
        // Cache timestamp for 1ms to avoid excessive system calls
        {
            std::lock_guard<std::mutex> lock(timestampMutex);
            auto elapsed = duration_cast<milliseconds>(now - lastTimestampUpdate);
            
            if (!cachedTimestamp.empty() && elapsed.count() < 1) {
                return cachedTimestamp;
            }
        }
        
        auto duration = now.time_since_epoch();
        auto micros = duration_cast<microseconds>(duration) % 1'000'000;

        std::time_t t = system_clock::to_time_t(now);
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        std::ostringstream oss;
        if (includeDate) {
            oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        } else {
            oss << std::put_time(&tm, "%H:%M:%S");
        }
        oss << "." << std::setfill('0') << std::setw(6) << micros.count() << " | ";
        
        std::string result = oss.str();
        
        // Update cache
        {
            std::lock_guard<std::mutex> lock(timestampMutex);
            cachedTimestamp = result;
            lastTimestampUpdate = now;
        }
        
        return result;
    }

    /**
     * @brief Determines if file should be flushed based on policy.
     */
    bool shouldFlush() const
    {
        switch (flushPolicy) {
            case FlushPolicy::ALWAYS:
                return true;
            case FlushPolicy::ERROR_AND_ABOVE:
                return currentLevel >= LOG_ERROR;
            case FlushPolicy::NEVER:
                return false;
            default:
                return true;
        }
    }

    /**
     * @brief Internal print without locking (called from locked context).
     */
    void printUnsafe()
    {
        // Early exit if log won't be written anywhere
        if (currentLevel < consoleThreshold && 
            (!fileLoggingEnabled || currentLevel < fileThreshold)) {
            reset();
            return;
        }
        
        std::string timestamp = getTimestamp();
        const char* levelStr = toString(currentLevel);
        
        // Build message once
        std::ostringstream oss;
        oss << timestamp << levelStr << " | " << buffer;
        if (truncated) {
            oss << " [TRUNCATED]";
        }
        oss << "\n";
        
        std::string fullMessage = oss.str();

        // Console output
        if (currentLevel >= consoleThreshold) {
            if (useColors) {
                std::printf("%s%s\033[0m", getColor(currentLevel), fullMessage.c_str());
            } else {
                std::printf("%s", fullMessage.c_str());
            }
            std::fflush(stdout);
        }

        // File output
        if (fileLoggingEnabled && currentLevel >= fileThreshold && logFile.is_open()) {
            logFile << fullMessage;
            if (shouldFlush()) {
                logFile.flush();
            }
        }

        reset();
    }

    /**
     * @brief Prints the log message with improved performance.
     */
    void print()
    {
        std::lock_guard<std::mutex> lock(logMutex);
        printUnsafe();
    }

    /**
     * @brief Manually flush the log file.
     */
    void flush()
    {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile.flush();
        }
    }

    /**
     * @brief Sets the current log level.
     */
    void setLevel(LogLevel level)
    {
        currentLevel = level;
    }

    /**
     * @brief Sets the console log level threshold.
     */
    void setConsoleThreshold(LogLevel level)
    {
        consoleThreshold = level;
    }

    /**
     * @brief Sets the file log level threshold.
     */
    void setFileThreshold(LogLevel level)
    {
        fileThreshold = level;
    }

    /**
     * @brief Sets the flush policy for file logging.
     */
    void setFlushPolicy(FlushPolicy policy)
    {
        std::lock_guard<std::mutex> lock(logMutex);
        flushPolicy = policy;
    }

    /**
     * @brief Enables file logging with optional custom filename.
     */
    void enableFileLogging(const std::string& filename = "")
    {
        std::lock_guard<std::mutex> lock(logMutex);
        
        if (!fileLoggingEnabled) {
            std::string logFilename;
            
            if (filename.empty()) {
                std::ostringstream oss;
                auto now = std::chrono::system_clock::now();
                std::time_t t = std::chrono::system_clock::to_time_t(now);
                std::tm tm;
#ifdef _WIN32
                localtime_s(&tm, &t);
#else
                localtime_r(&t, &tm);
#endif
                oss << "log_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".txt";
                logFilename = oss.str();
            } else {
                logFilename = filename;
            }
            
            logFile.open(logFilename, std::ios::out | std::ios::app);
            fileLoggingEnabled = logFile.is_open();
        }
    }

    /**
     * @brief Disables file logging.
     */
    void disableFileLogging()
    {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile.flush();
            logFile.close();
        }
        fileLoggingEnabled = false;
    }

    /**
     * @brief Destructor ensures file is properly closed.
     */
    ~LogBuffer()
    {
        disableFileLogging();
    }
};

/**
 * @brief Global instance.
 */
inline std::shared_ptr<LogBuffer> log_local = std::make_shared<LogBuffer>();

/**
 * @brief Gets the global log buffer instance.
 */
inline std::shared_ptr<LogBuffer> getLogger()
{
    return log_local;
}

/**
 * @brief Sets the global log buffer instance.
 */
inline void setLogger(std::shared_ptr<LogBuffer> logger)
{
    log_local = logger;
}

/** --------------------------------  Macros ----------------------------------------------- */

#define LOG_STRING(TEXT)   log_local->append(TEXT);
#define LOG_PTR(PTR)       log_local->append(PTR);
#define LOG_BOOL(V)        log_local->append(static_cast<bool>(V));
#define LOG_CHAR(C)        log_local->append(static_cast<char>(C));
#define LOG_UINT8(V)       log_local->append(static_cast<uint8_t>(V));
#define LOG_UINT16(V)      log_local->append(static_cast<uint16_t>(V));
#define LOG_UINT32(V)      log_local->append(static_cast<uint32_t>(V));
#define LOG_UINT64(V)      log_local->append(static_cast<uint64_t>(V));
#define LOG_SIZET(V)       log_local->append(static_cast<size_t>(V));
#define LOG_INT8(V)        log_local->append(static_cast<int8_t>(V));
#define LOG_INT16(V)       log_local->append(static_cast<int16_t>(V));
#define LOG_INT32(V)       log_local->append(static_cast<int32_t>(V));
#define LOG_INT64(V)       log_local->append(static_cast<int64_t>(V));
#define LOG_INT(V)         log_local->append(static_cast<int>(V));
#define LOG_FLOAT(V)       log_local->append(static_cast<float>(V));
#define LOG_DOUBLE(V)      log_local->append(static_cast<double>(V));
#define LOG_HEX8(V)        log_local->appendHex(static_cast<uint8_t>(V));
#define LOG_HEX16(V)       log_local->appendHex(static_cast<uint16_t>(V));
#define LOG_HEX32(V)       log_local->appendHex(static_cast<uint32_t>(V));
#define LOG_HEX64(V)       log_local->appendHex(static_cast<uint64_t>(V));
#define LOG_HEXSIZET(V)    log_local->appendHex(static_cast<size_t>(V));

/**
 * @brief Thread-safe logging macro with automatic mutex protection.
 */
#define LOG_PRINT(SEVERITY, ...)  \
    do { \
        std::lock_guard<std::mutex> _log_guard(log_local->logMutex); \
        log_local->setLevel(SEVERITY); \
        __VA_ARGS__ \
        log_local->printUnsafe(); \
    } while(0)

/**
 * @brief Enhanced logger initialization with flush policy.
 */
#define LOG_INIT(CONSOLE_LEVEL, FILE_LEVEL, ENABLE_FILE, ENABLE_COLORS, INCLUDE_DATE) \
    do { \
        log_local->setConsoleThreshold(CONSOLE_LEVEL); \
        log_local->setFileThreshold(FILE_LEVEL); \
        log_local->useColors = ENABLE_COLORS; \
        log_local->includeDate = INCLUDE_DATE; \
        if (ENABLE_FILE) { \
            log_local->enableFileLogging(); \
        } else { \
            log_local->disableFileLogging(); \
        } \
    } while(0)

/**
 * @brief Extended initialization with flush policy control.
 */
#define LOG_INIT_EXT(CONSOLE_LEVEL, FILE_LEVEL, ENABLE_FILE, ENABLE_COLORS, INCLUDE_DATE, FLUSH_POLICY) \
    do { \
        LOG_INIT(CONSOLE_LEVEL, FILE_LEVEL, ENABLE_FILE, ENABLE_COLORS, INCLUDE_DATE); \
        log_local->setFlushPolicy(FLUSH_POLICY); \
    } while(0)

/**
 * @brief Deinitialize logger and flush any pending data.
 */
#define LOG_DEINIT() \
    do { \
        log_local->flush(); \
        log_local->disableFileLogging(); \
    } while(0)

/**
 * @brief Manual flush macro.
 */
#define LOG_FLUSH() \
    log_local->flush()

#endif // ULOGGER_H