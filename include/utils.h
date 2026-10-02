/**
 * @file utils.h
 * @brief Helper utility functions for path handling, formatting, and permissions.
 */

#ifndef UTILS_H
#define UTILS_H

#include "compat.h"

/**
 * @brief Safely join two path components with a directory separator.
 * @param dir Directory part.
 * @param file File/Subdirectory part.
 * @return Dynamically allocated string (caller must free), or NULL on failure.
 */
char *utils_path_join(const char *dir, const char *file);

/**
 * @brief Format size in bytes to human-readable string (for -h).
 * @param bytes Number of bytes.
 * @param buf Output buffer.
 * @param buflen Buffer size.
 */
void utils_humanize_size(long long bytes, char *buf, size_t buflen);

/**
 * @brief Format a UNIX timestamp for long listing.
 * @param t File timestamp.
 * @param buf Output buffer.
 * @param buflen Buffer size.
 */
void utils_format_time(time_t t, char *buf, size_t buflen);

/**
 * @brief Format file permissions into a 10-character string (e.g. "drwxr-xr-x").
 * @param mode st_mode from stat struct.
 * @param buf Output buffer of at least 11 characters.
 */
void utils_format_mode(mode_t mode, char *buf);

/**
 * @brief Get classifier character for -F flag.
 * @param mode st_mode from stat struct.
 * @return Classifier character ('/', '*', '@', '=', '|', or '\0').
 */
char utils_get_classifier(mode_t mode);

/**
 * @brief Sanitize string for display according to -q / -w flags.
 * @param name Original filename string.
 * @param buf Output buffer.
 * @param buflen Buffer capacity.
 * @param force_question If true, replace non-printable chars with '?'.
 */
void utils_sanitize_name(const char *name, char *buf, size_t buflen, bool force_question);

#endif /* UTILS_H */
