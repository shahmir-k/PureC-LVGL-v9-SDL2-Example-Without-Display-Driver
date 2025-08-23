#!/bin/sh
# HELP: Custom SDL2 Screensaver
# ICON: screensaver
# GRID: SDL2 Screensaver

#help,icon,grid are used by muOS to populate help message, app icon, and app name in the app grid

. /opt/muos/script/var/func.sh

# Set up environment for our SDL2 screensaver
APP_NAME="app-exampleLVGL_ARM64"
APP_DIR="/mnt/mmc/MUOS/application/app-exampleLVGL"


# Let the frontend know to back to the app grid after the app exits
echo app >/tmp/ACT_GO

# Controller support is handled directly by SDL2

# Launcher
cd "$APP_DIR" || exit

# Handle terminal mode
if [ "$1" = "-terminal" ]; then # If the -terminal flag is passed, force terminal mode
    FORCE_TERMINAL=true
else
    FORCE_TERMINAL=false
fi

if [ -t 0 ] || [ "$FORCE_TERMINAL" = true ]; then
    echo "Terminal detected, running in terminal mode"
    echo ""
    # Stop the frontend
    echo "Looking for muxlaunch and frontend.sh processes to stop frontend"
    while pgrep muxlaunch >/dev/null || pgrep frontend.sh >/dev/null; do
        killall -9 muxlaunch frontend.sh
        sleep 1
    done
fi

# Trap cleanup for when app exits (normally or forcibly)
cleanup() {
    # Clear the screen using dd to write black pixels
    #dd if=/dev/zero of=/dev/fb0 2>/dev/null
    
    # Kill any remaining processes
    killall -9 "$APP_NAME" 2>/dev/null
    #kill -9 "$(pidof gptokeyb2)" 2>/dev/null
}
trap cleanup EXIT INT TERM

# Run Application with output redirection
./"$APP_NAME" 2>&1     # Run our screensaver and show all output

# Cleanup after application exit
if [ -t 0 ] || [ "$FORCE_TERMINAL" = true ]; then
    echo "Application was running in terminal mode, restarting frontend"
    echo ""
    /opt/muos/script/mux/frontend.sh >/dev/null 2>&1 &  # Restart the frontend
fi
#kill -9 "$(pidof gptokeyb2)" 2>/dev/null # Kill the gptokeyb process