#ifndef MACROS_H
#define MACROS_H

// Useful macros
#define CHECK_PTR_RET(p) \
    if(!p)               \
    return
#define CHECK_PTR_RET_FALSE(p) \
    if(!p)                     \
    return false
#define CHECK_PTR_RET_NULL(p) \
    if(!p)                    \
    return NULL
#define CHECK_PTR_RET_EMPTY_STRING(p) \
    if(!p)                            \
    return wxEmptyString

#define CHECK_COND_RET(p) \
    if(!(p))              \
    return
#define CHECK_COND_RET_FALSE(p) \
    if(!(p))                    \
    return false
#define CHECK_COND_RET_NULL(p) \
    if(!(p))                   \
    return NULL
#define CHECK_COND_RET_EMPTY_STRING(p) \
    if(!(p))                           \
    return wxEmptyString

#define CHECK_ITEM_RET(item) \
    if(!item.IsOk())         \
    return
#define CHECK_ITEM_RET_FALSE(item) \
    if(!item.IsOk())               \
    return false
#define CHECK_ITEM_RET_NULL(item) \
    if(!item.IsOk())              \
    return NULL
#define CHECK_ITEM_RET_EMPTY_STRING(item) \
    if(!item.IsOk())                      \
    return wxEmptyString

#endif // MACROS_H
