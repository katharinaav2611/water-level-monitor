(function () {
  const SETTINGS_KEY = 'water-dashboard-settings';

  const state = {
    stationsConfig: [],
    stationsData: [],
    stationHistory: new Map(),
    autoRefresh: true,
    refreshIntervalSeconds: 15,
    timerId: null,
    charts: null,
    azureClient: null
  };

  const elements = {
    grid: document.getElementById('station-grid'),
    refreshStatus: document.getElementById('refresh-status'),
    lastRefreshLabel: document.getElementById('last-refresh-label'),
    connectionStatus: document.getElementById('connection-status'),
    autoRefreshToggle: document.getElementById('auto-refresh-toggle'),
    refreshIntervalSelect: document.getElementById('refresh-interval-select'),
    menuToggle: document.getElementById('menu-toggle'),
    mainNav: document.getElementById('main-nav'),
    metricOperational: document.getElementById('metric-operational'),
    metricAverageLevel: document.getElementById('metric-average-level'),
    metricTotalRefills: document.getElementById('metric-total-refills'),
    metricEffectiveness: document.getElementById('metric-effectiveness'),
    usageRanking: document.getElementById('usage-ranking')
  };

  async function init() {
    restoreSettings();
    bindControls();

    state.azureClient = new window.AzureIoTClient({
      endpoint: window.DASHBOARD_CONFIG?.iotDataEndpoint,
      apiKey: window.DASHBOARD_CONFIG?.endpointApiKey
    });

    state.charts = new window.DashboardCharts();

    try {
      const response = await fetch('data/stations-config.json');
      state.stationsConfig = await response.json();
    } catch (error) {
      elements.refreshStatus.textContent = 'Unable to load station configuration.';
      return;
    }

    await refreshData();
    applyAutoRefresh();
    updateConnectionStatus();
  }

  function restoreSettings() {
    const raw = localStorage.getItem(SETTINGS_KEY);
    if (!raw) {
      applySettingsToControls();
      return;
    }

    try {
      const parsed = JSON.parse(raw);
      state.autoRefresh = parsed.autoRefresh !== false;
      const interval = Number(parsed.refreshIntervalSeconds);
      if ([10, 15, 20, 30].includes(interval)) {
        state.refreshIntervalSeconds = interval;
      }
    } catch (error) {
      state.autoRefresh = true;
      state.refreshIntervalSeconds = 15;
    }

    applySettingsToControls();
  }

  function applySettingsToControls() {
    elements.autoRefreshToggle.checked = state.autoRefresh;
    elements.refreshIntervalSelect.value = String(state.refreshIntervalSeconds);
  }

  function persistSettings() {
    localStorage.setItem(
      SETTINGS_KEY,
      JSON.stringify({
        autoRefresh: state.autoRefresh,
        refreshIntervalSeconds: state.refreshIntervalSeconds
      })
    );
  }

  function bindControls() {
    elements.autoRefreshToggle.addEventListener('change', () => {
      state.autoRefresh = elements.autoRefreshToggle.checked;
      persistSettings();
      applyAutoRefresh();
    });

    elements.refreshIntervalSelect.addEventListener('change', () => {
      const selected = Number(elements.refreshIntervalSelect.value);
      if ([10, 15, 20, 30].includes(selected)) {
        state.refreshIntervalSeconds = selected;
        persistSettings();
        applyAutoRefresh();
      }
    });

    elements.menuToggle.addEventListener('click', () => {
      const expanded = elements.menuToggle.getAttribute('aria-expanded') === 'true';
      elements.menuToggle.setAttribute('aria-expanded', String(!expanded));
      elements.mainNav.classList.toggle('show');
    });

    window.addEventListener('online', updateConnectionStatus);
    window.addEventListener('offline', updateConnectionStatus);
  }

  async function refreshData() {
    elements.refreshStatus.textContent = 'Refreshing station telemetry…';
    state.stationsData = await state.azureClient.fetchStationData(state.stationsConfig);
    updateHistory(state.stationsData);
    renderStations(state.stationsData);
    renderOverview(state.stationsData);
    renderRankings(state.stationsData);
    state.charts.update(state.stationsData, state.stationHistory);

    const now = new Date();
    const timestamp = now.toLocaleString();
    elements.refreshStatus.textContent = `Live data updated at ${timestamp}`;
    elements.lastRefreshLabel.textContent = `Last refresh: ${timestamp}`;
  }

  function applyAutoRefresh() {
    if (state.timerId) {
      clearInterval(state.timerId);
      state.timerId = null;
    }

    if (!state.autoRefresh) {
      elements.refreshStatus.textContent = 'Auto refresh paused. Data is cached locally for offline viewing.';
      return;
    }

    state.timerId = setInterval(refreshData, state.refreshIntervalSeconds * 1000);
  }

  function updateHistory(stations) {
    const label = new Date().toLocaleTimeString();
    stations.forEach((station) => {
      const series = state.stationHistory.get(station.stationId) || [];
      series.push({ label, value: station.waterLevel });
      if (series.length > 24) {
        series.shift();
      }
      state.stationHistory.set(station.stationId, series);
    });
  }

  function renderStations(stations) {
    elements.grid.textContent = '';

    stations.forEach((station) => {
      const card = document.createElement('article');
      card.className = 'station-card';

      const header = document.createElement('div');
      header.className = 'station-header';

      const title = document.createElement('h3');
      title.textContent = station.stationName;

      const badge = document.createElement('span');
      const maintenance = station.status === 'maintenance';
      badge.className = `status-badge ${maintenance ? 'status-maintenance' : 'status-operational'}`;
      badge.textContent = maintenance ? 'Maintenance' : 'Operational';

      header.append(title, badge);

      const gauge = document.createElement('div');
      gauge.className = 'gauge';
      gauge.style.setProperty('--value', String(station.waterLevel));
      gauge.setAttribute('data-label', `${station.waterLevel}%`);

      const meta = document.createElement('div');
      meta.className = 'station-meta';

      const location = document.createElement('p');
      const coordinates = station.coordinates
        ? `(${station.coordinates.lat.toFixed(4)}, ${station.coordinates.lng.toFixed(4)})`
        : 'coordinates unavailable';
      location.textContent = `📍 ${station.location} ${coordinates}`;

      const stationId = document.createElement('p');
      stationId.textContent = `ID: ${station.stationId}`;

      const lastReading = document.createElement('p');
      lastReading.textContent = `Last reading: ${new Date(station.lastUpdate).toLocaleString()}`;

      meta.append(location, stationId, lastReading);
      card.append(header, gauge, meta);
      elements.grid.appendChild(card);
    });
  }

  function renderOverview(stations) {
    const operationalCount = stations.filter((item) => item.status !== 'maintenance').length;
    const averageLevel = stations.reduce((sum, item) => sum + item.waterLevel, 0) / Math.max(stations.length, 1);
    const totalRefills = stations.reduce((sum, item) => sum + item.totalRefills, 0);
    const successRate = stations.length ? Math.round((operationalCount / stations.length) * 100) : 0;

    elements.metricOperational.textContent = `${operationalCount}/${stations.length}`;
    elements.metricAverageLevel.textContent = `${Math.round(averageLevel)}%`;
    elements.metricTotalRefills.textContent = totalRefills.toLocaleString();
    elements.metricEffectiveness.textContent = `${successRate}% healthy`; 
  }

  function renderRankings(stations) {
    const ranked = [...stations].sort((a, b) => b.totalRefills - a.totalRefills);
    elements.usageRanking.textContent = '';
    ranked.slice(0, 5).forEach((station) => {
      const item = document.createElement('li');
      item.textContent = `${station.stationName}: ${station.totalRefills} refills`;
      elements.usageRanking.appendChild(item);
    });
  }

  function updateConnectionStatus() {
    elements.connectionStatus.textContent = navigator.onLine
      ? 'Online mode: dashboard attempts to sync data from Azure endpoint.'
      : 'Offline mode: showing cached and fallback station data.';
  }

  init();
})();
