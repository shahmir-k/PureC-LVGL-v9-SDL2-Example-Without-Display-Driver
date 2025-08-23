#!/bin/bash

# This script is used to build an ARM64 version of the app on a Raspberry Pi
# If launched with the -upload flag, it will upload the app to the muOS device




# Configuration variables - Edit these with your app details
APP_OLD_NAME="app-exampleLVGL" # This is the executable that the makefile is set up to build
APP_NEW_NAME="app-exampleLVGL_ARM64" # This will be the new name of the executable once moved back to the project folder
BUILD_DIR="Documents" # The directory where the app folder is made for building
APP_FOLDER_NAME="app-exampleLVGL" # The name of the app folder in which the app is built

# Configuration variables - Edit these with your Raspberry Pi details
RPI_USER="shahmir"
RPI_PASS="khan"
RPI_ADDRESS="192.168.0.214"

# Configuration variables - Edit these with your muOS device details
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


echo -e "${GREEN}Starting Raspberry Pi build process...${NC}"

# Check if sshpass is installed
if ! command -v sshpass &> /dev/null; then
    echo -e "${RED}sshpass is not installed. Installing...${NC}"
    sudo apt-get update && sudo apt-get install -y sshpass
fi

# Function to run SSH commands
run_ssh_command() {
    sshpass -p "$RPI_PASS" ssh -o StrictHostKeyChecking=no "$RPI_USER@$RPI_ADDRESS" "$1"
}

# Function to run SCP commands
run_scp_command() {
    sshpass -p "$RPI_PASS" scp -o StrictHostKeyChecking=no "$1" "$2"
}

echo "Checking connection to Raspberry Pi..."
if ! run_ssh_command "echo 'Connection successful'"; then
    echo -e "${RED}Failed to connect to Raspberry Pi. Please check your credentials and connection.${NC}"
    exit 1
fi

echo "Removing old $APP_FOLDER_NAME directory if it exists..."
run_ssh_command "rm -rf ~/$BUILD_DIR/$APP_FOLDER_NAME"

echo "Installing required packages on Raspberry Pi..."
#run_ssh_command "sudo apt-get update && sudo apt-get install -y libsdl2-dev libsdl2-ttf-dev"

echo "Creating $BUILD_DIR directory if it doesn't exist..."
run_ssh_command "mkdir -p ~/$BUILD_DIR"

echo "Uploading $APP_FOLDER_NAME directory..."
sshpass -p "$RPI_PASS" scp -r $APP_FOLDER_NAME "$RPI_USER@$RPI_ADDRESS:~/$BUILD_DIR/"

echo "Building on Raspberry Pi..."
run_ssh_command "cd ~/$BUILD_DIR/$APP_FOLDER_NAME && make clean && make"

echo "Copying built executable back..."
sshpass -p "$RPI_PASS" scp "$RPI_USER@$RPI_ADDRESS:~/$BUILD_DIR/$APP_FOLDER_NAME/$APP_OLD_NAME" ./$APP_FOLDER_NAME/$APP_NEW_NAME

echo -e "${GREEN}Build process completed!${NC}"

# Check if the executable was copied back successfully
if [ -f "./$APP_FOLDER_NAME/$APP_NEW_NAME" ]; then
    echo -e "${GREEN}Successfully built and retrieved the executable${NC}"
else
    echo -e "${RED}Failed to retrieve the executable${NC}"
    exit 1
fi

# Check for -upload flag
if [ "$1" = "-upload" ]; then
    upload_to_device
fi
