#ifndef SYNTAX_H_
#define SYNTAX_H_

/*
 * Canonical, human-readable grammar reminders, one per statement kind.
 *
 * These are meant to be shown alongside diagnostics as a "syntax:"
 * line -- a quick reminder of the correct shape of the statement the
 * reader was trying to write, independent of whatever specifically
 * went wrong. See diagnostic.h's DiagsPushFull / CodegenErrorFull.
 *
 * Deliberately not covering every possible sub-production (e.g. there
 * is no OPERAND_SYNTAX) -- these macros exist for whole statements,
 * which have one unambiguous canonical shape. Sub-grammars used in
 * multiple contexts (like operands) don't have a single shape to show
 * and so are left to their own diagnostic wording instead.
 */

#define LABEL_SYNTAX "label <name>:"
#define MOVE_SYNTAX "move <number> to <register>;"
#define SYSCALL_SYNTAX "syscall;"
#define ENTRY_SYNTAX "entry <label>;"

#endif /* SYNTAX_H_ */
