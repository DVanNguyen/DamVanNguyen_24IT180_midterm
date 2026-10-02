/**
 * @file utils.c
 * @brief Implementation of helper utility functions.
 */

#include "utils.h"

char *utils_path_join(const char *dir, const char *file) {
    if (!file) return NULL;
    if (!dir || *dir == '\0') {
        return strdup(file);
    }

    size_t dlen = strlen(dir);
    size_t flen = strlen(file);
    bool needs_sep = (dlen > 0 && dir[dlen - 1] != '/' && dir[dlen - 1] != '\\');

    char *result = malloc(dlen + flen + (needs_sep ? 2 : 1));
    if (!result) return NULL;

    if (needs_sep) {
        snprintf(result, dlen + flen + 2, "%s/%s", dir, file);
    } else {
        snprintf(result, dlen + flen + 1, "%s%s", dir, file);
    }

    return result;
}

void utils_humanize_size(long long bytes, char *buf, size_t buflen) {
    if (!buf || buflen == 0) return;

    if (bytes < 0) {
        snprintf(buf, buflen, "0B");
        return;
    }

    if (bytes < 1024) {
        snprintf(buf, buflen, "%lldB", bytes);
        return;
    }

    const char units[] = {'K', 'M', 'G', 'T', 'P'};
    double val = (double)bytes / 1024.0;
    int u = 0;

    while (val >= 1024.0 && u < 4) {
        val /= 1024.0;
        u++;
    }

    if (val < 10.0) {
        snprintf(buf, buflen, "%.1f%c", val, units[u]);
    } else {
        snprintf(buf, buflen, "%.0f%c", val, units[u]);
    }
}

void utils_format_time(time_t t, char *buf, size_t buflen) {
    if (!buf || buflen == 0) return;

    struct tm *tm_info = localtime(&t);
    if (!tm_info) {
        snprintf(buf, buflen, "------------");
        return;
    }

    time_t now = time(NULL);
    double diff = difftime(now, t);

    /* Display "Mmm dd hh:mm" if within 6 months (approx 180 days), else "Mmm dd  YYYY" */
    if (diff >= 0 && diff < (180.0 * 86400.0)) {
        strftime(buf, buflen, "%b %e %H:%M", tm_info);
    } else {
        strftime(buf, buflen, "%b %e  %Y", tm_info);
    }
}

void utils_format_mode(mode_t mode, char *buf) {
    if (!buf) return;

    /* Entry type character */
    if (S_ISDIR(mode)) {
        buf[0] = 'd';
    } else if (S_ISLNK(mode)) {
        buf[0] = 'l';
    } else if (S_ISCHR(mode)) {
        buf[0] = 'c';
    } else if (S_ISBLK(mode)) {
        buf[0] = 'b';
    } else if (S_ISFIFO(mode)) {
        buf[0] = 'p';
    } else if (S_ISSOCK(mode)) {
        buf[0] = 's';
    } else {
        buf[0] = '-';
    }

    /* Owner permissions */
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID) {
        buf[3] = (mode & S_IXUSR) ? 's' : 'S';
    } else {
        buf[3] = (mode & S_IXUSR) ? 'x' : '-';
    }

    /* Group permissions */
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID) {
        buf[6] = (mode & S_IXGRP) ? 's' : 'S';
    } else {
        buf[6] = (mode & S_IXGRP) ? 'x' : '-';
    }

    /* Other permissions */
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX) {
        buf[9] = (mode & S_IXOTH) ? 't' : 'T';
    } else {
        buf[9] = (mode & S_IXOTH) ? 'x' : '-';
    }

    buf[10] = '\0';
}

char utils_get_classifier(mode_t mode) {
    if (S_ISDIR(mode)) return '/';
    if (S_ISLNK(mode)) return '@';
    if (S_ISSOCK(mode)) return '=';
    if (S_ISFIFO(mode)) return '|';
    if ((mode & 0111) && S_ISREG(mode)) return '*';
    return '\0';
}

void utils_sanitize_name(const char *name, char *buf, size_t buflen, bool force_question) {
    if (!name || !buf || buflen == 0) return;

    size_t i = 0;
    for (; name[i] != '\0' && (i + 1) < buflen; i++) {
        unsigned char c = (unsigned char)name[i];
        if (force_question && !isprint(c)) {
            buf[i] = '?';
        } else {
            buf[i] = (char)c;
        }
    }
    buf[i] = '\0';
}
