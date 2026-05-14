(function () {
  const CACHE_KEY = 'water-dashboard-cache-v1';

  class AzureIoTClient {
    constructor(options) {
      this.endpoint = options?.endpoint || '';
      this.apiKey = options?.apiKey || '';
    }

    async fetchStationData(stationConfigs) {
      const fallbackRecords = stationConfigs.map((station) => this.createFallbackStation(station));

      if (!this.endpoint || !navigator.onLine) {
        return this.getFromCacheOrFallback(fallbackRecords);
      }

      try {
        const response = await fetch(this.endpoint, {
          headers: this.apiKey ? { 'x-api-key': this.apiKey } : {}
        });

        if (!response.ok) {
          throw new Error('Azure endpoint request failed');
        }

        const payload = await response.json();
        const records = Array.isArray(payload) ? payload : payload.stations;

        if (!Array.isArray(records) || !records.length) {
          throw new Error('Azure endpoint returned no records');
        }

        const merged = fallbackRecords.map((fallback) => {
          const fromCloud = records.find((item) => item.stationId === fallback.stationId);
          return this.normalizeRecord(fromCloud || fallback, fallback);
        });

        this.saveCache(merged);
        return merged;
      } catch (error) {
        return this.getFromCacheOrFallback(fallbackRecords);
      }
    }

    createFallbackStation(station) {
      const waterLevel = Math.round(Math.random() * 60 + 20);
      return {
        stationId: station.stationId,
        stationName: station.stationName,
        location: station.location,
        coordinates: station.coordinates,
        waterLevel,
        status: Math.random() < 0.15 ? 'maintenance' : 'operational',
        lastUpdate: new Date().toISOString(),
        totalRefills: Math.round(Math.random() * 350 + 100),
        dailyConsumption: Math.round(Math.random() * 150 + 30)
      };
    }

    normalizeRecord(record, fallback) {
      return {
        stationId: (record && record.stationId) || fallback.stationId,
        stationName: (record && record.stationName) || fallback.stationName,
        location: (record && record.location) || fallback.location,
        coordinates: (record && record.coordinates) || fallback.coordinates,
        waterLevel: this.toPercent(record && record.waterLevel, fallback.waterLevel),
        status: record && record.status === 'maintenance' ? 'maintenance' : 'operational',
        lastUpdate: (record && record.lastUpdate) || fallback.lastUpdate,
        totalRefills: this.toPositiveInt(record && record.totalRefills, fallback.totalRefills),
        dailyConsumption: this.toPositiveInt(record && record.dailyConsumption, fallback.dailyConsumption)
      };
    }

    toPercent(value, fallback) {
      const number = Number(value);
      if (Number.isFinite(number)) {
        return Math.max(0, Math.min(100, Math.round(number)));
      }

      return fallback;
    }

    toPositiveInt(value, fallback) {
      const number = Number(value);
      if (Number.isFinite(number) && number >= 0) {
        return Math.round(number);
      }

      return fallback;
    }

    saveCache(records) {
      localStorage.setItem(
        CACHE_KEY,
        JSON.stringify({
          updatedAt: new Date().toISOString(),
          records
        })
      );
    }

    getFromCacheOrFallback(fallbackRecords) {
      const cachedRaw = localStorage.getItem(CACHE_KEY);
      if (!cachedRaw) {
        return fallbackRecords;
      }

      try {
        const cached = JSON.parse(cachedRaw);
        if (Array.isArray(cached.records) && cached.records.length) {
          return cached.records;
        }
      } catch (error) {
        return fallbackRecords;
      }

      return fallbackRecords;
    }
  }

  window.AzureIoTClient = AzureIoTClient;
})();
