/**
 * @file format.c
 * @brief Output formatting routines for displaying files and directories.
 */

#include "format.h"
#include "utils.h"

void format_print_total(FileEntry **entries, int count, const Options *opts) {
    if (!opts) return;
    if (!opts->long_format && !(opts->display_blocks && opts->is_terminal)) {
        return;
    }

    long long total_blocks = 0;
    for (int i = 0; i < count; i++) {
        if (entries[i] && entries[i]->stat_ok) {
            total_blocks += entries[i]->display_blocks;
        }
    }

    if (opts->human_readable) {
        char hbuf[32];
        utils_humanize_size(total_blocks * 512, hbuf, sizeof(hbuf));
        printf("total %s\n", hbuf);
    } else {
        printf("total %lld\n", total_blocks);
    }
}

void format_print_entries(FileEntry **entries, int count, const Options *opts) {
    if (!entries || count <= 0 || !opts) return;

    int max_inode_w = 0;
    int max_blocks_w = 0;
    int max_links_w = 0;
    int max_owner_w = 0;
    int max_group_w = 0;
    int max_size_w = 0;

    /* Measure maximum column widths */
    for (int i = 0; i < count; i++) {
        FileEntry *e = entries[i];
        if (!e) continue;

        if (opts->inode) {
            int len = (int)strlen(e->inode_str);
            if (len > max_inode_w) max_inode_w = len;
        }

        if (opts->display_blocks) {
            int len = (int)strlen(e->blocks_str);
            if (len > max_blocks_w) max_blocks_w = len;
        }

        if (opts->long_format && e->stat_ok) {
            char link_buf[32];
            snprintf(link_buf, sizeof(link_buf), "%u", (unsigned int)e->sb.st_nlink);
            int llen = (int)strlen(link_buf);
            if (llen > max_links_w) max_links_w = llen;

            int olen = (int)strlen(e->owner_str);
            if (olen > max_owner_w) max_owner_w = olen;

            int glen = (int)strlen(e->group_str);
            if (glen > max_group_w) max_group_w = glen;

            int slen = (int)strlen(e->size_str);
            if (slen > max_size_w) max_size_w = slen;
        }
    }

    /* Print each entry */
    char safe_name[1024];
    for (int i = 0; i < count; i++) {
        FileEntry *e = entries[i];
        if (!e) continue;

        utils_sanitize_name(e->name, safe_name, sizeof(safe_name), opts->non_printable_q);

        /* Print Inode (-i) */
        if (opts->inode) {
            printf("%*s ", max_inode_w, e->inode_str);
        }

        /* Print Blocks (-s) */
        if (opts->display_blocks) {
            printf("%*s ", max_blocks_w, e->blocks_str);
        }

        if (opts->long_format) {
            if (e->stat_ok) {
                printf("%s %*u %-*s  %-*s  %*s %s %s",
                       e->mode_str,
                       max_links_w, (unsigned int)e->sb.st_nlink,
                       max_owner_w, e->owner_str,
                       max_group_w, e->group_str,
                       max_size_w, e->size_str,
                       e->time_str,
                       safe_name);
            } else {
                printf("?????????? %*s %-*s  %-*s  %*s ------------ %s",
                       max_links_w, "?",
                       max_owner_w, "?",
                       max_group_w, "?",
                       max_size_w, "?",
                       safe_name);
            }

            /* Classifier (-F) */
            if (e->classifier != '\0') {
                putchar(e->classifier);
            }

            /* Symbolic link target */
            if (e->link_target) {
                char safe_target[1024];
                utils_sanitize_name(e->link_target, safe_target, sizeof(safe_target), opts->non_printable_q);
                printf(" -> %s", safe_target);
            }

            putchar('\n');
        } else {
            /* Standard one-entry-per-line format */
            printf("%s", safe_name);
            if (e->classifier != '\0') {
                putchar(e->classifier);
            }
            putchar('\n');
        }
    }
}
