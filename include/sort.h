/**
 * @file sort.h
 * @brief Sorting routines for directory entries.
 */

#ifndef SORT_H
#define SORT_H

#include "entry.h"
#include "options.h"

/**
 * @brief Sort an array of FileEntry pointers according to the given options.
 * @param entries Array of FileEntry pointers.
 * @param count Number of entries.
 * @param opts Sorting options (-t, -S, -r, -f, etc.).
 */
void sort_entries(FileEntry **entries, int count, const Options *opts);

#endif /* SORT_H */
