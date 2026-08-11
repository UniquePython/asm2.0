#!/usr/bin/env bash
#
# Test harness for asm2.0.
#
# Runs every case in tests/cases/<name>/, comparing the assembler's exit
# code / stdout / stderr (and, for successful --compile cases, the exit
# code of the produced binary) against tests/cases/<name>/expect.txt.
#
# All scratch output (compiled binaries, captured stdout/stderr) lives in
# a per-run tmpdir -- nothing is ever written to the project root.
#
# Exit status: the number of failed tests (0 == all passed). This makes
# the script directly usable as a CI/pre-commit gate.

set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BINARY="$ROOT_DIR/bin/asm2.0"
CASES_DIR="$ROOT_DIR/tests/cases"

if [ ! -x "$BINARY" ]; then
    echo "error: $BINARY not found or not executable -- run 'make' first" >&2
    exit 1
fi

TMPDIR_ROOT=$(mktemp -d)
trap 'rm -rf "$TMPDIR_ROOT"' EXIT

pass_count=0
fail_count=0

# Reads a single 'key: value' line out of an expect.txt file.
# Returns empty string (not an error) if the key isn't present.
read_field()
{
    local file="$1"
    local key="$2"
    local line

    line=$(grep -m1 "^${key}:" "$file" || true)
    [ -z "$line" ] && return 0

    echo "${line#"${key}": }"
}

# Reads the values of ALL 'key: value' lines with a given key, one per
# output line. Used for fields that may legitimately repeat in a single
# fixture (stdout_contains, stderr_contains) -- e.g. asserting multiple
# distinct diagnostics were all reported.
read_field_all()
{
    local file="$1"
    local key="$2"

    grep "^${key}:" "$file" | sed "s/^${key}: //"
}

run_case()
{
    local case_dir="$1"
    local name
    name=$(basename "$case_dir")

    local input="$case_dir/input.asm2"
    local expect="$case_dir/expect.txt"

    if [ ! -f "$input" ] || [ ! -f "$expect" ]; then
        echo "FAIL: $name (missing input.asm2 or expect.txt)"
        fail_count=$((fail_count + 1))
        return
    fi

    local stage want_exit want_run_exit
    stage=$(read_field "$expect" "stage")
    want_exit=$(read_field "$expect" "exit")
    want_run_exit=$(read_field "$expect" "run_exit")

    local stdout_wants stderr_wants
    stdout_wants=$(read_field_all "$expect" "stdout_contains")
    stderr_wants=$(read_field_all "$expect" "stderr_contains")

    if [ -z "$stage" ] || [ -z "$want_exit" ]; then
        echo "FAIL: $name (expect.txt missing required 'stage:' or 'exit:')"
        fail_count=$((fail_count + 1))
        return
    fi

    local case_tmp out_file err_file bin_file
    case_tmp=$(mktemp -d "$TMPDIR_ROOT/${name}.XXXXXX")
    out_file="$case_tmp/stdout.txt"
    err_file="$case_tmp/stderr.txt"
    bin_file="$case_tmp/a.out"

    local args=("--$stage")
    if [ "$stage" = "compile" ]; then
        args+=(--output "$bin_file")
    fi
    args+=("$input")

    "$BINARY" "${args[@]}" >"$out_file" 2>"$err_file"
    local got_exit=$?

    local failures=()

    if [ "$got_exit" != "$want_exit" ]; then
        failures+=("exit code: want $want_exit, got $got_exit")
    fi

    if [ -n "$stdout_wants" ]; then
        while IFS= read -r want; do
            [ -z "$want" ] && continue
            if ! grep -qF -- "$want" "$out_file"; then
                failures+=("stdout missing: \"$want\"")
            fi
        done <<< "$stdout_wants"
    fi

    if [ -n "$stderr_wants" ]; then
        while IFS= read -r want; do
            [ -z "$want" ] && continue
            if ! grep -qF -- "$want" "$err_file"; then
                failures+=("stderr missing: \"$want\"")
            fi
        done <<< "$stderr_wants"
    fi

    if [ -n "$want_run_exit" ]; then
        if [ "$stage" != "compile" ] || [ "$want_exit" != "0" ]; then
            failures+=("run_exit set but stage isn't a successful compile -- fixture error")
        elif [ ! -x "$bin_file" ]; then
            failures+=("run_exit expected but no binary was produced at $bin_file")
        else
            "$bin_file"
            local got_run_exit=$?
            if [ "$got_run_exit" != "$want_run_exit" ]; then
                failures+=("run exit code: want $want_run_exit, got $got_run_exit")
            fi
        fi
    fi

    if [ ${#failures[@]} -eq 0 ]; then
        echo "PASS: $name"
        pass_count=$((pass_count + 1))
    else
        echo "FAIL: $name"
        for f in "${failures[@]}"; do
            echo "    - $f"
        done
        if [ -s "$out_file" ]; then
            echo "    stdout:"
            sed 's/^/      /' "$out_file"
        fi
        if [ -s "$err_file" ]; then
            echo "    stderr:"
            sed 's/^/      /' "$err_file"
        fi
        fail_count=$((fail_count + 1))
    fi
}

if [ ! -d "$CASES_DIR" ] || [ -z "$(ls -A "$CASES_DIR" 2>/dev/null)" ]; then
    echo "no test cases found under $CASES_DIR"
    exit 0
fi

for case_dir in "$CASES_DIR"/*/; do
    run_case "${case_dir%/}"
done

echo "-----"
echo "$pass_count passed, $fail_count failed"

exit "$fail_count"
