#!/bin/bash
echo "[*] Installing"
meson install -C builddir
echo "[*] Running IDA"
IDA_BIN="/Applications/IDA Pro 7.6/ida.app/Contents/MacOS"
echo "[*] BIN: $IDA_BIN"
SRC_ROOT=$PWD
IN_FILE=$SRC_ROOT/../mips_samples/babymips
# IN_FILE=$SRC_ROOT/../../../shared/tctf/release_mipscoder/mipscoder2
# IN_FILE=$SRC_ROOT/../mips_samples/libc.so
echo "[*] Target file: $IN_FILE"
LOG_FILE=$SRC_ROOT/ida.log
PLUG_LOG_FILE=$SRC_ROOT/plugin.log
SCRIPT=$SRC_ROOT/startup_script.py

echo "Running \"$IDA_BIN/ida\" -A -c -pmipsl -L$LOG_FILE -Onmips_log_file:$PLUG_LOG_FILE -S"$SCRIPT" $IN_FILE"

"$IDA_BIN/ida" -c -L$LOG_FILE -Onmips_log_file:$PLUG_LOG_FILE -S"$SCRIPT" $IN_FILE
# ida_pid=$!
# tail -f $LOG_FILE &
# tail_pid=$!

# echo "[*] waiting on IDA to exit"
# wait $ida_pid
# kill -9 $tail_pid