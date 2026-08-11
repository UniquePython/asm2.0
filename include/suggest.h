#ifndef SUGGEST_H_
#define SUGGEST_H_

#include "types.h"

/*
 * Finds the candidate in `candidates` (an array of `n` NUL-terminated
 * strings) closest to `input` by DamerauLevenshtein distance, provided
 * that distance is <= maxDistance. Returns NULL if no candidate
 * qualifies.
 *
 * On ties (multiple candidates at the same minimal distance), the
 * first one encountered in `candidates` (in array order) wins.
 *
 * The returned pointer aliases an entry of `candidates` -- it is
 * borrowed, not owned, and must not be freed.
 *
 * `maxDistance` is deliberately a parameter rather than a constant
 * baked into this function: how "close" is close enough to suggest is
 * a policy decision that depends on the caller's vocabulary (e.g.
 * proportional to the input's length), and this function stays a
 * generic reusable primitive rather than encoding that policy itself.
 */
const char *SuggestClosest(const char *input, const char *const *candidates, usize n, usize maxDistance);

#endif /* SUGGEST_H_ */
