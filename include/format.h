/**
 * @file format.h
 * @brief Output formatting routines for ls(1).
 */

#ifndef FORMAT_H
#define FORMAT_H

#include "entry.h"
#include "options.h"

/**
 * @brief Print total block count for a directory listing.
 * @param entries Array of FileEntry pointers.
 * @param count Number of entries.
 * @param opts Program options.
 */
void format_print_total(FileEntry **entries, int count, const Options *opts);

/**
 * @brief Print a list of file entries according to specified options.
 * @param entries Array of FileEntry pointers.
 * @param count Number of entries.
 * @param opts Program options.
 */
void format_print_entries(FileEntry **entries, int count, const Options *opts);

#endif /* FORMAT_H */
