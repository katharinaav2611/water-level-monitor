# Static Water Station Dashboard

A static dashboard for campus water stations using HTML/CSS/JS and Chart.js.

## Features

- Real-time style station monitoring cards (water level, status, location, last reading)
- Mobile-responsive layout with hamburger navigation
- Configurable auto-refresh interval (10/15/20/30 seconds)
- Chart.js analytics for consumption, historical water level, and most-used stations
- Offline-capable behavior with local data caching (`localStorage`)
- Azure IoT endpoint integration via configurable API endpoint

## File Structure

```
dashboard/
├── index.html
├── css/
│   └── style.css
├── js/
│   ├── dashboard.js
│   ├── azure-iot-client.js
│   └── charts.js
└── data/
    └── stations-config.json
```

## Run Locally

From repository root:

```bash
python3 -m http.server 8080
```

Open: `http://localhost:8080/dashboard/`

## Azure IoT Integration

This dashboard expects telemetry in this shape:

```json
{
  "stationId": "STATION-001",
  "stationName": "Engineering Building",
  "location": "Building A, Floor 2",
  "waterLevel": 75,
  "status": "operational",
  "lastUpdate": "2026-05-14T10:30:00Z",
  "totalRefills": 245,
  "dailyConsumption": 120
}
```

Set endpoint details in `index.html`:

```html
<script>
  window.DASHBOARD_CONFIG = {
    iotDataEndpoint: 'https://<your-endpoint>/api/stations',
    endpointApiKey: '<optional-api-key>'
  };
</script>
```

If endpoint data is unavailable, the dashboard automatically uses cached data and then fallback sample data.

## GitHub Pages

Push the repository and enable GitHub Pages. The dashboard is available at `/dashboard/`.
