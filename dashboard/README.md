# Dashboard — Environmental Monitor

React frontend + Node.js/Express backend that receives data from Pod 2 (ESP32) and displays live sensor readings.

## Stack
- **Frontend**: React + Recharts
- **Backend**: Node.js + Express
- **Storage**: JSON file (or swap for SQLite/InfluxDB)

## Setup
```bash
cd dashboard
npm install
npm run dev      # starts both frontend and backend
```

## API Endpoints
| Method | Route | Description |
|--------|-------|-------------|
| POST | `/api/data` | Receive sensor data from pods |
| GET | `/api/data/latest` | Get latest readings |
| GET | `/api/data/history` | Get historical readings |

## Environment Variables
Create a `.env` file:
```
PORT=3000
DATA_FILE=./server/data.json
```
