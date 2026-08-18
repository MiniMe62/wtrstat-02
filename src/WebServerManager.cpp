#include "WebServerManager.h"

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="sk">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>wtrStat-02 • Meteostanica</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap');
        
        :root {
            --bg-gradient: linear-gradient(135deg, #0b1329 0%, #101c3d 50%, #0d1527 100%);
            --card-bg: rgba(22, 33, 62, 0.65);
            --card-border: rgba(255, 255, 255, 0.08);
            --primary: #38bdf8;
            --primary-glow: rgba(56, 189, 248, 0.25);
            --accent: #f43f5e;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --success: #34d399;
            --table-row-even: rgba(255, 255, 255, 0.02);
            --table-row-hover: rgba(56, 189, 248, 0.08);
        }

        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Inter', system-ui, -apple-system, sans-serif; }
        
        body {
            background: var(--bg-gradient);
            color: var(--text-main);
            min-height: 100vh;
            padding: 24px 16px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .container {
            width: 100%;
            max-width: 960px;
        }

        /* Header */
        header {
            display: flex;
            flex-wrap: wrap;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 28px;
            background: var(--card-bg);
            backdrop-filter: blur(16px);
            border: 1px solid var(--card-border);
            padding: 20px 24px;
            border-radius: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }

        .brand h1 {
            font-size: 1.6rem;
            font-weight: 700;
            background: linear-gradient(135deg, #38bdf8 0%, #818cf8 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .brand p {
            color: var(--text-muted);
            font-size: 0.85rem;
            margin-top: 2px;
        }

        .header-meta {
            display: flex;
            flex-direction: column;
            align-items: flex-end;
            gap: 6px;
        }

        .status-badge {
            display: flex;
            align-items: center;
            gap: 8px;
            background: rgba(52, 211, 153, 0.12);
            color: var(--success);
            border: 1px solid rgba(52, 211, 153, 0.3);
            padding: 4px 14px;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: 600;
        }

        .pulse-dot {
            width: 8px;
            height: 8px;
            background-color: var(--success);
            border-radius: 50%;
            box-shadow: 0 0 10px var(--success);
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(52, 211, 153, 0.7); }
            70% { transform: scale(1); box-shadow: 0 0 0 8px rgba(52, 211, 153, 0); }
            100% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(52, 211, 153, 0); }
        }

        .time-display {
            font-size: 0.9rem;
            color: var(--primary);
            font-weight: 600;
            letter-spacing: 0.5px;
        }

        /* Metric Cards Grid */
        .cards-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 16px;
            margin-bottom: 28px;
        }

        .card {
            background: var(--card-bg);
            backdrop-filter: blur(12px);
            border: 1px solid var(--card-border);
            border-radius: 18px;
            padding: 20px;
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            box-shadow: 0 8px 24px rgba(0,0,0,0.25);
            transition: all 0.25s ease;
        }

        .card:hover {
            transform: translateY(-3px);
            border-color: rgba(56, 189, 248, 0.3);
            box-shadow: 0 12px 30px var(--primary-glow);
        }

        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            color: var(--text-muted);
            font-size: 0.8rem;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.8px;
        }

        .card-val {
            font-size: 2.2rem;
            font-weight: 700;
            color: var(--text-main);
            margin: 12px 0 6px 0;
        }

        .unit {
            font-size: 1rem;
            font-weight: 400;
            color: var(--text-muted);
        }

        .card-sub {
            font-size: 0.78rem;
            color: var(--text-muted);
        }

        .compass-box {
            display: flex;
            align-items: center;
            justify-content: space-around;
            margin: 8px 0;
        }

        .compass-dial {
            width: 54px;
            height: 54px;
            border-radius: 50%;
            border: 2px dashed var(--primary);
            display: flex;
            align-items: center;
            justify-content: center;
            background: rgba(56, 189, 248, 0.05);
        }

        .compass-arrow {
            font-size: 1.4rem;
            color: var(--accent);
            transition: transform 0.6s cubic-bezier(0.34, 1.56, 0.64, 1);
        }

        /* Section Table */
        .section-box {
            background: var(--card-bg);
            backdrop-filter: blur(16px);
            border: 1px solid var(--card-border);
            border-radius: 20px;
            padding: 24px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }

        .section-title {
            font-size: 1.1rem;
            font-weight: 600;
            color: var(--text-main);
            margin-bottom: 16px;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .table-responsive {
            overflow-x: auto;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            text-align: left;
            font-size: 0.9rem;
        }

        th {
            background: rgba(255, 255, 255, 0.04);
            color: var(--text-muted);
            font-weight: 600;
            text-transform: uppercase;
            font-size: 0.75rem;
            letter-spacing: 0.8px;
            padding: 14px 18px;
            border-bottom: 1px solid var(--card-border);
        }

        th:first-child { border-top-left-radius: 12px; border-bottom-left-radius: 12px; }
        th:last-child { border-top-right-radius: 12px; border-bottom-right-radius: 12px; }

        td {
            padding: 14px 18px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.04);
            color: var(--text-main);
        }

        tr:last-child td { border-bottom: none; }
        tr:nth-child(even) { background: var(--table-row-even); }
        tr:hover { background: var(--table-row-hover); }

        .val-highlight {
            font-weight: 600;
            color: var(--primary);
        }

        /* Wind Rose Chart Section */
        .chart-box {
            margin-top: 28px;
            background: var(--card-bg);
            backdrop-filter: blur(16px);
            border: 1px solid var(--card-border);
            border-radius: 20px;
            padding: 24px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }

        .chart-controls {
            display: flex;
            flex-wrap: wrap;
            justify-content: space-between;
            align-items: center;
            gap: 12px;
            margin: 12px 0 16px 0;
            background: rgba(255, 255, 255, 0.02);
            padding: 10px 14px;
            border-radius: 14px;
            border: 1px solid var(--card-border);
        }

        .select-custom {
            background: #0f172a;
            color: var(--text-main);
            border: 1px solid var(--card-border);
            padding: 6px 12px;
            border-radius: 10px;
            font-size: 0.82rem;
            outline: none;
            cursor: pointer;
        }

        .select-custom:focus {
            border-color: var(--primary);
        }

        .btn-group {
            display: flex;
            flex-wrap: wrap;
            gap: 6px;
        }

        .btn-period {
            background: rgba(255, 255, 255, 0.05);
            color: var(--text-muted);
            border: 1px solid var(--card-border);
            padding: 6px 12px;
            border-radius: 10px;
            font-size: 0.78rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s ease;
        }

        .btn-period:hover {
            color: var(--text-main);
            border-color: rgba(56, 189, 248, 0.4);
        }

        .btn-period.active {
            background: rgba(56, 189, 248, 0.18);
            color: var(--primary);
            border-color: var(--primary);
            box-shadow: 0 0 12px var(--primary-glow);
        }

        .chart-container-wrapper {
            position: relative;
            width: 100%;
            max-width: 440px;
            margin: 16px auto;
            aspect-ratio: 1 / 1;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .chart-line-wrapper {
            position: relative;
            width: 100%;
            height: 320px;
            margin: 16px 0;
        }

        .btn-view {
            background: rgba(255, 255, 255, 0.05);
            color: var(--text-muted);
            border: 1px solid var(--card-border);
            padding: 6px 12px;
            border-radius: 10px;
            font-size: 0.78rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s ease;
        }

        .btn-view:hover {
            color: var(--text-main);
            border-color: rgba(251, 113, 133, 0.4);
        }

        .btn-view.active {
            background: rgba(251, 113, 133, 0.18);
            color: #fb7185;
            border-color: #fb7185;
            box-shadow: 0 0 12px rgba(251, 113, 133, 0.25);
        }

        .chart-stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(130px, 1fr));
            gap: 12px;
            margin-top: 18px;
        }

        .stat-badge {
            background: rgba(255, 255, 255, 0.03);
            border: 1px solid var(--card-border);
            padding: 12px 14px;
            border-radius: 14px;
            text-align: center;
            display: flex;
            flex-direction: column;
            gap: 4px;
            transition: all 0.2s ease;
        }

        .stat-badge:hover {
            border-color: rgba(56, 189, 248, 0.3);
            background: rgba(56, 189, 248, 0.05);
        }

        .stat-badge-lbl {
            font-size: 0.72rem;
            color: var(--text-muted);
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.6px;
        }

        .stat-badge-val {
            font-size: 1.15rem;
            font-weight: 700;
            color: var(--primary);
        }

        .chart-header-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 4px;
        }

        .chart-header-row .section-title {
            margin-bottom: 0;
        }

        .btn-bubble-toggle {
            display: inline-flex;
            align-items: center;
            gap: 6px;
            background: rgba(255, 255, 255, 0.06);
            color: var(--text-muted);
            border: 1px solid var(--card-border);
            padding: 6px 12px;
            border-radius: 20px;
            font-size: 0.78rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s ease;
        }

        .btn-bubble-toggle:hover {
            color: var(--text-main);
            border-color: rgba(56, 189, 248, 0.4);
        }

        .btn-bubble-toggle.active {
            background: rgba(56, 189, 248, 0.15);
            color: var(--primary);
            border-color: var(--primary);
            box-shadow: 0 0 10px var(--primary-glow);
        }

        .btn-bubble-toggle span {
            font-weight: 700;
            font-size: 0.72rem;
            padding: 2px 7px;
            border-radius: 10px;
            background: rgba(0, 0, 0, 0.35);
            color: #94a3b8;
        }

        .btn-bubble-toggle.active span {
            background: var(--primary);
            color: #0b1329;
        }

        @media (max-width: 640px) {
            .container {
                padding: 14px 10px;
            }
            .cards-grid {
                grid-template-columns: 1fr;
            }
            .chart-box {
                padding: 16px 12px;
                border-radius: 16px;
            }
            .chart-controls {
                flex-direction: column;
                align-items: stretch;
                padding: 10px 8px;
                gap: 8px;
            }
            .btn-group {
                width: 100%;
                display: flex;
                flex-wrap: wrap;
                gap: 4px;
            }
            .btn-view, .btn-period {
                flex: 1 1 auto;
                padding: 6px 6px;
                font-size: 0.74rem;
                text-align: center;
            }
            .chart-stats-grid {
                grid-template-columns: 1fr 1fr;
                gap: 8px;
            }
            .stat-badge {
                padding: 8px 10px;
            }
            .stat-badge-val {
                font-size: 1rem;
            }
        }

        footer {
            margin-top: 32px;
            text-align: center;
            font-size: 0.8rem;
            color: var(--text-muted);
        }
    </style>
    <!-- Chart.js CDN pre Veternú Ružicu -->
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
    <div class="container">
        <header>
            <div class="brand">
                <h1>🌤️ wtrStat-02</h1>
                <p>Meteorologická stanica ESP32 • Lokálny Dashboard</p>
            </div>
            <div class="header-meta">
                <div class="status-badge">
                    <div class="pulse-dot"></div>
                    <span>ONLINE</span>
                </div>
                <div class="time-display" id="timestamp">--:--:--</div>
            </div>
        </header>

        <!-- Metric Cards -->
        <div class="cards-grid">
            <div class="card">
                <div class="card-header">Vnútorná Teplota <span>Tin</span></div>
                <div class="card-val" id="tempIn">-- <span class="unit">°C</span></div>
                <div class="card-sub">DS18B20 #1 (krok 0.25°C)</div>
            </div>

            <div class="card">
                <div class="card-header">Vonkajšia Teplota <span>Tout</span></div>
                <div class="card-val" id="tempOut">-- <span class="unit">°C</span></div>
                <div class="card-sub">DS18B20 #2 (krok 0.25°C)</div>
            </div>

            <div class="card">
                <div class="card-header">Rýchlosť Vetra <span>Hall</span></div>
                <div class="card-val" id="windSpeed">-- <span class="unit">m/s</span></div>
                <div class="card-sub">60s kĺzavý priemer</div>
            </div>

            <div class="card">
                <div class="card-header">Smer Vetra <span id="windDirNameBadge">--</span></div>
                <div class="compass-box">
                    <div class="compass-dial">
                        <div class="compass-arrow" id="arrow">↑</div>
                    </div>
                    <div>
                        <div class="card-val" style="margin:0; font-size:2.2rem; font-weight:700;" id="windDirNameText">--</div>
                        <div class="card-sub" style="font-size:0.95rem; font-weight:600; color:var(--primary);" id="windDirDeg">--°</div>
                    </div>
                </div>
            </div>
        </div>

        <!-- Detailed Table -->
        <div class="section-box">
            <div class="section-title">📊 Podrobný Prehľad Nameraných Hodnôt</div>
            <div class="table-responsive">
                <table>
                    <thead>
                        <tr>
                            <th>Veličina / Parameter</th>
                            <th>Nameraná Hodnota</th>
                            <th>Jednotka</th>
                            <th>Typ Senzora / Zbernica</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td>Vnútorná teplota (Tin)</td>
                            <td class="val-highlight" id="tblTempIn">--</td>
                            <td>°C</td>
                            <td>Dallas DS18B20 (GPIO 14)</td>
                        </tr>
                        <tr>
                            <td>Vonkajšia teplota (Tout)</td>
                            <td class="val-highlight" id="tblTempOut">--</td>
                            <td>°C</td>
                            <td>Dallas DS18B20 (GPIO 14)</td>
                        </tr>
                        <tr>
                            <td>Rýchlosť vetra</td>
                            <td class="val-highlight" id="tblWindSpeed">--</td>
                            <td>m/s</td>
                            <td>Anemometer Hall ISR (GPIO 18)</td>
                        </tr>
                        <tr>
                            <td>Smer vetra (Azimut)</td>
                            <td class="val-highlight" id="tblWindDirDeg">--</td>
                            <td>° (Stupne)</td>
                            <td>WindVane ADC delič (GPIO 34)</td>
                        </tr>
                        <tr>
                            <td>Smer vetra (Názov)</td>
                            <td class="val-highlight" id="tblWindDirName">--</td>
                            <td>Sektor</td>
                            <td>Goniometrický priemer</td>
                        </tr>
                        <tr>
                            <td>Pripojená WiFi sieť</td>
                            <td class="val-highlight" id="tblWifiSSID">--</td>
                            <td>SSID</td>
                            <td>Auto-select najsilnejšej siete</td>
                        </tr>
                        <tr>
                            <td>Sila WiFi signálu</td>
                            <td class="val-highlight" id="tblWifiRSSI">--</td>
                            <td>dBm</td>
                            <td>ESP32 WiFi Station</td>
                        </tr>
                        <tr>
                            <td>IP Adresa stanice</td>
                            <td class="val-highlight" id="tblIp">--</td>
                            <td>IPv4</td>
                            <td>DHCP Server</td>
                        </tr>
                        <tr>
                            <td>Čas chodu (Uptime)</td>
                            <td class="val-highlight" id="tblUptime">--</td>
                            <td>sekund</td>
                            <td>System millis()</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </div>

        <!-- Temperature History Section -->
        <div class="chart-box">
            <div class="chart-header-row">
                <div class="section-title">📈 História a Priebeh Teplôt</div>
                <button class="btn-bubble-toggle active" id="btnToggleTempTooltip" onclick="toggleTempTooltip()" title="Zapnúť / Vypnúť bubliny">
                    💬 Bubliny <span id="lblTempTooltip">ZAP</span>
                </button>
            </div>
            <p style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 8px;">
                Priebeh vnútornej a vonkajšej teploty s automatickým prispôsobením citlivosti škály.
            </p>

            <!-- Control Bar (View Mode & Timeframe Selector) -->
            <div class="chart-controls">
                <div class="btn-group">
                    <button class="btn-view active" id="btnTempBoth" onclick="setTempView('both')">🌡️ Obe Teploty</button>
                    <button class="btn-view" id="btnTempIn" onclick="setTempView('in')">🏠 Len Vnútorná (Tin)</button>
                    <button class="btn-view" id="btnTempOut" onclick="setTempView('out')">🌲 Len Vonkajšia (Tout)</button>
                </div>

                <div class="btn-group">
                    <button class="btn-period active" id="btnTempPeriodLive" onclick="setTempPeriod('live')">⚡ Živé</button>
                    <button class="btn-period" id="btnTempPeriod24h" onclick="setTempPeriod('24h')">📅 24h</button>
                    <button class="btn-period" id="btnTempPeriod3d" onclick="setTempPeriod('3d')">📆 3d</button>
                    <button class="btn-period" id="btnTempPeriod7d" onclick="setTempPeriod('7d')">🗓️ 7d</button>
                </div>
            </div>

            <div id="tempChartLoading" style="display: none; font-size: 0.82rem; color: var(--primary); text-align: center; margin: 8px 0; font-weight: 500;">
                ⏳ Načítavam dáta teplôt z ThingSpeak API...
            </div>
            <div id="tempChartNotice" style="display: none; font-size: 0.82rem; color: #fbbf24; text-align: center; margin: 8px 0; background: rgba(251, 191, 36, 0.1); padding: 6px 12px; border-radius: 8px;">
            </div>

            <div class="chart-line-wrapper">
                <canvas id="tempLineChart"></canvas>
            </div>
            
            <div class="chart-stats-grid">
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Vnútorná (Min / Max)</span>
                    <span class="stat-badge-val" id="statTempInMinMax" style="color: #fb7185;">-- / -- °C</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Vonkajšia (Min / Max)</span>
                    <span class="stat-badge-val" id="statTempOutMinMax" style="color: #38bdf8;">-- / -- °C</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Rozdiel (Tout - Tin)</span>
                    <span class="stat-badge-val" id="statTempDiff">-- °C</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Priemer (Tin / Tout)</span>
                    <span class="stat-badge-val" id="statTempAvg">-- / -- °C</span>
                </div>
            </div>
        </div>

        <!-- Wind Rose Section -->
        <div class="chart-box">
            <div class="chart-header-row">
                <div class="section-title">🌪️ Veterná Ružica</div>
                <button class="btn-bubble-toggle active" id="btnToggleRoseTooltip" onclick="toggleRoseTooltip()" title="Zapnúť / Vypnúť bubliny">
                    💬 Bubliny <span id="lblRoseTooltip">ZAP</span>
                </button>
            </div>
            <p style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 8px;">
                Graf zobrazuje smerovú frekvenciu a intenzitu prúdenia vetra v 16 sektoroch.
            </p>

            <!-- Control Bar (Station & Timeframe Selector) -->
            <div class="chart-controls">
                <div style="display: flex; align-items: center; gap: 8px;">
                    <span style="font-size: 0.78rem; font-weight: 600; color: var(--text-muted); text-transform: uppercase;">Stanica:</span>
                    <select id="selStation" class="select-custom" onchange="onStationOrPeriodChange()">
                        <option value="3205571" selected>TEST (TestESP32Meteo)</option>
                        <option value="1554841">GO85 (Gombos Vidiek)</option>
                        <option value="287161">RU48 (MiniMe)</option>
                    </select>
                </div>

                <div class="btn-group">
                    <button class="btn-period active" id="btnLive" onclick="setPeriod('live')">⚡ Živé</button>
                    <button class="btn-period" id="btn24h" onclick="setPeriod('24h')">📅 24h</button>
                    <button class="btn-period" id="btn3d" onclick="setPeriod('3d')">📆 3d</button>
                    <button class="btn-period" id="btn7d" onclick="setPeriod('7d')">🗓️ 7d</button>
                </div>
            </div>

            <div id="chartLoading" style="display: none; font-size: 0.82rem; color: var(--primary); text-align: center; margin: 8px 0; font-weight: 500;">
                ⏳ Načítavam dáta z ThingSpeak API...
            </div>
            <div id="chartNotice" style="display: none; font-size: 0.82rem; color: #fbbf24; text-align: center; margin: 8px 0; background: rgba(251, 191, 36, 0.1); padding: 6px 12px; border-radius: 8px;">
            </div>

            <div class="chart-container-wrapper">
                <canvas id="windRoseChart"></canvas>
            </div>
            
            <div class="chart-stats-grid">
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Dominantný Smer</span>
                    <span class="stat-badge-val" id="statDomDir">--</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Max Rýchlosť</span>
                    <span class="stat-badge-val" id="statMaxSpeed">-- m/s</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Priemerná Rýchlosť</span>
                    <span class="stat-badge-val" id="statAvgSpeed">-- m/s</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Počet Meraní</span>
                    <span class="stat-badge-val" id="statSamplesCount">0</span>
                </div>
            </div>
        </div>

        <footer>
            wtrStat-02 • ESP32 Weather Station Firmware • PlatformIO C++
        </footer>
    </div>

    <script>
        // 16-smerové svetové strany
        const ROSE_DIRS = ["S", "SSV", "SV", "VSV", "V", "VJV", "JV", "JJV", "J", "JJZ", "JZ", "ZJZ", "Z", "ZSZ", "SZ", "SSZ"];
        
        // ThingSpeak konfigurácia pre stanice
        const TS_CONFIG = {
            "3205571": { key: "SXG8MK33NKFEA0UX", name: "TEST" },
            "1554841": { key: "M4SJ7BSW2LR4WQVD", name: "GO85" },
            "287161": { key: "WEO05BAL45Y3E52D", name: "RU48" }
        };

        // ================= TEPLOTNÝ GRAF =================
        let tempViewMode = 'both'; // 'both' | 'in' | 'out'
        let tempPeriod = 'live';   // 'live' | '24h' | '3d' | '7d'

        let liveTempLabels = ['08:00', '08:30', '09:00', '09:30', '10:00', '10:30', '11:00', '11:30', '12:00', '12:30', '13:00', '13:15'];
        let liveTempIn = [22.50, 22.50, 22.75, 23.00, 23.00, 23.25, 23.50, 23.50, 23.75, 23.75, 24.00, 24.00];
        let liveTempOut = [19.25, 20.50, 22.00, 23.75, 25.50, 27.00, 28.50, 29.50, 30.75, 31.50, 31.00, 30.50];

        let activeTempLabels = [...liveTempLabels];
        let activeTempIn = [...liveTempIn];
        let activeTempOut = [...liveTempOut];

        let tempLineChartInstance = null;

        function initTempLineChart() {
            const ctx = document.getElementById('tempLineChart').getContext('2d');

            tempLineChartInstance = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: activeTempLabels,
                    datasets: [
                        {
                            label: 'Vnútorná (Tin)',
                            data: activeTempIn,
                            borderColor: '#fb7185',
                            backgroundColor: 'rgba(251, 113, 133, 0.12)',
                            fill: true,
                            tension: 0.35,
                            borderWidth: 2.2,
                            pointRadius: 2,
                            pointHoverRadius: 6,
                            pointBackgroundColor: '#fb7185'
                        },
                        {
                            label: 'Vonkajšia (Tout)',
                            data: activeTempOut,
                            borderColor: '#38bdf8',
                            backgroundColor: 'rgba(56, 189, 248, 0.12)',
                            fill: true,
                            tension: 0.35,
                            borderWidth: 2.2,
                            pointRadius: 2,
                            pointHoverRadius: 6,
                            pointBackgroundColor: '#38bdf8'
                        }
                    ]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    interaction: {
                        mode: 'index',
                        intersect: false
                    },
                    animation: {
                        duration: 500,
                        easing: 'easeOutQuart'
                    },
                    scales: {
                        x: {
                            grid: {
                                color: 'rgba(255, 255, 255, 0.05)'
                            },
                            ticks: {
                                color: '#94a3b8',
                                font: { size: 10, family: 'Inter' },
                                maxRotation: 0,
                                autoSkip: true,
                                maxTicksLimit: 10
                            }
                        },
                        y: {
                            grace: '5%',
                            grid: {
                                color: 'rgba(255, 255, 255, 0.06)'
                            },
                            ticks: {
                                color: '#94a3b8',
                                font: { size: 11, family: 'Inter' },
                                callback: function(val) {
                                    return val.toFixed(1) + ' °C';
                                }
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            display: true,
                            position: 'top',
                            align: 'end',
                            labels: {
                                boxWidth: 12,
                                color: '#94a3b8',
                                font: { size: 11, family: 'Inter', weight: '600' }
                            }
                        },
                        tooltip: {
                            backgroundColor: 'rgba(15, 23, 42, 0.92)',
                            titleColor: '#38bdf8',
                            titleFont: { weight: '700', size: 12 },
                            bodyFont: { size: 12 },
                            borderColor: 'rgba(56, 189, 248, 0.3)',
                            borderWidth: 1,
                            padding: 10,
                            callbacks: {
                                label: function(context) {
                                    return ` ${context.dataset.label}: ${parseFloat(context.parsed.y).toFixed(2)} °C`;
                                }
                            }
                        }
                    }
                }
            });

            updateTempStats();
        }

        function updateTempStats() {
            const validIn = activeTempIn.filter(v => v !== null && !isNaN(v) && v > -50);
            const validOut = activeTempOut.filter(v => v !== null && !isNaN(v) && v > -50);

            if (validIn.length > 0) {
                const minIn = Math.min(...validIn).toFixed(2);
                const maxIn = Math.max(...validIn).toFixed(2);
                const avgIn = (validIn.reduce((a, b) => a + b, 0) / validIn.length).toFixed(2);
                document.getElementById('statTempInMinMax').innerText = `${minIn} / ${maxIn} °C`;
                document.getElementById('statTempAvg').innerText = `${avgIn} / ${(validOut.length > 0 ? (validOut.reduce((a, b) => a + b, 0) / validOut.length).toFixed(2) : '--')} °C`;
            } else {
                document.getElementById('statTempInMinMax').innerText = `-- / -- °C`;
            }

            if (validOut.length > 0) {
                const minOut = Math.min(...validOut).toFixed(2);
                const maxOut = Math.max(...validOut).toFixed(2);
                document.getElementById('statTempOutMinMax').innerText = `${minOut} / ${maxOut} °C`;
            } else {
                document.getElementById('statTempOutMinMax').innerText = `-- / -- °C`;
            }

            if (validIn.length > 0 && validOut.length > 0) {
                const lastIn = validIn[validIn.length - 1];
                const lastOut = validOut[validOut.length - 1];
                const diff = (lastOut - lastIn);
                document.getElementById('statTempDiff').innerText = `${diff >= 0 ? '+' : ''}${diff.toFixed(2)} °C`;
            } else {
                document.getElementById('statTempDiff').innerText = `-- °C`;
            }
        }

        function setTempView(view) {
            tempViewMode = view;

            ['btnTempBoth', 'btnTempIn', 'btnTempOut'].forEach(id => {
                document.getElementById(id).classList.remove('active');
            });

            if (view === 'both') document.getElementById('btnTempBoth').classList.add('active');
            if (view === 'in') document.getElementById('btnTempIn').classList.add('active');
            if (view === 'out') document.getElementById('btnTempOut').classList.add('active');

            if (tempLineChartInstance) {
                if (view === 'both') {
                    tempLineChartInstance.setDatasetVisibility(0, true);
                    tempLineChartInstance.setDatasetVisibility(1, true);
                } else if (view === 'in') {
                    tempLineChartInstance.setDatasetVisibility(0, true);
                    tempLineChartInstance.setDatasetVisibility(1, false);
                } else if (view === 'out') {
                    tempLineChartInstance.setDatasetVisibility(0, false);
                    tempLineChartInstance.setDatasetVisibility(1, true);
                }
                tempLineChartInstance.update();
            }
        }

        let tempTooltipEnabled = true;
        function toggleTempTooltip() {
            tempTooltipEnabled = !tempTooltipEnabled;
            const btn = document.getElementById('btnToggleTempTooltip');
            const lbl = document.getElementById('lblTempTooltip');
            if (btn) btn.classList.toggle('active', tempTooltipEnabled);
            if (lbl) lbl.innerText = tempTooltipEnabled ? 'ZAP' : 'VYP';
            if (tempLineChartInstance) {
                tempLineChartInstance.options.plugins.tooltip.enabled = tempTooltipEnabled;
                tempLineChartInstance.update('none');
            }
        }

        let roseTooltipEnabled = true;
        function toggleRoseTooltip() {
            roseTooltipEnabled = !roseTooltipEnabled;
            const btn = document.getElementById('btnToggleRoseTooltip');
            const lbl = document.getElementById('lblRoseTooltip');
            if (btn) btn.classList.toggle('active', roseTooltipEnabled);
            if (lbl) lbl.innerText = roseTooltipEnabled ? 'ZAP' : 'VYP';
            if (windRoseChartInstance) {
                windRoseChartInstance.options.plugins.tooltip.enabled = roseTooltipEnabled;
                windRoseChartInstance.update('none');
            }
        }

        function setTempPeriod(period) {
            tempPeriod = period;

            ['btnTempPeriodLive', 'btnTempPeriod24h', 'btnTempPeriod3d', 'btnTempPeriod7d'].forEach(id => {
                document.getElementById(id).classList.remove('active');
            });

            if (period === 'live') document.getElementById('btnTempPeriodLive').classList.add('active');
            if (period === '24h') document.getElementById('btnTempPeriod24h').classList.add('active');
            if (period === '3d') document.getElementById('btnTempPeriod3d').classList.add('active');
            if (period === '7d') document.getElementById('btnTempPeriod7d').classList.add('active');

            document.getElementById('tempChartNotice').style.display = 'none';

            if (period === 'live') {
                document.getElementById('tempChartLoading').style.display = 'none';
                activeTempLabels = [...liveTempLabels];
                activeTempIn = [...liveTempIn];
                activeTempOut = [...liveTempOut];
                renderTempChart();
            } else {
                let resultsCount = 96; // 24h
                if (period === '3d') resultsCount = 288;
                if (period === '7d') resultsCount = 672;
                fetchThingSpeakTempHistory(resultsCount);
            }
        }

        function renderTempChart() {
            if (!tempLineChartInstance) return;

            tempLineChartInstance.data.labels = activeTempLabels;
            tempLineChartInstance.data.datasets[0].data = activeTempIn;
            tempLineChartInstance.data.datasets[1].data = activeTempOut;

            tempLineChartInstance.update();
            updateTempStats();
        }

        async function fetchThingSpeakTempHistory(resultsCount) {
            const chanId = document.getElementById('selStation').value;
            const cfg = TS_CONFIG[chanId];
            if (!cfg) return;

            const loadingEl = document.getElementById('tempChartLoading');
            const noticeEl = document.getElementById('tempChartNotice');
            loadingEl.style.display = 'block';
            noticeEl.style.display = 'none';

            const url = `https://api.thingspeak.com/channels/${chanId}/feeds.json?api_key=${cfg.key}&results=${resultsCount}`;

            try {
                const res = await fetch(url);
                if (!res.ok) throw new Error("Chyba odpovede z ThingSpeak API");
                const data = await res.json();

                if (!data.feeds || data.feeds.length === 0) {
                    throw new Error("Žiadne záznamy.");
                }

                let labels = [];
                let tinSeries = [];
                let toutSeries = [];

                for (const feed of data.feeds) {
                    if (!feed.created_at) continue;
                    
                    const d = new Date(feed.created_at);
                    let label = d.toLocaleTimeString('sk-SK', { hour: '2-digit', minute: '2-digit' });
                    if (resultsCount > 96) {
                        label = `${d.getDate()}.${d.getMonth() + 1}. ` + label;
                    }

                    let tIn = (feed.field1 !== null && feed.field1 !== undefined && feed.field1 !== "") ? parseFloat(feed.field1) : null;
                    let tOut = (feed.field2 !== null && feed.field2 !== undefined && feed.field2 !== "") ? parseFloat(feed.field2) : null;

                    // Plauzibilný filter teplôt: -40.0°C až +60.0°C (odfiltrovať -127.0°C a +85.0°C)
                    if (tIn !== null && (isNaN(tIn) || tIn < -40.0 || tIn > 60.0 || Math.abs(tIn - 85.0) < 0.1 || Math.abs(tIn + 127.0) < 0.1)) {
                        tIn = null;
                    }
                    if (tOut !== null && (isNaN(tOut) || tOut < -40.0 || tOut > 60.0 || Math.abs(tOut - 85.0) < 0.1 || Math.abs(tOut + 127.0) < 0.1)) {
                        tOut = null;
                    }

                    labels.push(label);
                    tinSeries.push(tIn);
                    toutSeries.push(tOut);
                }

                loadingEl.style.display = 'none';

                if (labels.length === 0) {
                    noticeEl.innerText = `ℹ️ Stanica ${cfg.name} nemá v zvolenom období záznamy o teplote.`;
                    noticeEl.style.display = 'block';
                    return;
                }

                activeTempLabels = labels;
                activeTempIn = tinSeries;
                activeTempOut = toutSeries;
                renderTempChart();

            } catch (err) {
                loadingEl.style.display = 'none';
                noticeEl.innerText = `⚠️ Nepodarilo sa načítať teploty: ${err.message}`;
                noticeEl.style.display = 'block';
            }
        }

        function addLiveTempSample(timeLabel, tIn, tOut) {
            // Plauzibilný filter pre živé dáta
            const validIn = (tIn !== null && !isNaN(tIn) && tIn >= -40.0 && tIn <= 60.0 && Math.abs(tIn - 85.0) > 0.1 && Math.abs(tIn + 127.0) > 0.1) ? tIn : null;
            const validOut = (tOut !== null && !isNaN(tOut) && tOut >= -40.0 && tOut <= 60.0 && Math.abs(tOut - 85.0) > 0.1 && Math.abs(tOut + 127.0) > 0.1) ? tOut : null;

            liveTempLabels.push(timeLabel);
            liveTempIn.push(validIn);
            liveTempOut.push(validOut);

            if (liveTempLabels.length > 25) {
                liveTempLabels.shift();
                liveTempIn.shift();
                liveTempOut.shift();
            }

            if (tempPeriod === 'live') {
                activeTempLabels = [...liveTempLabels];
                activeTempIn = [...liveTempIn];
                activeTempOut = [...liveTempOut];
                renderTempChart();
            }
        }

        // ================= VETERNÁ RUŽICA =================
        let currentPeriod = 'live'; // 'live' | '24h' | '3d' | '7d'

        // Živá relácia (akumulátor)
        let liveCounts = [4, 6, 8, 14, 18, 12, 6, 3, 2, 4, 7, 10, 15, 22, 19, 9];
        let liveSpeedSums = [8.2, 14.5, 20.0, 39.2, 54.0, 31.2, 12.0, 5.4, 3.6, 7.8, 15.4, 26.0, 48.0, 79.2, 62.7, 23.4];
        let liveTotalSamples = 169;
        let liveMaxSpeed = 5.8;

        // Aktuálne zobrazovaný dataset (pre graf)
        let activeCounts = [...liveCounts];
        let activeSpeedSums = [...liveSpeedSums];
        let activeTotalSamples = liveTotalSamples;
        let activeMaxSpeed = liveMaxSpeed;

        let windRoseChartInstance = null;

        function initWindRoseChart() {
            const ctx = document.getElementById('windRoseChart').getContext('2d');
            const percentages = activeCounts.map(c => ((c / activeTotalSamples) * 100).toFixed(1));
            const maxCount = Math.max(...activeCounts, 1);
            const domIndex = activeCounts.indexOf(Math.max(...activeCounts));

            const bgColors = activeCounts.map((c, i) => {
                if (i === domIndex && c > 0) return 'rgba(244, 63, 94, 0.75)';
                const intensity = (c / maxCount) * 0.5 + 0.15;
                return `rgba(56, 189, 248, ${intensity.toFixed(2)})`;
            });

            const borderColors = activeCounts.map((c, i) => {
                return (i === domIndex && c > 0) ? '#f43f5e' : '#38bdf8';
            });

            windRoseChartInstance = new Chart(ctx, {
                type: 'polarArea',
                data: {
                    labels: ROSE_DIRS,
                    datasets: [{
                        label: 'Zastúpenie smeru (%)',
                        data: percentages,
                        backgroundColor: bgColors,
                        borderColor: borderColors,
                        borderWidth: 1.5
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    animation: {
                        duration: 600,
                        easing: 'easeOutQuart'
                    },
                    scales: {
                        r: {
                            startAngle: -11.25,
                            angleLines: {
                                color: 'rgba(255, 255, 255, 0.12)',
                                lineWidth: 1
                            },
                            grid: {
                                color: 'rgba(255, 255, 255, 0.06)'
                            },
                            pointLabels: {
                                display: true,
                                centerPointLabels: true,
                                font: {
                                    size: 11,
                                    weight: '600',
                                    family: 'Inter'
                                },
                                color: '#94a3b8'
                            },
                            ticks: {
                                display: false,
                                backdropColor: 'transparent'
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            display: false
                        },
                        tooltip: {
                            backgroundColor: 'rgba(15, 23, 42, 0.92)',
                            titleColor: '#38bdf8',
                            titleFont: { weight: '700', size: 13 },
                            bodyColor: '#f8fafc',
                            bodyFont: { size: 12 },
                            borderColor: 'rgba(56, 189, 248, 0.3)',
                            borderWidth: 1,
                            padding: 12,
                            boxPadding: 6,
                            callbacks: {
                                title: function(items) {
                                    return `Smer: ${items[0].label}`;
                                },
                                label: function(context) {
                                    const idx = context.dataIndex;
                                    const count = activeCounts[idx];
                                    const avg = count > 0 ? (activeSpeedSums[idx] / count).toFixed(1) : '0.0';
                                    return [
                                        ` Výskyt: ${count} vzoriek (${context.parsed.r}%)`,
                                        ` Priemerná rýchlosť: ${avg} m/s`
                                    ];
                                }
                            }
                        }
                    }
                }
            });

            updateWindRoseStats();
        }

        function updateWindRoseStats() {
            if (activeTotalSamples === 0) {
                document.getElementById('statDomDir').innerText = "--";
                document.getElementById('statMaxSpeed').innerText = "-- m/s";
                document.getElementById('statAvgSpeed').innerText = "-- m/s";
                document.getElementById('statSamplesCount').innerText = "0";
                return;
            }

            const maxCount = Math.max(...activeCounts);
            const domIndex = activeCounts.indexOf(maxCount);
            const domDir = ROSE_DIRS[domIndex];
            const domPct = ((maxCount / activeTotalSamples) * 100).toFixed(0);

            const totalSpeedSum = activeSpeedSums.reduce((a, b) => a + b, 0);
            const overallAvgSpeed = (totalSpeedSum / activeTotalSamples).toFixed(1);

            document.getElementById('statDomDir').innerText = `${domDir} (${domPct}%)`;
            document.getElementById('statMaxSpeed').innerText = `${activeMaxSpeed.toFixed(1)} m/s`;
            document.getElementById('statAvgSpeed').innerText = `${overallAvgSpeed} m/s`;
            document.getElementById('statSamplesCount').innerText = activeTotalSamples;
        }

        function renderActiveChart() {
            if (!windRoseChartInstance) return;

            const maxCount = Math.max(...activeCounts, 1);
            const domIndex = activeCounts.indexOf(Math.max(...activeCounts));
            const percentages = activeTotalSamples > 0 
                ? activeCounts.map(c => ((c / activeTotalSamples) * 100).toFixed(1))
                : new Array(16).fill(0);

            windRoseChartInstance.data.datasets[0].data = percentages;
            windRoseChartInstance.data.datasets[0].backgroundColor = activeCounts.map((c, i) => {
                if (i === domIndex && c > 0) return 'rgba(244, 63, 94, 0.75)';
                const intensity = (c / maxCount) * 0.5 + 0.15;
                return `rgba(56, 189, 248, ${intensity.toFixed(2)})`;
            });
            windRoseChartInstance.data.datasets[0].borderColor = activeCounts.map((c, i) => {
                return (i === domIndex && c > 0) ? '#f43f5e' : '#38bdf8';
            });

            windRoseChartInstance.update();
            updateWindRoseStats();
        }

        function addWindRoseSample(dirDeg, speed) {
            // Plauzibilný filter: smer 0..360, rýchlosť max 40 m/s
            if (dirDeg === undefined || isNaN(dirDeg) || dirDeg < 0 || dirDeg > 360) return;
            if (speed === undefined || isNaN(speed) || speed < 0 || speed > 40.0) return;

            const idx = Math.round(((dirDeg % 360) / 22.5)) % 16;
            
            liveCounts[idx]++;
            liveSpeedSums[idx] += speed;
            liveTotalSamples++;
            if (speed > liveMaxSpeed) {
                liveMaxSpeed = speed;
            }

            if (currentPeriod === 'live') {
                activeCounts = [...liveCounts];
                activeSpeedSums = [...liveSpeedSums];
                activeTotalSamples = liveTotalSamples;
                activeMaxSpeed = liveMaxSpeed;
                renderActiveChart();
            }
        }

        function setPeriod(period) {
            currentPeriod = period;
            
            ['btnLive', 'btn24h', 'btn3d', 'btn7d'].forEach(id => {
                document.getElementById(id).classList.remove('active');
            });
            
            if (period === 'live') document.getElementById('btnLive').classList.add('active');
            if (period === '24h') document.getElementById('btn24h').classList.add('active');
            if (period === '3d') document.getElementById('btn3d').classList.add('active');
            if (period === '7d') document.getElementById('btn7d').classList.add('active');

            document.getElementById('chartNotice').style.display = 'none';

            if (period === 'live') {
                document.getElementById('chartLoading').style.display = 'none';
                activeCounts = [...liveCounts];
                activeSpeedSums = [...liveSpeedSums];
                activeTotalSamples = liveTotalSamples;
                activeMaxSpeed = liveMaxSpeed;
                renderActiveChart();
            } else {
                let resultsCount = 96; // 24h
                if (period === '3d') resultsCount = 288;
                if (period === '7d') resultsCount = 672;
                fetchThingSpeakHistory(resultsCount);
            }
        }

        function onStationOrPeriodChange() {
            if (currentPeriod !== 'live') {
                setPeriod(currentPeriod);
            }
        }

        async function fetchThingSpeakHistory(resultsCount) {
            const chanId = document.getElementById('selStation').value;
            const cfg = TS_CONFIG[chanId];
            if (!cfg) return;

            const loadingEl = document.getElementById('chartLoading');
            const noticeEl = document.getElementById('chartNotice');
            loadingEl.style.display = 'block';
            noticeEl.style.display = 'none';

            const url = `https://api.thingspeak.com/channels/${chanId}/feeds.json?api_key=${cfg.key}&results=${resultsCount}`;

            try {
                const res = await fetch(url);
                if (!res.ok) throw new Error("Chyba odpovede z ThingSpeak API");
                const data = await res.json();

                if (!data.feeds || data.feeds.length === 0) {
                    throw new Error("Žiadne záznamy.");
                }

                let counts = new Array(16).fill(0);
                let speedSums = new Array(16).fill(0);
                let total = 0;
                let maxSpd = 0;
                let validPoints = 0;

                for (const feed of data.feeds) {
                    if (feed.field5 === null || feed.field5 === undefined || feed.field5 === "") continue;
                    const dirDeg = parseFloat(feed.field5);
                    if (isNaN(dirDeg) || dirDeg < 0 || dirDeg > 360) continue;

                    let speed = feed.field6 ? parseFloat(feed.field6) : 0.0;
                    let speedMax = feed.field7 ? parseFloat(feed.field7) : speed;
                    
                    // Plauzibilný filter pre vietor: max 40 m/s priemer, max 45 m/s náraz
                    if (isNaN(speed) || speed < 0 || speed > 40.0) continue;
                    if (isNaN(speedMax) || speedMax < 0 || speedMax > 45.0) speedMax = speed;

                    const idx = Math.round(((dirDeg % 360) / 22.5)) % 16;
                    counts[idx]++;
                    speedSums[idx] += speed;
                    total++;
                    validPoints++;

                    if (speedMax > maxSpd) maxSpd = speedMax;
                    else if (speed > maxSpd) maxSpd = speed;
                }

                loadingEl.style.display = 'none';

                if (validPoints === 0) {
                    noticeEl.innerText = `ℹ️ Stanica ${cfg.name} nemá v zvolenom období uložené dáta o smere vetra (Field 5).`;
                    noticeEl.style.display = 'block';
                    return;
                }

                activeCounts = counts;
                activeSpeedSums = speedSums;
                activeTotalSamples = total;
                activeMaxSpeed = maxSpd;
                renderActiveChart();

            } catch (err) {
                loadingEl.style.display = 'none';
                noticeEl.innerText = `⚠️ Nepodarilo sa načítať ThingSpeak dáta: ${err.message}`;
                noticeEl.style.display = 'block';
            }
        }

        async function fetchLive() {
            try {
                const res = await fetch('/api/live');
                if(!res.ok) return;
                const data = await res.json();

                const tempInFormatted = data.tempIn.toFixed(2);
                const tempOutFormatted = data.tempOut.toFixed(2);
                const speedFormatted = data.windSpeed.toFixed(1);
                const degFormatted = data.windDirDeg.toFixed(0) + '°';

                // Cards Update
                document.getElementById('timestamp').innerText = data.timestamp;
                document.getElementById('tempIn').innerHTML = tempInFormatted + ' <span class="unit">°C</span>';
                document.getElementById('tempOut').innerHTML = tempOutFormatted + ' <span class="unit">°C</span>';
                document.getElementById('windSpeed').innerHTML = speedFormatted + ' <span class="unit">m/s</span>';
                
                document.getElementById('windDirNameBadge').innerText = data.windDirName;
                document.getElementById('windDirNameText').innerText = data.windDirName;
                document.getElementById('windDirDeg').innerText = degFormatted;
                document.getElementById('arrow').style.transform = `rotate(${data.windDirDeg}deg)`;

                // Table Update
                document.getElementById('tblTempIn').innerText = tempInFormatted;
                document.getElementById('tblTempOut').innerText = tempOutFormatted;
                document.getElementById('tblWindSpeed').innerText = speedFormatted;
                document.getElementById('tblWindDirDeg').innerText = degFormatted;
                document.getElementById('tblWindDirName').innerText = data.windDirName;
                document.getElementById('tblWifiSSID').innerText = data.wifiSSID;
                document.getElementById('tblWifiRSSI').innerText = data.rssi + ' dBm';
                document.getElementById('tblIp').innerText = data.ip;
                document.getElementById('tblUptime').innerText = data.uptimeSec + ' s';

                // Wind Rose Update
                if (data.windSpeed !== undefined && data.windDirDeg !== undefined) {
                    addWindRoseSample(data.windDirDeg, data.windSpeed);
                }

                // Temp Line Chart Update
                if (data.tempIn !== undefined && data.tempOut !== undefined) {
                    const timeLabel = new Date().toLocaleTimeString('sk-SK', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
                    addLiveTempSample(timeLabel, data.tempIn, data.tempOut);
                }

            } catch(e) {
                console.error("Fetch error:", e);
            }
        }

        window.addEventListener('DOMContentLoaded', () => {
            initTempLineChart();
            initWindRoseChart();
            fetchLive();
        });

        setInterval(fetchLive, 2000);
    </script>
</body>
</html>
)rawliteral";

WebServerManager::WebServerManager()
    : _server(80),
      _tempMgr(nullptr),
      _anemometer(nullptr),
      _windVane(nullptr),
      _wifiService(nullptr),
      _timeMgr(nullptr) {
}

void WebServerManager::begin(const TempSensorManager* tempMgr, const Anemometer* anemometer, const WindVane* windVane, const WifiService* wifiService, const TimeManager* timeMgr) {
    _tempMgr = tempMgr;
    _anemometer = anemometer;
    _windVane = windVane;
    _wifiService = wifiService;
    _timeMgr = timeMgr;

    _server.on("/", [this]() { handleRoot(); });
    _server.on("/api/live", [this]() { handleApiLive(); });
    _server.onNotFound([this]() { handleNotFound(); });

    _server.begin();
    Serial.println("[WebServer] HTTP Web Dashboard spustený na porte 80");
}

void WebServerManager::handleRoot() {
    _server.send_P(200, "text/html", INDEX_HTML);
}

void WebServerManager::handleNotFound() {
    _server.send(404, "text/plain", "404 Not Found");
}

void WebServerManager::handleApiLive() {
    if (!_tempMgr || !_anemometer || !_windVane || !_wifiService || !_timeMgr) {
        _server.send(500, "application/json", "{\"error\":\"Not initialized\"}");
        return;
    }

    StaticJsonDocument<320> doc;
    doc["timestamp"] = _timeMgr->getFormattedCustom();
    doc["tempIn"] = _tempMgr->getTempIn();
    doc["tempOut"] = _tempMgr->getTempOut();
    doc["windSpeed"] = _anemometer->getWindSpeed();
    doc["windDirDeg"] = _windVane->getInstantAngle();
    doc["windDirName"] = _windVane->getInstantDirName();
    doc["wifiSSID"] = _wifiService->getConnectedSSID();
    doc["rssi"] = _wifiService->getRSSI();
    doc["ip"] = _wifiService->getIPAddress();
    doc["uptimeSec"] = millis() / 1000;

    String json;
    serializeJson(doc, json);
    _server.send(200, "application/json", json);
}

void WebServerManager::handleClient() {
    _server.handleClient();
}
