# Balise Sonore Testing Environment

## Setup

### Prerequisites
- Docker and Docker Compose installed
- Node.js installed (for running tests)

### Install Dependencies
1. Navigate to the tests directory:
```bash
cd tests
```

2. Install Node.js dependencies:
```bash
npm install
```

### Docker Environment Setup
1. Build the Docker images:
```bash
# From the tests directory
docker compose build
```

2. Verify the images were created:
```bash
docker images | grep balise_sonore
```

3. Start the containers:
```bash
docker compose up
```

4. Or start in detached mode:
```bash
docker compose up -d
```

5. Check running containers:
```bash
docker compose ps
```

### Access the Application
- Frontend: http://localhost:8080
- Mock API: http://localhost:5000

### Stopping the Environment
1. If running in foreground, use Ctrl+C
2. If running in detached mode:
```bash
docker compose down
```

### Rebuilding After Changes
```bash
docker compose down
docker compose build --no-cache
docker compose up
```

## Running Tests
```bash
npm test
```

## Troubleshooting
1. If ports are already in use:
```bash
# Check what's using the ports
sudo lsof -i :8080
sudo lsof -i :5000
```

2. Clean Docker environment:
```bash
docker compose down
docker system prune -f
```

## Directory Structure
```
tests/
├── app.test.js      # Frontend tests
├── mock_api.py      # Python mock API
├── nginx.conf       # Nginx configuration
├── requirements.txt # Python dependencies
├── package.json     # Node.js dependencies
└── docker-compose.yml
```
