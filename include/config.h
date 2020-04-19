#ifndef TAN_INCLUDE_CONFIG_H
#define TAN_INCLUDE_CONFIG_H

// clang-format off
#define TAN_VERSION_MAJOR 0
#define TAN_VERSION_MINOR 0
#define TAN_VERSION_PATCH 1
// clang-format on

#define MAKE_VERSION(major, minor, patch)                                      \
    constexpr unsigned int TAN_VERSION[3] = {major, minor, patch}
MAKE_VERSION(TAN_VERSION_MAJOR, TAN_VERSION_MINOR, TAN_VERSION_PATCH);

#define TAN_PTR_SIZE_BITS 64
#define MAX_N_FUNCTION_CALLS 256

#endif //TAN_INCLUDE_CONFIG_H
