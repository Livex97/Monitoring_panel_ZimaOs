#!/bin/bash

# Script to build and push Docker image with optional version tag
# Usage: ./build-and-push-docker.sh [version]
# If no version is provided, it will use the git tag or current date as default

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
IMAGE_NAME="ghcr.io/${GITHUB_REPOSITORY_OWNER:-$(git config user.name)}/zimaos-monitor-api"
PLATFORMS="linux/amd64,linux/arm64"
DRY_RUN=""

# Parse arguments
VERSION="${1:-}"

print_usage() {
    echo "Usage: $0 [version]"
    echo "  version: Optional tag version (e.g., v1.2.3). If not provided, will use git tag or generate from date."
    echo "  --dry-run: Only build, don't push to registry"
    echo "  --help: Show this help message"
}

# Parse flags
while [[ $# -gt 0 ]]; do
    case $1 in
        --dry-run)
            DRY_RUN="--dry-run"
            shift
            ;;
        --help)
            print_usage
            exit 0
            ;;
        -*)
            echo "Unknown option: $1"
            print_usage
            exit 1
            ;;
        *)
            VERSION="$1"
            shift
            ;;
    esac
done

# Get repository info
REPO_OWNER="$(git config user.name || echo "unknown")"
REPO_NAME="$(basename "$(git rev-parse --show-toplevel)")"
IMAGE_NAME="ghcr.io/${REPO_OWNER}/${REPO_NAME}"

echo -e "${GREEN}🐳 Building Docker image for ${IMAGE_NAME}${NC}"

# Get version if not provided
if [ -z "$VERSION" ]; then
    # Try to get the current git tag
    VERSION="$(git describe --tags --abbrev=0 2>/dev/null || echo "v$(date +%Y.%m.%d)")"
    echo -e "${YELLOW}📅 No version specified, using: ${VERSION}${NC}"
fi

# Remove 'v' prefix if present for image tag comparison
VERSION_CLEAN="${VERSION#v}"

# Generate tags
TAGS=(
    "${IMAGE_NAME}:${VERSION_CLEAN}"
    "${IMAGE_NAME}:latest"
)

# Add version-specific tag if different from latest
if [[ "$VERSION_CLEAN" != "latest" ]]; then
    TAGS+=("${IMAGE_NAME}:latest-${VERSION_CLEAN}")
fi

# Create buildx if not exists
if ! docker buildx inspect monitor-builder >/dev/null 2>&1; then
    echo -e "${YELLOW}🔧 Creating Docker Buildx builder...${NC}"
    docker buildx create --name monitor-builder --use
fi

# Build the Docker image
BUILD_ARGS=""
if [ -n "$DRY_RUN" ]; then
    BUILD_ARGS="$BUILD_ARGS $DRY_RUN"
fi

BUILD_START=$(date +%s)

echo -e "${YELLOW}🏗️  Building image with tags: ${TAGS[*]}${NC}"

docker buildx build \
    --platform "$PLATFORMS" \
    --tag "${TAGS[0]}" \
    --tag "${TAGS[1]}" \
    ${TAGS[2]:+--tag "${TAGS[2]}"} \
    --file zimaos-monitor-api/Dockerfile \
    --build-arg BUILD_DATE="$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
    --build-arg VCS_REF="$(git rev-parse HEAD)" \
    $BUILD_ARGS \
    ./zimaos-monitor-api

BUILD_END=$(date +%s)
BUILD_DURATION=$((BUILD_END - BUILD_START))

echo -e "${GREEN}✅ Build completed in ${BUILD_DURATION}s${NC}"

# Push to registry (unless dry run)
if [ -z "$DRY_RUN" ]; then
    echo -e "${YELLOW}🚀 Pushing image to GitHub Container Registry...${NC}"

    PUSH_START=$(date +%s)
    for TAG in "${TAGS[@]}"; do
        echo -e "   Pushing ${TAG}..."
        docker buildx build \
            --platform "$PLATFORMS" \
            --tag "$TAG" \
            --file zimaos-monitor-api/Dockerfile \
            --push \
            ./zimaos-monitor-api
    done
    PUSH_END=$(date +%s)
    PUSH_DURATION=$((PUSH_END - PUSH_START))

    echo -e "${GREEN}✅ Push completed in ${PUSH_DURATION}s${NC}"
    echo -e "${GREEN}📦 Image available at: ${TAGS[0]}${NC}"
else
    echo -e "${YELLOW}🔍 Dry run mode - skipping push${NC}"
fi

echo -e "${GREEN}🎉 Docker image build and push completed successfully!${NC}"
echo ""
echo "Generated tags:"
for TAG in "${TAGS[@]}"; do
    echo "  - ${TAG}"
done