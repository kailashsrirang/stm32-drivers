#!/bin/bash

SESSION_NAME="rtos-dev"
PROJECT_DIR="/Users/kailashsriranganathan/Documents/stm32-drivers"
ELF_PATH="build/kernel.elf"

# 1. Kill any existing session with this name to avoid conflicts
tmux kill-session -t $SESSION_NAME 2>/dev/null

# 2. Start a new detached session with Window 0 (Build & Flash)
tmux new-session -d -s $SESSION_NAME -n "Build"
tmux send-keys -t $SESSION_NAME:0 "cd $PROJECT_DIR && make clean && make && make flash" C-m

# 3. Create Window 1 (OpenOCD)
tmux new-window -t $SESSION_NAME:1 -n "OpenOCD"
tmux send-keys -t $SESSION_NAME:1 "cd $PROJECT_DIR && openocd -f interface/stlink.cfg -f target/stm32f4x.cfg" C-m

# 4. Create Window 2 (GDB)
tmux new-window -t $SESSION_NAME:2 -n "GDB"
tmux send-keys -t $SESSION_NAME:2 "cd $PROJECT_DIR && arm-none-eabi-gdb -ex 'target remote localhost:3333' -ex 'monitor reset halt' $ELF_PATH" C-m

# 5. Attach to the session and open on Window 0
tmux attach-session -t $SESSION_NAME
