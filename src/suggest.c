#include "suggest.h"

#include <string.h>

#include "mem.h"

static usize Min3(usize a, usize b, usize c)
{
    usize m = a;
    if (b < m)
        m = b;
    if (c < m)
        m = c;
    return m;
}

static usize DamerauLevenshtein(const char *a, const char *b)
{
    usize lenA = strlen(a);
    usize lenB = strlen(b);

    usize cols = lenB + 1;
    usize tableLen = (lenA + 1) * cols;

    usize *d = Alloc(tableLen * sizeof(*d));

    /* Base cases. */
    for (usize i = 0; i <= lenA; i++)
        d[i * cols] = i;

    for (usize j = 0; j <= lenB; j++)
        d[j] = j;

    /* Fill DP table. */
    for (usize i = 1; i <= lenA; i++)
    {
        for (usize j = 1; j <= lenB; j++)
        {
            usize cost = a[i - 1] == b[j - 1] ? 0 : 1;

            usize best = Min3(
                d[(i - 1) * cols + j] + 1,
                d[i * cols + (j - 1)] + 1,
                d[(i - 1) * cols + (j - 1)] + cost);

            if (i >= 2 && j >= 2 && a[i - 1] == b[j - 2] && a[i - 2] == b[j - 1])
            {
                usize transposition = d[(i - 2) * cols + (j - 2)] + 1;

                if (transposition < best)
                    best = transposition;
            }

            d[i * cols + j] = best;
        }
    }

    usize result = d[lenA * cols + lenB];

    Free(d);

    return result;
}

const char *SuggestClosest(const char *input, const char *const *candidates, usize n, usize maxDistance)
{
    const char *best = NULL;
    usize bestDistance = maxDistance + 1;

    for (usize i = 0; i < n; i++)
    {
        usize distance = DamerauLevenshtein(input, candidates[i]);

        if (distance <= maxDistance && distance < bestDistance)
        {
            best = candidates[i];
            bestDistance = distance;
        }
    }

    return best;
}
