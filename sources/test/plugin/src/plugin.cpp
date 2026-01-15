
#include "uLogger.hpp"
#include "IPlugin.hpp"

#include <memory>

#define LT_HDR     "PLUGIN  :"
#define LOG_HDR    LOG_STRING(LT_HDR)


class MyPlugin : public Plugin
{
    public:

        void initialize_logger(std::shared_ptr<LogBuffer> logger) override
        {
            setLogger(logger); // Set the shared logger
        }

        void run() override
        {
            LOG_PRINT(LOG_VERBOSE, LOG_HDR; LOG_STRING("Verbose message from plugin"));
            LOG_PRINT(LOG_DEBUG,   LOG_HDR; LOG_STRING("Debug message from plugin"));
            LOG_PRINT(LOG_INFO,    LOG_HDR; LOG_STRING("Info message from plugin"));
            LOG_PRINT(LOG_WARNING, LOG_HDR; LOG_STRING("Warning message from plugin"));
            LOG_PRINT(LOG_ERROR,   LOG_HDR; LOG_STRING("Error message from plugin"));
            LOG_PRINT(LOG_FATAL,   LOG_HDR; LOG_STRING("Fatal message from plugin"));
            LOG_PRINT(LOG_FIXED,   LOG_HDR; LOG_STRING("Fixed message from plugin"));
        }
};

extern "C" Plugin* create_plugin()
{
    return new MyPlugin();
}

extern "C" void destroy_plugin(Plugin* plugin)
{
    delete plugin;
}
