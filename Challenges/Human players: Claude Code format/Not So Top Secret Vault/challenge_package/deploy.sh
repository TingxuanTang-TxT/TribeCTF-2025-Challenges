#!/bin/bash

# CTF Challenge Deployment Script
# Not So Secret Vault - Session Forgery Challenge

echo "🔐 Not So Secret Vault CTF Challenge Deployment"
echo "============================================="

# Function to find available port
find_available_port() {
    local start_port=$1
    local port=$start_port

    while lsof -Pi :$port -sTCP:LISTEN -t >/dev/null 2>&1; do
        port=$((port + 1))
    done

    echo $port
}

# Create data directory for SQLite databases
mkdir -p data

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "❌ Docker is not installed. Please install Docker first."
    exit 1
fi

# Check if Docker Compose is installed
if ! command -v docker-compose &> /dev/null; then
    echo "❌ Docker Compose is not installed. Please install Docker Compose first."
    exit 1
fi

echo "✅ Docker and Docker Compose are installed"

# Find available ports
FLASK_PORT=$(find_available_port 5000)
NGINX_PORT=$(find_available_port 8080)

echo "🔍 Found available ports: Flask=$FLASK_PORT, Nginx=$NGINX_PORT"

# Update docker-compose.yml with available ports
sed -i.bak "s/\"5000:5000\"/\"$FLASK_PORT:5000\"/g" docker-compose.yml
sed -i.bak "s/\"80:80\"/\"$NGINX_PORT:80\"/g" docker-compose.yml

# Build and start the application
echo "🚀 Building and starting the CTF challenge..."

if [ "$1" = "production" ]; then
    echo "🌐 Starting with Nginx proxy (production mode)"
    docker-compose --profile production up --build -d
    ACTUAL_PORT=$NGINX_PORT
    echo "📋 Challenge is available at: http://localhost:$ACTUAL_PORT"
else
    echo "🧪 Starting in development mode"
    docker-compose up --build -d
    ACTUAL_PORT=$FLASK_PORT
    echo "📋 Challenge is available at: http://localhost:$ACTUAL_PORT"
fi

# Store the port info for cleanup
echo "FLASK_PORT=$FLASK_PORT" > .port_info
echo "NGINX_PORT=$NGINX_PORT" >> .port_info
echo "ACTUAL_PORT=$ACTUAL_PORT" >> .port_info

echo ""
echo "🎯 Challenge Information:"
echo "========================"
echo "• Challenge Type: Session Forgery"
echo "• Difficulty: Intermediate"
echo "• Objective: Access admin's secret notes"
echo ""
echo "🔍 Getting Started:"
echo "• Visit the application and click 'Start Challenge'"
echo ""
echo "🛠️ Management Commands:"
echo "• Stop challenge: docker-compose down"
echo "• View logs: docker-compose logs -f"
echo "• Clean up: docker-compose down -v && rm -rf data/"
echo ""
echo "✅ Deployment complete!"