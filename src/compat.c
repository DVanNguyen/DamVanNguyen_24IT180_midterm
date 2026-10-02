/**
 * @file compat.c
 * @brief Cross-platform compatibility implementation.
 */

#include "compat.h"

void compat_get_username(uid_t uid, char *buf, size_t len) {
    if (!buf || len == 0) return;

#ifndef _WIN32
    struct passwd *pwd = getpwuid(uid);
    if (pwd && pwd->pw_name) {
        snprintf(buf, len, "%s", pwd->pw_name);
        return;
    }
#else
    DWORD win_len = (DWORD)len;
    if (GetUserNameA(buf, &win_len)) {
        return;
    }
#endif
    snprintf(buf, len, "%u", (unsigned int)uid);
}

void compat_get_groupname(gid_t gid, char *buf, size_t len) {
    if (!buf || len == 0) return;

#ifndef _WIN32
    struct grp *gr = getgrgid(gid);
    if (gr && gr->gr_name) {
        snprintf(buf, len, "%s", gr->gr_name);
        return;
    }
#endif
    snprintf(buf, len, "%u", (unsigned int)gid);
}

int compat_lstat(const char *path, struct stat *sb) {
#ifndef _WIN32
    return lstat(path, sb);
#else
    return stat(path, sb);
#endif
}

ssize_t compat_readlink(const char *path, char *buf, size_t bufsiz) {
#ifndef _WIN32
    return readlink(path, buf, bufsiz);
#else
    (void)path;
    (void)buf;
    (void)bufsiz;
    errno = EINVAL;
    return -1;
#endif
}

long long compat_get_blocks(const struct stat *sb) {
    if (!sb) return 0;

#if defined(_WIN32)
    /* Windows stat structure does not contain st_blocks */
    long long sz = sb->st_size;
    return (sz + 511) / 512;
#elif defined(__APPLE__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__linux__)
    return (long long)sb->st_blocks;
#else
    return (sb->st_size + 511) / 512;
#endif
}

bool compat_is_stdout_terminal(void) {
#ifdef _WIN32
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}
