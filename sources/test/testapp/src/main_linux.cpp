#include "uLogger.hpp"
#include "IPlugin.hpp"

#include <dlfcn.h>
#include <memory>

#define LT_HDR     "APP     :"
#define LOG_HDR    LOG_STRING(LT_HDR)


int main()
{
    // Initialize logger
    LOG_INIT(LOG_VERBOSE/*ConsoleLogLevel*/, LOG_VERBOSE /*FileLogLevel*/, true /*ENABLE_FILE*/, true /*ENABLE_COLORS*/, true /*INCLUDE_DATE*/ );

    // Log something from the main app
    LOG_PRINT(LOG_VERBOSE, LOG_HDR; LOG_STRING("Verbose message from main app"));
    LOG_PRINT(LOG_DEBUG,   LOG_HDR; LOG_STRING("Debug message from main app"));
    LOG_PRINT(LOG_INFO,    LOG_HDR; LOG_STRING("Info message from main app"));
    LOG_PRINT(LOG_WARNING, LOG_HDR; LOG_STRING("Warning message from main app"));
    LOG_PRINT(LOG_ERROR,   LOG_HDR; LOG_STRING("Error message from main app"));
    LOG_PRINT(LOG_FATAL,   LOG_HDR; LOG_STRING("Fatal message from main app"));
    LOG_PRINT(LOG_FIXED,   LOG_HDR; LOG_STRING("Fixed message from main app"));

    // Load plugin
    void* handle = dlopen("./libplugin.so", RTLD_LAZY);
    if (!handle) {
        LOG_PRINT(LOG_ERROR, LOG_STRING("Failed to load plugin"));
        return 1;
    }

    // Get the create_plugin function from the plugin
    CreatePluginFunc createPlugin = (CreatePluginFunc)dlsym(handle, "create_plugin");
    if (!createPlugin) {
        LOG_PRINT(LOG_ERROR, LOG_HDR; LOG_STRING("Failed to get create_plugin function"));
        dlclose(handle);
        return 1;
    }

    // Get the destroy_plugin function from the plugin
    DestroyPluginFunc destroyPlugin = (DestroyPluginFunc)dlsym(handle, "destroy_plugin");
    if (!destroyPlugin) {
        LOG_PRINT(LOG_ERROR, LOG_HDR; LOG_STRING("Failed to get destroy_plugin function"));
        dlclose(handle);
        return 1;
    }

    // Create plugin instance
    Plugin* plugin = createPlugin();
    if (plugin) {
        plugin->initialize_logger(getLogger()); // Share the logger with the plugin
        plugin->run();
        destroyPlugin(plugin);
    }

    dlclose(handle);

    LOG_DEINIT();

    return 0;

}
