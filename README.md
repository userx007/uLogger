
# 📘 Logging Utility for C++

This header-only C++ logging utility provides a lightweight, flexible, and extensible mechanism for logging messages with various severity levels. It supports both console and file outputs, color-coded messages, and timestamping, making it ideal for debugging and monitoring applications.

---

### ✨ Features

- **Multiple Log Levels**: Supports `VERBOSE`, `DEBUG`, `INFO`, `WARNING`, `ERROR`, `FATAL`, and `FIXED`.
- **Color-coded Console Output**: ANSI color codes for better readability.
- **Timestamped Logs**: Includes precise timestamps with microsecond resolution.
- **Thread-safe Logging**: Uses `std::mutex` to ensure safe concurrent access.
- **File Logging Support**: Optionally logs messages to a timestamped file.
- **Customizable Output**: Toggle colors, date inclusion, and log thresholds.

---
### 🛠️ Macros

### 🧩 Components
#### enum class LogLevel
Defines the severity levels for log messages.

#### LogBuffer
A struct that manages the log message buffer, formatting, and output. Key members include:

A struct that manages the log message buffer, formatting, and output. Key members include:

- buffer: Fixed-size character buffer.
- currentLevel: Current log severity.
- consoleThreshold / fileThreshold: Minimum severity for output.
- logFile: Output file stream.
- logMutex: Ensures thread safety.
- useColors, includeDate, fileLoggingEnabled: Output configuration flags.

### Logging Functions
- append(...): Overloaded to support strings, booleans, integers, floats, pointers, and hexadecimal values.
- print(): Outputs the formatted log message to the console and/or file.
- getTimestamp(): Generates a formatted timestamp string.

### Logger Access
- getLogger(), setLogger(...): Access or replace the global logger instance

### Macros
These macros simplify logging usage:

#### Log Message Construction:

    LOG_STRING("Message");
    LOG_INT(42);
    LOG_BOOL(true);
    LOG_HEX32(0xDEADBEEF);

#### Log Message Output

    LOG_PRINT(LOG_INFO, LOG_STRING("Initialization complete"));

#### Logger Initialization:

    LOG_INIT(LOG_DEBUG /* console severity */ , LOG_WARNING /* file severity */, true /* ENABLE_FILE */, true /* ENABLE_COLORS */, true /* INCLUDE_DATE */);

#### Logger Deinitialization:

    LOG_DEINIT();

📦 Example Usage

    #include "uLogger.h"

    int main() {
        LOG_INIT(LOG_DEBUG, LOG_INFO, true, true, true);

        LOG_PRINT(LOG_INFO, LOG_STRING("Starting application"); LOG_INT(123); LOG_BOOL(true));

        LOG_DEINIT();
        return 0;
    }

#### Output Example
    2025-05-31 19:41:53.123456 INFO Starting application 123 true

📌 Notes
File logs are saved with a timestamped filename like log_20250531_194153.txt.