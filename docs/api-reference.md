# Dashboard API Reference

Base URL: `http://localhost:3000`

## Endpoints

### POST /api/data
Receive a sensor data packet from Pod 2.
Body (JSON):
```json
{
  "pod": "pod-2",
  "timestamp": 1700000000,
  "soil": { "moisture": 45.2, "temp": 22.3, "ec": 1.1, "ph": 6.8 },
  "spectral": { "F1": 120, "F2": 340, "F3": 512, "NIR": 95, "CLR": 1024 }
}
```

### GET /api/data/latest
Returns the most recent reading from each pod.

### GET /api/data/history?hours=24
Returns readings for the past N hours (default 24).

### GET /api/status
Returns connectivity status of both pods and last-seen timestamps.
