#include "uLogger.hpp"
#include "IPlugin.hpp"

#include <windows.h>
#include <iostream>

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

    // Load plugin DLL
    HMODULE handle = LoadLibraryEx("./libplugin.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
    if (!handle) {
        LOG_PRINT(LOG_ERROR, LOG_STRING("Failed to load plugin"));
        return 1;
    }

    // Get create_plugin function
    CreatePluginFunc createPlugin = reinterpret_cast<CreatePluginFunc>(GetProcAddress(handle, "create_plugin"));

    if (!createPlugin) {
        LOG_PRINT(LOG_ERROR, LOG_HDR; LOG_STRING("Failed to get create_plugin function"));
        FreeLibrary(handle);
        return 1;
    }

    // Get destroy_plugin function
    DestroyPluginFunc destroyPlugin = reinterpret_cast<DestroyPluginFunc>(GetProcAddress(handle, "destroy_plugin"));

    if (!destroyPlugin) {
        LOG_PRINT(LOG_ERROR, LOG_HDR; LOG_STRING("Failed to get destroy_plugin function"));
        FreeLibrary(handle);
        return 1;
    }

    // Create and run plugin
    Plugin* plugin = createPlugin();
    if (plugin) {
        plugin->initialize_logger(getLogger());
        plugin->run();
        destroyPlugin(plugin);
    }

    FreeLibrary(handle);

    LOG_DEINIT();

    return 0;

}
