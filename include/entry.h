/**
 * @file entry.h
 * @brief Representation and metadata extraction of directory entries.
 */

#ifndef ENTRY_H
#define ENTRY_H

#include "compat.h"
#include "options.h"

#ifndef major
    #define major(dev) ((unsigned int)(((dev) >> 8) & 0xff))
#endif
#ifndef minor
    #define minor(dev) ((unsigned int)((dev) & 0xff))
#endif

typedef struct FileEntry {
    char *name;             /* Entry name for display */
    char *full_path;        /* Complete filesystem path */
    char *link_target;      /* Target of symbolic link if applicable */
    struct stat sb;         /* Stat structure */
    bool stat_ok;           /* True if stat succeeded */
    int error_number;       /* errno if stat failed */

    /* Pre-formatted strings for alignment calculations */
    char mode_str[12];      /* "drwxr-xr-x" */
    char owner_str[64];     /* username or numeric UID */
    char group_str[64];     /* groupname or numeric GID */
    char size_str[32];      /* byte count, humanized size, or "major, minor" */
    char time_str[32];      /* e.g. "Oct 27 16:04" */
    char blocks_str[32];    /* formatted block count */
    char inode_str[32];     /* formatted inode */
    char classifier;        /* classifier character ('/', '*', etc.) for -F */

    long long display_blocks; /* Block count for total summation */
    time_t selected_time;     /* Timestamp selected by -c / -u / default */
} FileEntry;

/**
 * @brief Create and populate a FileEntry from path.
 * @param dir Directory path (can be NULL or empty for root/standalone paths).
 * @param name File/Directory name.
 * @param opts Program options.
 * @param follow_symlink Whether to dereference symlink with stat instead of lstat.
 * @return Allocated FileEntry or NULL on allocation error.
 */
FileEntry *entry_create(const char *dir, const char *name, const Options *opts, bool follow_symlink);

/**
 * @brief Free a FileEntry and its allocated fields.
 * @param entry FileEntry to free.
 */
void entry_free(FileEntry *entry);

#endif /* ENTRY_H */
