# Azure Table Storage Setup Guide

This guide shows how to set up Azure Table Storage to store your water level telemetry data for historical analysis and reporting.

## Prerequisites

- Azure subscription
- Azure IoT Hub already created
- Azure Storage account (created in IoT Hub setup)

## Step 1: Create Storage Account

### Via Azure Portal

1. **Create Storage Account:**
   - Azure Portal → "Create a resource"
   - Search for "Storage account"
   - Click "Storage account" → "Create"

2. **Fill in details:**
   - **Subscription:** Your subscription
   - **Resource group:** Same as IoT Hub (e.g., `water-monitor-rg`)
   - **Storage account name:** Must be globally unique
     - Example: `watermonitordata001` (lowercase, no hyphens)
   - **Region:** Same as IoT Hub
   - **Performance:** Standard
   - **Redundancy:** Locally-redundant storage (LRS)
   - Click "Review + create" → "Create"

3. **Wait for deployment** (1-2 minutes)

## Step 2: Create Table Storage

1. **Navigate to Storage account:**
   - Azure Portal → Resource groups → water-monitor-rg
   - Select your storage account

2. **Create table:**
   - Left menu → "Tables" (under Data storage)
   - Click "+ Table"
   - **Table name:** `WaterLevelReadings`
   - Click "OK"

## Step 3: Table Schema

Your table will have the following structure:

```
PartitionKey    | RowKey              | Timestamp           | Depth | WaterPercent | Signal
────────────────┼─────────────────────┼─────────────────────┼───────┼──────────────┼────────
DAY-2026-05-11  | 2026-05-11T10:30:45 | 2026-05-11T10:30:45Z| 45.2  | 45           | -65
DAY-2026-05-11  | 2026-05-11T10:31:15 | 2026-05-11T10:31:15Z| 45.1  | 45           | -64
DAY-2026-05-12  | 2026-05-12T14:22:30 | 2026-05-12T14:22:30Z| 52.3  | 52           | -68
```

### Field Definitions

- **PartitionKey:** `DAY-YYYY-MM-DD` (used for efficient querying by date)
- **RowKey:** ISO timestamp of reading (ensures unique rows)
- **Timestamp:** Azure-managed timestamp
- **Depth:** Water depth in centimeters (float)
- **WaterPercent:** Water level percentage 0-100 (int)
- **Signal:** WiFi signal strength in dBm (int)
- **DeviceId:** Device identifier (string)

## Step 4: Get Storage Connection String

1. **In Storage account:**
   - Left menu → "Access keys"
   - Copy **Connection string** (key1)
   - Example format:
   ```
   DefaultEndpointsProtocol=https;AccountName=watermonitordata001;AccountKey=XXXXX==;EndpointSuffix=core.windows.net
   ```

2. **Save to your dashboard:**
   - `dashboard/.env` → `STORAGE_CONNECTION_STRING=<paste here>`

## Step 5: Configure IoT Hub to Route Data to Storage

### Create Blob Storage Endpoint

1. **In Storage account:**
   - Left menu → "Containers"
   - Click "+ Container"
   - **Name:** `iot-messages`
   - **Public access level:** Private
   - Click "Create"

2. **In your IoT Hub:**
   - Left menu → "Message routing" → "Endpoints"
   - Click "+ Add" → "Storage"
   - **Endpoint name:** `water-data-blob`
   - **Storage account:** Select your account
   - **Container:** `iot-messages`
   - **File name format:** `{iothub}/{partition}/{YYYY}/{MM}/{DD}/{HH}/{mm}/{ss}.json`
   - Click "Create"

### Create Routing Rule

1. **Still in Message routing:**
   - Click "Routes" tab
   - Click "+ Add"
   - **Name:** `water-data-route`
   - **Endpoint:** `water-data-blob`
   - **Data source:** Device Telemetry Messages
   - **Query:** `true` (all messages)
   - Toggle "Enable route:** ON
   - Click "Save"

## Step 6: Insert Data into Table Storage

Your Node.js dashboard will automatically insert data into Table Storage. Here's the insertion logic:

```javascript
// Example: Inserting a reading from the dashboard
const { TableClient } = require("@azure/data-tables");

const tableClient = new TableClient(
  process.env.STORAGE_CONNECTION_STRING,
  "WaterLevelReadings"
);

const entity = {
  partitionKey: `DAY-${new Date().toISOString().split('T')[0]}`,
  rowKey: new Date().toISOString(),
  depth: 45.2,
  waterPercent: 45,
  signal: -65,
  deviceId: "esp32-water-monitor"
};

await tableClient.createEntity(entity);
```

## Step 7: Query Data

### Via Azure Portal

1. **In Table Storage:**
   - Select your table
   - Click "Query editor" (preview)
   - Write OData queries:

```odata
# Get all readings from today
PartitionKey eq 'DAY-2026-05-11'

# Get readings with water level > 50%
water_level_percent gt 50

# Get last 10 readings
PartitionKey eq 'DAY-2026-05-11' and RowKey ge '2026-05-11T10:00:00Z'
```

### Via Node.js (in your dashboard)

```javascript
const { TableClient } = require("@azure/data-tables");

const tableClient = new TableClient(
  process.env.STORAGE_CONNECTION_STRING,
  "WaterLevelReadings"
);

// Get readings from today
const today = new Date().toISOString().split('T')[0];
const entities = await tableClient.listEntities({
  queryOptions: { filter: `PartitionKey eq 'DAY-${today}'` }
});

for await (const entity of entities) {
  console.log(entity);
}
```

## Step 8: Monitor Storage

1. **In Storage account:**
   - Left menu → "Metrics"
   - Monitor used storage and transaction counts
   - Set up alerts for unusual activity

2. **Set up alerts:**
   - Left menu → "Alerts" → "New alert rule"
   - **Condition:** Storage capacity exceeds threshold
   - **Action:** Send email or webhook

## Cost Estimation

- **Storage account:** $0.50-2/month for small projects
  - $0.01 per 10,000 write operations
  - $0.01 per 10,000 read operations
  - $0.001 per GB stored

Example: 3 readings/minute = 4,320 writes/day ≈ $0.15/month

## Best Practices

✅ **Do:**
- Partition data by date (improves query performance)
- Use rowkey with timestamp (ensures uniqueness and ordering)
- Archive old data to Blob Storage (save costs)
- Implement data retention policies
- Monitor storage costs

❌ **Don't:**
- Store every raw sensor reading (sample/aggregate)
- Use timestamps as partition keys (creates hot partitions)
- Keep all historical data forever (archive old data)
- Use storage for real-time dashboards (use cache layer)

## Data Retention

Implement automatic cleanup:

```javascript
// Delete readings older than 90 days
const cutoffDate = new Date();
cutoffDate.setDate(cutoffDate.getDate() - 90);

const dayString = cutoffDate.toISOString().split('T')[0];

const entities = await tableClient.listEntities({
  queryOptions: { filter: `PartitionKey le 'DAY-${dayString}'` }
});

for await (const entity of entities) {
  await tableClient.deleteEntity(entity.partitionKey, entity.rowKey);
}
```

## Troubleshooting

### Cannot create table
- Check Storage account is in same region as IoT Hub
- Verify you have "Contributor" role

### High storage costs
- Check message routing isn't duplicating messages
- Implement data retention cleanup
- Archive old data to blob storage

### Slow queries
- Ensure you're filtering by PartitionKey first
- Use rowkey range queries (efficient)
- Avoid scanning entire table

## Next Steps

1. [Deploy web dashboard](../dashboard/README.md)
2. [Set up Stream Analytics](stream_analytics_setup.md) (optional)
3. [Configure data retention policies]()

## References

- [Azure Table Storage Documentation](https://docs.microsoft.com/en-us/azure/storage/tables/)
- [Table Storage REST API](https://docs.microsoft.com/en-us/rest/api/storageservices/table-service-rest-api)
- [Azure SDK for Node.js - Tables](https://github.com/Azure/azure-sdk-for-js/tree/main/sdk/tables/data-tables)
