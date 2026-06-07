# Use Python 3.11 slim image
FROM python:3.11-slim

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# Copy requirements and install Python dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy application files
COPY . .

# Ensure original database exists
COPY flaskr.sqlite .

# Create directory for SQLite databases
RUN mkdir -p /app/data

# Set environment variables
ENV FLASK_APP=hosted-app.py
ENV FLASK_SECRET_KEY=production_secret_key

# Expose port
EXPOSE 5000

# Run the application
CMD ["python", "hosted-app.py"]