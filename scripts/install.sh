#!/bin/sh
set -e

PROJECT_OWNER="Ohto-Ai"
PROJECT_NAME="webhook"
BINARY_NAME="ohtoai-webhook"

# Detect OS and architecture
OS=$(uname -s)
case "$OS" in
    Linux)  OS_NAME="ubuntu-22.04" ;;
    *)      echo "Unsupported OS: $OS"; exit 1 ;;
esac

ARCH=$(uname -m)
case "$ARCH" in
    x86_64)  PLATFORM="${OS_NAME}" ;;
    aarch64) PLATFORM="${OS_NAME}-arm64" ;;
    *)       echo "Unsupported architecture: $ARCH"; exit 1 ;;
esac

# Fetch latest release info from GitHub API
echo "Fetching latest release for ${PROJECT_OWNER}/${PROJECT_NAME}..."
RELEASE_JSON=$(curl -fsSL "https://api.github.com/repos/${PROJECT_OWNER}/${PROJECT_NAME}/releases/latest" 2>/dev/null)

# Find matching asset
ASSET_NAME=$(echo "$RELEASE_JSON" | grep -o "\"name\": \"${PLATFORM}_[^\"]*\"" | head -1 | cut -d'"' -f4)
DOWNLOAD_URL=$(echo "$RELEASE_JSON" | grep -o "\"browser_download_url\": \"[^\"]*${ASSET_NAME}\"" | cut -d'"' -f4)

if [ -z "$DOWNLOAD_URL" ]; then
    echo "No matching release found for platform: ${PLATFORM}"
    echo "Falling back to building from source..."
    exit 1
fi

echo "Downloading ${ASSET_NAME}..."
curl -fsSL -o "/tmp/${BINARY_NAME}" "$DOWNLOAD_URL"

if echo "$ASSET_NAME" | grep -q '\.deb$'; then
    echo "Installing .deb package..."
    sudo dpkg -i "/tmp/${BINARY_NAME}"
else
    chmod +x "/tmp/${BINARY_NAME}"
    sudo mv "/tmp/${BINARY_NAME}" "/usr/local/bin/${BINARY_NAME}"
fi

echo "${BINARY_NAME} installed successfully to /usr/local/bin/${BINARY_NAME}"
