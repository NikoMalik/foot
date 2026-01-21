#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "xmalloc.h"
#include "debug.h"

#if defined(MIMALLOC)
#include <mimalloc.h>
#define XMALLOC_PREFIX mi_
#else
#define XMALLOC_PREFIX
#endif

static inline void *
check_alloc(void *ptr) {
    if (unlikely(ptr == NULL)) {
        FATAL_ERROR(__func__, ENOMEM);
    }
    return ptr;
}

void *
xmalloc(size_t size) {
    if (unlikely(size == 0))
        size = 1;
#if defined(MIMALLOC)
    return check_alloc(mi_malloc(size));
#else
    return check_alloc(malloc(size));
#endif
}

void *
xcalloc(size_t nmemb, size_t size) {
    if (unlikely(nmemb == 0 || size == 0)) {
        nmemb = 1;
        size = 1;
    }
#if defined(MIMALLOC)
    return check_alloc(mi_calloc(nmemb, size));
#else
    return check_alloc(calloc(nmemb, size));
#endif
}

void *
xrealloc(void *ptr, size_t size) {
    if (unlikely(size == 0))
        size = 1;
#if defined(MIMALLOC)
    return check_alloc(mi_realloc(ptr, size));
#else
    return check_alloc(realloc(ptr, size));
#endif
}

inline void *
xreallocarray(void *ptr, size_t n, size_t size) {
    if (unlikely(n == 0 || size == 0)) {
        n = 1;
        size = 1;
    }
#if defined(MIMALLOC)
    return check_alloc(mi_reallocarray(ptr, n, size));
#else
    return check_alloc(reallocarray(ptr, n, size));
#endif
}

inline void xfree(void *ptr) {
    if (unlikely(ptr == NULL))
        return;
#if defined(MIMALLOC)
    mi_free(ptr);
#else
    free(ptr);
#endif
}

char *
xstrdup(const char *s) {
#if defined(MIMALLOC)
    return check_alloc(mi_strdup(s));
#else
    return check_alloc(strdup(s));
#endif
}

char *
xstrndup(const char *s, size_t n) {
#if defined(MIMALLOC)
    return check_alloc(mi_strndup(s, n));
#else
    return check_alloc(strndup(s, n));
#endif
}

inline char32_t *
xc32dup(const char32_t *str) {
    return check_alloc(c32dup(str));
}

static VPRINTF(2) int xvasprintf_(char **strp, const char *format, va_list ap) {
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, format, ap2);
    if (unlikely(n < 0)) {
        FATAL_ERROR("vsnprintf", EILSEQ);
    }
    va_end(ap2);
    *strp = xmalloc(n + 1);
    return vsnprintf(*strp, n + 1, format, ap);
}

char *
xvasprintf(const char *format, va_list ap) {
    char *str;
    xvasprintf_(&str, format, ap);
    return str;
}

char *
xasprintf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    char *str = xvasprintf(format, ap);
    va_end(ap);
    return str;
}

inline void xtrim(void) {
#if defined(MIMALLOC)
    mi_collect(true);
#elif defined(__GLIBC__)
    if (!malloc_trim(0))
        LOG_WARN("malloc_trim(0) failed");
#endif
}
