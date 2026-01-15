#include "uLogger.hpp"

// Plugin interface
class Plugin
{
    public:
        virtual void initialize_logger(std::shared_ptr<LogBuffer>) = 0;

        virtual void run() = 0;

        virtual ~Plugin() = default;

};

typedef Plugin* (*CreatePluginFunc)();
typedef void (*DestroyPluginFunc)(Plugin*);
