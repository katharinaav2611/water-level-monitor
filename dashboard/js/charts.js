(function () {
  class DashboardCharts {
    constructor() {
      this.consumptionChart = this.createConsumptionChart();
      this.historyChart = this.createHistoryChart();
      this.usageChart = this.createUsageChart();
    }

    createConsumptionChart() {
      const element = document.getElementById('consumption-chart');
      return new Chart(element, {
        type: 'bar',
        data: {
          labels: [],
          datasets: [{
            label: 'Daily consumption (L)',
            data: [],
            backgroundColor: '#3a7ee0'
          }]
        },
        options: {
          responsive: true,
          maintainAspectRatio: false
        }
      });
    }

    createHistoryChart() {
      const element = document.getElementById('history-chart');
      return new Chart(element, {
        type: 'line',
        data: {
          labels: [],
          datasets: []
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          interaction: { mode: 'index', intersect: false },
          scales: {
            y: {
              min: 0,
              max: 100,
              title: { display: true, text: 'Water level %' }
            }
          }
        }
      });
    }

    createUsageChart() {
      const element = document.getElementById('usage-chart');
      return new Chart(element, {
        type: 'doughnut',
        data: {
          labels: [],
          datasets: [{
            data: [],
            backgroundColor: ['#3a7ee0', '#45aaf2', '#00b894', '#fdcb6e', '#e17055', '#6c5ce7']
          }]
        },
        options: {
          responsive: true,
          maintainAspectRatio: false
        }
      });
    }

    update(stations, stationHistory) {
      this.updateConsumption(stations);
      this.updateHistory(stations, stationHistory);
      this.updateUsage(stations);
    }

    updateConsumption(stations) {
      this.consumptionChart.data.labels = stations.map((station) => station.stationName);
      this.consumptionChart.data.datasets[0].data = stations.map((station) => station.dailyConsumption);
      this.consumptionChart.update();
    }

    updateHistory(stations, stationHistory) {
      const firstSeries = stationHistory.get(stations[0]?.stationId) || [];
      this.historyChart.data.labels = firstSeries.map((point) => point.label);
      this.historyChart.data.datasets = stations.slice(0, 4).map((station, index) => {
        const palette = ['#0064d8', '#0f9d58', '#f4b400', '#d93025'];
        return {
          label: station.stationName,
          data: (stationHistory.get(station.stationId) || []).map((point) => point.value),
          borderColor: palette[index],
          fill: false,
          tension: 0.25
        };
      });
      this.historyChart.update();
    }

    updateUsage(stations) {
      const ranked = [...stations].sort((a, b) => b.totalRefills - a.totalRefills).slice(0, 6);
      this.usageChart.data.labels = ranked.map((station) => station.stationName);
      this.usageChart.data.datasets[0].data = ranked.map((station) => station.totalRefills);
      this.usageChart.update();
    }
  }

  window.DashboardCharts = DashboardCharts;
})();
