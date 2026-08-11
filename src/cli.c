#include "cli.h"

#include <string.h>
#include <getopt.h>

#include "diagnostic.h"
#include "mem.h"

enum
{
    OPT_TOKENIZE = 1,
    OPT_PARSE,
    OPT_COMPILE,
    OPT_OUTPUT,
};

static struct option longOpts[] = {
    {"tokenize", no_argument, NULL, OPT_TOKENIZE},
    {"parse", no_argument, NULL, OPT_PARSE},
    {"compile", no_argument, NULL, OPT_COMPILE},
    {"output", required_argument, NULL, OPT_OUTPUT},
    {NULL, 0, NULL, 0},
};

/*
 * Derives the default output path from an input source path, when
 * --output wasn't given.
 *
 * Rule: take the basename (directory component discarded -- the
 * default output always lands in CWD, never alongside the input),
 * strip everything from the FIRST '.' onward (not just the last --
 * "all extensions", so "my.prog.asm2" -> "my"), then always append
 * ".out" -- even when there was nothing to strip. That last part is
 * deliberate: it's what stops the compiler from overwriting an
 * extension-less input file (e.g. plain "prog") with its own output.
 *
 * Only '/' is treated as a path separator -- this project is
 * Linux-only, so no '\' handling is needed here.
 *
 * Returns a heap-allocated, NUL-terminated string. Caller owns it.
 */
static char *DefaultOutputPath(const char *inputPath)
{
    const char *basename = strrchr(inputPath, '/');

    if (basename)
        basename++;
    else
        basename = inputPath;

    const char *dot = strchr(basename, '.');

    usize stemLen = dot ? (usize)(dot - basename) : strlen(basename);

    static const char suffix[] = ".out";
    usize suffixLen = sizeof(suffix) - 1;

    char *outputPath = Alloc(stemLen + suffixLen + 1);

    memcpy(outputPath, basename, stemLen);
    memcpy(outputPath + stemLen, suffix, suffixLen);

    outputPath[stemLen + suffixLen] = '\0';

    return outputPath;
}

/*
 * Copies `str` into a fresh heap allocation. Used to give --output's
 * value the same ownership story as the derived-default case, so
 * CliOptions::outputPath is uniformly owned no matter which path set
 * it -- see cli.h.
 */
static char *OwnedCopy(const char *str)
{
    usize len = strlen(str);

    char *copy = Alloc(len + 1);

    memcpy(copy, str, len);
    copy[len] = '\0';

    return copy;
}

CliOptions ParseCli(int argc, char **argv)
{
    CliOptions opts = {
        .inputPath = NULL,
        .outputPath = NULL,
        .wantTokenize = false,
        .wantParse = false,
        .wantCompile = false,
        .furthest = STAGE_TOKENIZE,
    };

    const char *outputArg = NULL; /* raw --output value, if given; NULL means "derive a default" */

    opterr = 0; /* suppress getopt_long's own stderr messages -- we report our own */

    int c;
    while ((c = getopt_long(argc, argv, "", longOpts, NULL)) != -1)
    {
        switch (c)
        {
        case OPT_TOKENIZE:
            opts.wantTokenize = true;
            break;

        case OPT_PARSE:
            opts.wantParse = true;
            break;

        case OPT_COMPILE:
            opts.wantCompile = true;
            break;

        case OPT_OUTPUT:
            outputArg = optarg;
            break;

        case '?':
            if (optopt)
                Error("unknown option '-%c'", optopt);
            Error("unknown option");

        default:
            InternalError("unhandled getopt_long return value %d", c);
        }
    }

    /* Exactly one positional argument: the input file. */
    if (optind >= argc)
        Error("no input file");

    if (argc - optind > 1)
        Error("unexpected extra argument '%s'", argv[optind + 1]);

    opts.inputPath = argv[optind];

    /* At least one pipeline stage must be requested. */
    if (!opts.wantTokenize && !opts.wantParse && !opts.wantCompile)
        Error("no pipeline stage specified");

    /* --output only makes sense when compiling. */
    if (outputArg && !opts.wantCompile)
        Error("--output requires --compile");

    /* Determine furthest requested stage. */
    if (opts.wantParse)
        opts.furthest = STAGE_PARSE;

    if (opts.wantCompile)
        opts.furthest = STAGE_COMPILE;

    /* outputPath is ALWAYS owned. */
    if (outputArg)
        opts.outputPath = OwnedCopy(outputArg);
    else
        opts.outputPath = DefaultOutputPath(opts.inputPath);

    return opts;
}

void CliOptionsFree(CliOptions *opts)
{
    free(opts->outputPath);
    opts->outputPath = NULL;
}
