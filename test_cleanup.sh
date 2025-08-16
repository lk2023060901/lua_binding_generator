#!/bin/bash

# Simple test for cleanup script
echo "Testing cleanup script..."

# Get the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLEANUP_SCRIPT="$SCRIPT_DIR/scripts/clean_thirdparty.sh"

echo "Script path: $CLEANUP_SCRIPT"

if [ -f "$CLEANUP_SCRIPT" ]; then
    echo "Cleanup script exists"
    
    # Make executable
    chmod +x "$CLEANUP_SCRIPT"
    
    # Test dry run
    echo "Running dry run test..."
    "$CLEANUP_SCRIPT" --dry-run --level=light --force
    
    echo "Test completed"
else
    echo "Cleanup script not found"
fi