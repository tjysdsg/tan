#ifndef TAN_INCLUDE_CONFIG_H
#define TAN_INCLUDE_CONFIG_H

#ifndef NDEBUG
#    define DEBUG_ENABLED
#endif

// clang-format off
#define TAN_VERSION_MAJOR                                                      \
    0
#define TAN_VERSION_MINOR                                                      \
    0
#define TAN_VERSION_PATCH                                                      \
    1
// clang-format on

#define MAKE_VERSION(major, minor, patch)                                      \
    constexpr unsigned int TAN_VERSION[3] = {major, minor, patch}

MAKE_VERSION(TAN_VERSION_MAJOR, TAN_VERSION_MINOR, TAN_VERSION_PATCH);

#endif //TAN_INCLUDE_CONFIG_H
