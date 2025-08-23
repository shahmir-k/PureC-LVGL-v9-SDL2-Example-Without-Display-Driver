#!/bin/bash

# Configuration variables

# Configuration variables - Edit these with your app details
APP_OLD_NAME="app-exampleLVGL" # This is the executable that the makefile is set up to build
APP_NEW_NAME="app-exampleLVGL_ARM64" # This will be the new name of the executable once moved back to the project folder
BUILD_DIR="Documents" # The directory where the app folder is made for building
APP_FOLDER_NAME="app-exampleLVGL" # The name of the app folder in which the app is built


DEVICE_USER="root"
DEVICE_PASS="root"
DEVICE_ADDRESS="192.168.0.112"
DEVICE_UPLOAD_DIR="/mnt/mmc/MUOS/application/"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Function to upload to device
upload_to_device() {
    echo -e "${GREEN}Starting device upload process...${NC}"
    
    echo "Checking connection to device..."
    if ! sshpass -p "$DEVICE_PASS" ssh -o StrictHostKeyChecking=no "$DEVICE_USER@$DEVICE_ADDRESS" "echo 'Connection successful'"; then
        echo -e "${RED}Failed to connect to device. Please check your credentials and connection.${NC}"
        exit 1
    fi

    echo "Removing old $APP_FOLDER_NAME directory if it exists..."
    sshpass -p "$DEVICE_PASS" ssh -o StrictHostKeyChecking=no "$DEVICE_USER@$DEVICE_ADDRESS" "rm -rf $DEVICE_UPLOAD_DIR/$APP_FOLDER_NAME"

    echo "Uploading $APP_FOLDER_NAME directory to device..."
    # Create a temporary directory
    temp_dir=$(mktemp -d)
    # Copy everything except components
    cp -r $APP_FOLDER_NAME/* "$temp_dir/" 2>/dev/null || true
    rm -rf "$temp_dir/components"
    # Upload the filtered content
    sshpass -p "$DEVICE_PASS" scp -r "$temp_dir/"* "$DEVICE_USER@$DEVICE_ADDRESS:$DEVICE_UPLOAD_DIR/$APP_FOLDER_NAME/"
    # Cleanup
    rm -rf "$temp_dir"

    
    echo -e "${GREEN}Upload to device completed successfully!${NC}"

    #sshpass -p "$DEVICE_PASS" ssh -o StrictHostKeyChecking=no "$DEVICE_USER@$DEVICE_ADDRESS" "cd $DEVICE_UPLOAD_DIR/.screensaver && ./mux_launch.sh -terminal"
    exit 0
}


upload_to_device