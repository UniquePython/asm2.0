#ifndef CLI_H_
#define CLI_H_

#include "types.h"

typedef enum pipeline_stage_t
{
    STAGE_TOKENIZE,
    STAGE_PARSE,
    STAGE_COMPILE,
} PipelineStage;

typedef struct cli_options_t
{
    const char *inputPath; /* borrowed from argv -- argv outlives this struct */
    char *outputPath;      /* ALWAYS owned, regardless of whether it came from
                            * --output or was derived from inputPath. Uniform
                            * ownership: always Free it, no exceptions, no
                            * branch-dependent rule. */

    bool wantTokenize;
    bool wantParse;
    bool wantCompile;

    PipelineStage furthest; /* derived: max stage among the wantX flags above */
} CliOptions;

CliOptions ParseCli(int argc, char **argv);
void CliOptionsFree(CliOptions *opts);

#endif /* CLI_H_ */
