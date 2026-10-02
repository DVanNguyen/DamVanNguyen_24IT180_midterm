/**
 * @file entry.c
 * @brief Implementation of directory entry creation and metadata extraction.
 */

#include "entry.h"
#include "utils.h"

FileEntry *entry_create(const char *dir, const char *name, const Options *opts, bool follow_symlink) {
    if (!name) return NULL;

    FileEntry *entry = calloc(1, sizeof(FileEntry));
    if (!entry) return NULL;

    entry->name = strdup(name);
    entry->full_path = (dir && *dir) ? utils_path_join(dir, name) : strdup(name);

    if (!entry->name || !entry->full_path) {
        entry_free(entry);
        return NULL;
    }

    int res;
    if (follow_symlink) {
        res = stat(entry->full_path, &entry->sb);
    } else {
        res = compat_lstat(entry->full_path, &entry->sb);
    }

    if (res != 0) {
        entry->stat_ok = false;
        entry->error_number = errno;
        return entry;
    }

    entry->stat_ok = true;

    /* Determine selected timestamp */
    if (opts->time_field == TIME_STATUS_CHANGE) {
        entry->selected_time = entry->sb.st_ctime;
    } else if (opts->time_field == TIME_ACCESS) {
        entry->selected_time = entry->sb.st_atime;
    } else {
        entry->selected_time = entry->sb.st_mtime;
    }

    /* Read symbolic link target if applicable */
    if (S_ISLNK(entry->sb.st_mode)) {
        char target_buf[1024];
        ssize_t len = compat_readlink(entry->full_path, target_buf, sizeof(target_buf) - 1);
        if (len != -1) {
            target_buf[len] = '\0';
            entry->link_target = strdup(target_buf);
        }
    }

    /* Format mode string (10 characters, e.g. "drwxr-xr-x") */
    utils_format_mode(entry->sb.st_mode, entry->mode_str);

    /* Format owner string */
    if (opts->numeric_ids) {
        snprintf(entry->owner_str, sizeof(entry->owner_str), "%u", (unsigned int)entry->sb.st_uid);
    } else {
        compat_get_username(entry->sb.st_uid, entry->owner_str, sizeof(entry->owner_str));
    }

    /* Format group string */
    if (opts->numeric_ids) {
        snprintf(entry->group_str, sizeof(entry->group_str), "%u", (unsigned int)entry->sb.st_gid);
    } else {
        compat_get_groupname(entry->sb.st_gid, entry->group_str, sizeof(entry->group_str));
    }

    /* Format size string */
    if (S_ISCHR(entry->sb.st_mode) || S_ISBLK(entry->sb.st_mode)) {
        snprintf(entry->size_str, sizeof(entry->size_str), "%u, %u",
                 major(entry->sb.st_rdev), minor(entry->sb.st_rdev));
    } else if (opts->human_readable) {
        utils_humanize_size((long long)entry->sb.st_size, entry->size_str, sizeof(entry->size_str));
    } else {
        snprintf(entry->size_str, sizeof(entry->size_str), "%lld", (long long)entry->sb.st_size);
    }

    /* Format inode string */
    snprintf(entry->inode_str, sizeof(entry->inode_str), "%llu", (unsigned long long)entry->sb.st_ino);

    /* Compute and format blocks */
    long long raw_512_blocks = compat_get_blocks(&entry->sb);
    long long raw_bytes = raw_512_blocks * 512;

    if (opts->human_readable) {
        utils_humanize_size(raw_bytes, entry->blocks_str, sizeof(entry->blocks_str));
        entry->display_blocks = raw_512_blocks;
    } else if (opts->kilobytes) {
        long long kb = (raw_bytes + 1023) / 1024;
        snprintf(entry->blocks_str, sizeof(entry->blocks_str), "%lld", kb);
        entry->display_blocks = kb;
    } else {
        long bsize = (opts->blocksize > 0) ? opts->blocksize : 512;
        long long blks = (raw_bytes + bsize - 1) / bsize;
        snprintf(entry->blocks_str, sizeof(entry->blocks_str), "%lld", blks);
        entry->display_blocks = blks;
    }

    /* Format time string */
    utils_format_time(entry->selected_time, entry->time_str, sizeof(entry->time_str));

    /* Classifier for -F */
    if (opts->classify) {
        entry->classifier = utils_get_classifier(entry->sb.st_mode);
    } else {
        entry->classifier = '\0';
    }

    return entry;
}

void entry_free(FileEntry *entry) {
    if (!entry) return;
    if (entry->name) free(entry->name);
    if (entry->full_path) free(entry->full_path);
    if (entry->link_target) free(entry->link_target);
    free(entry);
}
