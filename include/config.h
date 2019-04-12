#ifndef __TAN_INCLUDE_CONFIG_H__
#define __TAN_INCLUDE_CONFIG_H__

#ifndef NDEBUG
#    define DEBUG_ENABLED
#endif

#define TAN_VERSION_MAJOR 0
#define TAN_VERSION_MINOR 0
#define TAN_VERSION_PATCH 1

#define MAKE_VERSION(major, minor, patch)                              \
    constexpr unsigned int TAN_VERSION[3] = {major, minor, patch}

MAKE_VERSION(TAN_VERSION_MAJOR, TAN_VERSION_MINOR, TAN_VERSION_PATCH);

#endif // __TAN_INCLUDE_CONFIG_H__
