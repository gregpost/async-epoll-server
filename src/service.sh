#!/bin/bash

if [[ "$EUID" -ne 0 ]]; then
    echo "Please run this script with sudo."
    exit 1
fi

echo "Copying binary to /usr/local/bin..."
install -v -m 755 ./src/server /usr/local/bin/asynserver

echo "Creating systemd service file..."
install -v -m 644 ./src/asynserver.service /etc/systemd/system/asynserver.service

echo "Reloading systemd daemon..."
systemctl daemon-reload

echo "Enabling the service on boot..."
systemctl enable asynserver

echo "Starting the service..."
systemctl start asynserver

echo "Checking service status:"
systemctl status asynserver
