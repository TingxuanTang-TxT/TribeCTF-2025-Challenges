#!/bin/bash

# CTF Challenge Cleanup Script
# Restores original docker-compose.yml and cleans up

echo "🧹 Cleaning up CTF Challenge deployment"
echo "======================================"

# Stop and remove containers for this project only
if [ -f docker-compose.yml ]; then
    echo "🛑 Stopping CTF containers..."
    docker-compose down -v --remove-orphans

    # Clean up project-specific containers by name pattern
    echo "🧹 Removing project containers..."
    docker ps -a --filter "name=participants-" --format "{{.ID}}" | xargs -r docker rm -f

    # Remove project network if it exists
    docker network ls --filter "name=participants_" --format "{{.ID}}" | xargs -r docker network rm 2>/dev/null || true
fi

# Restore original docker-compose.yml if backup exists
if [ -f docker-compose.yml.bak ]; then
    echo "🔄 Restoring original docker-compose.yml..."
    mv docker-compose.yml.bak docker-compose.yml
fi

# Remove port info file
if [ -f .port_info ]; then
    echo "🗑️ Removing port configuration..."
    rm .port_info
fi

# Clean up SQLite databases
if [ -d data ]; then
    echo "🗃️ Cleaning up participant databases..."
    rm -rf data/
fi

# Remove participant-specific databases
echo "🗃️ Removing participant database files..."
rm -f flaskr_*.sqlite

# Remove Docker volumes
echo "🐳 Removing Docker volumes..."
docker volume prune -f

# Remove Docker images for this project
echo "🐳 Removing project Docker images..."
docker images --filter "reference=participants*" -q | xargs -r docker rmi -f

# Clear any cached database files
echo "🗃️ Cleaning cache and temporary files..."
rm -f *.sqlite-wal *.sqlite-shm

# Remove any other backup files
rm -f *.bak

echo "✅ Cleanup complete!"
echo ""
echo "To redeploy, run: ./deploy.sh"