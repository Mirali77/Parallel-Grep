#!/usr/bin/env bash
set -euo pipefail

BIN="${BIN:-./build-release/pgrep}"
DATA_DIR="${DATA_DIR:-data}"

JOBS_LIST_STR="${JOBS_LIST:-"1 4 6"}"
read -r -a JOBS_LIST <<< "$JOBS_LIST_STR"

WARMUP="${WARMUP:-2}"
RUNS="${RUNS:-10}"

REGEX_PATTERN="${REGEX_PATTERN:-ERROR|WARN}" # Ищем по регулярке "ERROR|WARN"
LITERAL_PATTERN="${LITERAL_PATTERN:-TODO}" # Ищем по литералу "TODO"

need_dir () {
    [[ -d "$1" ]] || { echo "Missing dataset dir: $1" >&2; exit 1; }
}

[[ -x "$BIN" ]] || { echo "Missing binary: $BIN" >&2; exit 1; }
need_dir "$DATA_DIR/small"
need_dir "$DATA_DIR/mix"
need_dir "$DATA_DIR/big"

run_group () {
    local title="$1"
    shift
    echo ""
    echo "=============================="
    echo "$title" # Принтим команду
    echo "=============================="
    hyperfine -w "$WARMUP" -r "$RUNS" "$@"
}

cmds_regex () {
    local dataset="$1"
    local cmds=()
    for j in "${JOBS_LIST[@]}"; do
        cmds+=("$BIN --count --jobs $j --regex \"$REGEX_PATTERN\" \"$dataset\" > /dev/null")
    done
    printf '%s\n' "${cmds[@]}"
}

cmds_literal () {
    local dataset="$1"
    local cmds=()
    for j in "${JOBS_LIST[@]}"; do
        cmds+=("$BIN --count --jobs $j \"$LITERAL_PATTERN\" \"$dataset\" > /dev/null")
    done
    printf '%s\n' "${cmds[@]}"
}

for ds in small mix big; do
    dataset="$DATA_DIR/$ds"

    mapfile -t RCMDS < <(cmds_regex "$dataset")
    run_group "REGEX dataset=$ds pattern=$REGEX_PATTERN jobs=${JOBS_LIST[*]}" "${RCMDS[@]}"

    mapfile -t LCMDS < <(cmds_literal "$dataset")
    run_group "LITERAL dataset=$ds pattern=$LITERAL_PATTERN jobs=${JOBS_LIST[*]}" "${LCMDS[@]}"
done
