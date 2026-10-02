/**
 * @file sort.c
 * @brief Implementation of sorting algorithms for ls(1).
 */

#include "sort.h"

static const Options *current_opts = NULL;

static int entry_cmp(const void *a, const void *b) {
    const FileEntry *ea = *(const FileEntry * const *)a;
    const FileEntry *eb = *(const FileEntry * const *)b;

    if (!ea && !eb) return 0;
    if (!ea) return 1;
    if (!eb) return -1;

    int res = 0;

    if (current_opts && current_opts->sort_by_size) {
        /* Sort by size, largest file first */
        if (ea->sb.st_size > eb->sb.st_size) {
            res = -1;
        } else if (ea->sb.st_size < eb->sb.st_size) {
            res = 1;
        } else {
            res = strcmp(ea->name, eb->name);
        }
    } else if (current_opts && current_opts->sort_by_time) {
        /* Sort by timestamp, newest file first */
        if (ea->selected_time > eb->selected_time) {
            res = -1;
        } else if (ea->selected_time < eb->selected_time) {
            res = 1;
        } else {
            res = strcmp(ea->name, eb->name);
        }
    } else {
        /* Default: lexicographical order */
        res = strcmp(ea->name, eb->name);
    }

    /* Reverse order if -r is set */
    if (current_opts && current_opts->reverse_sort) {
        res = -res;
    }

    return res;
}

void sort_entries(FileEntry **entries, int count, const Options *opts) {
    if (!entries || count <= 1 || !opts) return;

    /* -f: Output is not sorted */
    if (opts->unsorted) {
        return;
    }

    current_opts = opts;
    qsort(entries, (size_t)count, sizeof(FileEntry *), entry_cmp);
    current_opts = NULL;
}
