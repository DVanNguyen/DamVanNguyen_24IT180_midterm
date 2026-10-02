/**
 * @file compat.h
 * @brief Cross-platform compatibility definitions for POSIX and Windows (MinGW).
 * 
 * Provides unified interfaces for system calls, user/group databases,
 * symbolic links, and file metadata extraction.
 */

#ifndef COMPAT_H
#define COMPAT_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
    #include <io.h>
    #include <windows.h>
    typedef unsigned int uid_t;
    typedef unsigned int gid_t;
    #define isatty _isatty
    #define fileno _fileno
    #define STDOUT_FILENO 1
    #define PATH_SEPARATOR '\\'
    #define PATH_SEPARATOR_STR "\\"

    /* File mode flags if not defined by MinGW */
    #ifndef S_ISLNK
        #define S_ISLNK(m) 0
    #endif
    #ifndef S_ISSOCK
        #define S_ISSOCK(m) 0
    #endif
    #ifndef S_ISFIFO
        #define S_ISFIFO(m) 0
    #endif
    #ifndef S_ISUID
        #define S_ISUID 0004000
    #endif
    #ifndef S_ISGID
        #define S_ISGID 0002000
    #endif
    #ifndef S_ISVTX
        #define S_ISVTX 0001000
    #endif
    #ifndef S_IRGRP
        #define S_IRGRP 0000040
    #endif
    #ifndef S_IWGRP
        #define S_IWGRP 0000020
    #endif
    #ifndef S_IXGRP
        #define S_IXGRP 0000010
    #endif
    #ifndef S_IROTH
        #define S_IROTH 0000004
    #endif
    #ifndef S_IWOTH
        #define S_IWOTH 0000002
    #endif
    #ifndef S_IXOTH
        #define S_IXOTH 0000001
    #endif
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <grp.h>
    #define PATH_SEPARATOR '/'
    #define PATH_SEPARATOR_STR "/"
#endif

/**
 * @brief Get username for a given UID.
 * @param uid User ID.
 * @param buf Buffer to store username.
 * @param len Size of buffer.
 */
void compat_get_username(uid_t uid, char *buf, size_t len);

/**
 * @brief Get group name for a given GID.
 * @param gid Group ID.
 * @param buf Buffer to store group name.
 * @param len Size of buffer.
 */
void compat_get_groupname(gid_t gid, char *buf, size_t len);

/**
 * @brief Perform lstat on path (does not follow symlinks).
 * @param path File path.
 * @param sb Pointer to stat structure.
 * @return 0 on success, -1 on error.
 */
int compat_lstat(const char *path, struct stat *sb);

/**
 * @brief Read target of symbolic link.
 * @param path Symlink path.
 * @param buf Buffer to store target path.
 * @param bufsiz Buffer capacity.
 * @return Number of bytes read, or -1 on error.
 */
ssize_t compat_readlink(const char *path, char *buf, size_t bufsiz);

/**
 * @brief Get 512-byte block count from stat struct.
 * @param sb Pointer to stat structure.
 * @return Number of 512-byte blocks.
 */
long long compat_get_blocks(const struct stat *sb);

/**
 * @brief Check if standard output is a terminal.
 * @return true if terminal, false otherwise.
 */
bool compat_is_stdout_terminal(void);

#endif /* COMPAT_H */
