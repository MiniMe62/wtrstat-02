#include "WebServerManager.h"

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="sk">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title id="pageTitle">wtrStat-02 v)rawliteral" WTRSTAT_FIRMWARE_VERSION R"rawliteral( • Meteostanica</title>
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

        .wind-speed-legend {
            display: flex;
            flex-wrap: wrap;
            align-items: center;
            gap: 6px;
            font-size: 0.72rem;
            color: var(--text-muted);
            font-weight: 600;
        }

        .legend-pill {
            display: inline-flex;
            align-items: center;
            gap: 5px;
            padding: 3px 8px;
            border-radius: 12px;
            background: rgba(255, 255, 255, 0.03);
            border: 1px solid rgba(255, 255, 255, 0.1);
        }

        .legend-pill .dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            display: inline-block;
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

        /* Heliograph Sunshine Timeline Bar */
        .sunshine-bar-wrapper {
            margin: 14px 0 10px 0;
            background: rgba(15, 23, 42, 0.6);
            border: 1px solid var(--card-border);
            border-radius: 12px;
            padding: 12px 14px;
        }
        .sunshine-bar-title {
            display: flex;
            justify-content: space-between;
            align-items: center;
            font-size: 0.8rem;
            color: var(--text-muted);
            margin-bottom: 8px;
            font-weight: 500;
        }
        .sunshine-bar {
            display: flex;
            height: 24px;
            border-radius: 6px;
            overflow: hidden;
            background: #0f172a;
            border: 1px solid rgba(255, 255, 255, 0.08);
            position: relative;
        }
        .sunshine-segment {
            flex: 1;
            height: 100%;
            transition: opacity 0.2s ease;
            position: relative;
        }
        .sunshine-segment:hover {
            opacity: 0.8;
            filter: brightness(1.25);
            cursor: pointer;
        }
        .sunshine-segment.night { background: #1e293b; }
        .sunshine-segment.overcast-dark { background: #475569; }
        .sunshine-segment.overcast { background: #94a3b8; }
        .sunshine-segment.cloudy { background: #fde047; }
        .sunshine-segment.sunny { background: #f59e0b; }
        .sunshine-ticks {
            display: flex;
            justify-content: space-between;
            font-size: 0.72rem;
            color: #64748b;
            margin-top: 4px;
            padding: 0 2px;
        }
        .legend-pill-sun {
            display: inline-flex;
            align-items: center;
            gap: 5px;
            font-size: 0.72rem;
            color: var(--text-muted);
            background: rgba(255, 255, 255, 0.03);
            border: 1px solid var(--card-border);
            padding: 3px 8px;
            border-radius: 8px;
        }
        .legend-pill-sun .dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
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
                <h1 id="appTitle">🌤️ wtrStat-02 <span style="font-size: 0.92rem; font-weight: 500; opacity: 0.8; vertical-align: middle;">v)rawliteral" WTRSTAT_FIRMWARE_VERSION R"rawliteral(</span></h1>
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
                    <div class="compass-details">
                        <div class="card-val" id="windDirNameText" style="font-size: 1.5rem;">--</div>
                        <div class="card-sub" id="windDirDeg">--°</div>
                    </div>
                </div>
            </div>

            <div class="card">
                <div class="card-header">Jas a Slnko <span id="skyConditionBadge" style="background: rgba(251, 191, 36, 0.2); color: #fbbf24; padding: 2px 6px; border-radius: 6px; font-size: 0.75rem;">--</span></div>
                <div style="display: flex; align-items: center; gap: 12px; margin-top: 4px;">
                    <div style="position: relative; width: 68px; height: 48px; flex-shrink: 0;">
                        <svg viewBox="0 0 100 62" style="width: 100%; height: 100%; overflow: visible;">
                            <defs>
                                <linearGradient id="solarGaugeGrad" x1="0%" y1="0%" x2="100%" y2="0%">
                                    <stop offset="0%" stop-color="#38bdf8"/>
                                    <stop offset="30%" stop-color="#94a3b8"/>
                                    <stop offset="65%" stop-color="#fde047"/>
                                    <stop offset="100%" stop-color="#f59e0b"/>
                                </linearGradient>
                            </defs>
                            <path d="M 15 52 A 38 38 0 0 1 85 52" fill="none" stroke="rgba(255,255,255,0.08)" stroke-width="8" stroke-linecap="round"/>
                            <path id="solarGaugeArc" d="M 15 52 A 38 38 0 0 1 85 52" fill="none" stroke="url(#solarGaugeGrad)" stroke-width="8" stroke-linecap="round" stroke-dasharray="119.4" stroke-dashoffset="119.4" style="transition: stroke-dashoffset 0.6s cubic-bezier(0.4, 0, 0.2, 1);"/>
                            <text id="solarGaugeIcon" x="50" y="48" text-anchor="middle" font-size="16" fill="#fbbf24">☀️</text>
                        </svg>
                    </div>
                    <div>
                        <div class="card-val" id="lightPercent" style="margin:0; font-size:2.0rem; font-weight:700;">-- <span class="unit">%</span></div>
                        <div class="card-sub" id="sunshineDuration" style="font-size:0.82rem; color:var(--text-muted);">Svit: --</div>
                    </div>
                </div>
            </div>

            <div class="card">
                <div class="card-header">Zrážky <span id="rainIntensityBadge" style="background: rgba(56, 189, 248, 0.2); color: #38bdf8; padding: 2px 6px; border-radius: 6px; font-size: 0.75rem;">Bez zrážok</span></div>
                <div style="display: flex; align-items: center; gap: 12px; margin-top: 4px;">
                    <div style="position: relative; width: 44px; height: 56px; flex-shrink: 0;">
                        <svg viewBox="0 0 44 56" style="width: 100%; height: 100%; overflow: visible;">
                            <defs>
                                <linearGradient id="rainWaterGrad" x1="0%" y1="0%" x2="0%" y2="100%">
                                    <stop offset="0%" stop-color="#38bdf8"/>
                                    <stop offset="100%" stop-color="#0284c7"/>
                                </linearGradient>
                                <clipPath id="rainCylinderClip">
                                    <rect x="6" y="4" width="22" height="46" rx="4" ry="4"/>
                                </clipPath>
                            </defs>
                            <!-- Sklenený valec pozadie -->
                            <rect x="6" y="4" width="22" height="46" rx="4" ry="4" fill="rgba(255,255,255,0.04)" stroke="rgba(255,255,255,0.2)" stroke-width="1.5"/>
                            <!-- Hladina vody -->
                            <g clip-path="url(#rainCylinderClip)">
                                <rect id="rainWaterLevel" x="6" y="50" width="22" height="0" fill="url(#rainWaterGrad)" style="transition: all 0.6s cubic-bezier(0.4, 0, 0.2, 1);"/>
                                <path id="rainWaterWave" d="M 6 50 Q 11 48 17 50 T 28 50 L 28 50 L 6 50 Z" fill="#7dd3fc" opacity="0.6" style="transition: all 0.6s cubic-bezier(0.4, 0, 0.2, 1);"/>
                            </g>
                            <!-- Rysky stupnice (0, 5, 10, 15, 20, 25 mm) -->
                            <line x1="20" y1="12" x2="26" y2="12" stroke="rgba(255,255,255,0.45)" stroke-width="1"/>
                            <line x1="22" y1="20" x2="26" y2="20" stroke="rgba(255,255,255,0.25)" stroke-width="1"/>
                            <line x1="20" y1="28" x2="26" y2="28" stroke="rgba(255,255,255,0.45)" stroke-width="1"/>
                            <line x1="22" y1="36" x2="26" y2="36" stroke="rgba(255,255,255,0.25)" stroke-width="1"/>
                            <line x1="20" y1="44" x2="26" y2="44" stroke="rgba(255,255,255,0.45)" stroke-width="1"/>
                            <text x="29" y="14" font-size="6.5" font-weight="600" fill="var(--text-muted)">25</text>
                            <text x="29" y="30" font-size="6.5" font-weight="600" fill="var(--text-muted)">15</text>
                            <text x="29" y="46" font-size="6.5" font-weight="600" fill="var(--text-muted)">5</text>
                        </svg>
                    </div>
                    <div>
                        <div class="card-val" id="rainToday" style="margin:0; font-size:1.85rem; font-weight:700;">0.00 <span class="unit">mm</span></div>
                        <div class="card-sub" id="rainSub" style="font-size:0.80rem; color:var(--text-muted);">15 min: 0.00 mm • 0 tipov</div>
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
                            <td>Jas oblohy (Intenzita)</td>
                            <td class="val-highlight" id="tblLight">--</td>
                            <td>%</td>
                            <td>TEMT6000 ADC (GPIO 35)</td>
                        </tr>
                        <tr>
                            <td>Denný slnečný svit</td>
                            <td class="val-highlight" id="tblSun">--</td>
                            <td>h:min</td>
                            <td>Kumulatívne počítadlo svitu</td>
                        </tr>
                        <tr>
                            <td>Zrážky dnes (od 00:00)</td>
                            <td class="val-highlight" id="tblRainToday">--</td>
                            <td>mm</td>
                            <td>Tipping Bucket Reed (GPIO 4)</td>
                        </tr>
                        <tr>
                            <td>Zrážky za 15 minút</td>
                            <td class="val-highlight" id="tblRain15m">--</td>
                            <td>mm</td>
                            <td>15-minútová záverka</td>
                        </tr>
                        <tr>
                            <td>Intenzita zrážok (Rain Rate)</td>
                            <td class="val-highlight" id="tblRainRate">--</td>
                            <td>mm/h</td>
                            <td>Kĺzavé 60-minútové okno</td>
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
                    <button class="btn-view" id="btnTempLightToggle" onclick="toggleTempLightOverlay()" title="Podfarbenie intenzitou svetla na pozadí">☀️ +Svetlo</button>
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

        <!-- Light Intensity & Sunshine Section -->
        <div class="chart-box">
            <div class="chart-header-row">
                <div class="section-title">☀️ Vývoj Intenzity Osvetlenia a Slnečného Svitu</div>
                <button class="btn-bubble-toggle active" id="btnToggleLightTooltip" onclick="toggleLightTooltip()" title="Zapnúť / Vypnúť bubliny">
                    💬 Bubliny <span id="lblLightTooltip">ZAP</span>
                </button>
            </div>
            <p style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 8px;">
                Plošný priebeh intenzity svetla (TEMT6000), heliografická časová lišta a rozdelenie stavu oblohy.
            </p>

            <!-- Control Bar (View Mode & Timeframe Selector) -->
            <div class="chart-controls">
                <div class="btn-group">
                    <button class="btn-view active" id="btnLightModeArea" onclick="setLightViewMode('area')">📈 Plošný Graf (%)</button>
                    <button class="btn-view" id="btnLightModeTimeline" onclick="setLightViewMode('timeline')">⏱️ Heliograf (Lišta)</button>
                    <button class="btn-view" id="btnLightModeDonut" onclick="setLightViewMode('donut')">🥧 Denný Podiel</button>
                </div>

                <div class="btn-group">
                    <button class="btn-period active" id="btnLightPeriodLive" onclick="setLightPeriod('live')">⚡ Živé</button>
                    <button class="btn-period" id="btnLightPeriod24h" onclick="setLightPeriod('24h')">📅 24h</button>
                    <button class="btn-period" id="btnLightPeriod3d" onclick="setLightPeriod('3d')">📆 3d</button>
                    <button class="btn-period" id="btnLightPeriod7d" onclick="setLightPeriod('7d')">🗓️ 7d</button>
                </div>
            </div>

            <div id="lightChartLoading" style="display: none; font-size: 0.82rem; color: var(--primary); text-align: center; margin: 8px 0; font-weight: 500;">
                ⏳ Načítavam dáta intenzity svetla...
            </div>
            <div id="lightChartNotice" style="display: none; font-size: 0.82rem; color: #fbbf24; text-align: center; margin: 8px 0; background: rgba(251, 191, 36, 0.1); padding: 6px 12px; border-radius: 8px;">
            </div>

            <!-- 1. Area Chart Wrapper -->
            <div class="chart-line-wrapper" id="lightAreaWrapper">
                <canvas id="lightAreaChart"></canvas>
            </div>

            <!-- 2. Donut Chart Wrapper (Zobrazí sa pri móde 'donut') -->
            <div id="lightDonutWrapper" style="display: none; max-width: 380px; margin: 16px auto; position: relative;">
                <canvas id="lightDonutChart"></canvas>
            </div>

            <!-- 3. Sunshine Timeline Bar (Campbell-Stokes Heliograf) -->
            <div class="sunshine-bar-wrapper" id="sunshineBarWrapper">
                <div class="sunshine-bar-title">
                    <span>⏱️ Heliografická Lišta Slnečného Svitu (24h Záznam)</span>
                    <span id="sunshineTimelineTotal" style="color: #fbbf24; font-weight: 700;">Dnes: --</span>
                </div>
                <div class="sunshine-bar" id="sunshineBar">
                    <!-- Segmenty sa dynamicky generujú cez JS -->
                </div>
                <div class="sunshine-ticks">
                    <span>00:00</span>
                    <span>04:00</span>
                    <span>08:00</span>
                    <span>12:00</span>
                    <span>16:00</span>
                    <span>20:00</span>
                    <span>24:00</span>
                </div>
                <div style="display: flex; flex-wrap: wrap; gap: 8px; justify-content: center; margin-top: 10px;">
                    <span class="legend-pill-sun"><span class="dot" style="background:#1e293b;"></span> Noc / Tma</span>
                    <span class="legend-pill-sun"><span class="dot" style="background:#475569;"></span> Husto zamračené</span>
                    <span class="legend-pill-sun"><span class="dot" style="background:#94a3b8;"></span> Zamračené</span>
                    <span class="legend-pill-sun"><span class="dot" style="background:#fde047;"></span> Polooblačno</span>
                    <span class="legend-pill-sun"><span class="dot" style="background:#f59e0b;"></span> Priame slnko (Svit)</span>
                </div>
            </div>
            
            <div class="chart-stats-grid">
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Dnešný Svit (Heliograf)</span>
                    <span class="stat-badge-val" id="statLightSunshine" style="color: #fbbf24;">--</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Max Intenzita</span>
                    <span class="stat-badge-val" id="statLightMax" style="color: #f59e0b;">-- %</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Denný Priemer</span>
                    <span class="stat-badge-val" id="statLightAvg" style="color: #38bdf8;">-- %</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Aktuálny Stav Oblohy</span>
                    <span class="stat-badge-val" id="statLightCurrent" style="color: #4ade80;">--</span>
                </div>
            </div>
        </div>

        <!-- Rain & Hyetograph Section -->
        <div class="chart-box">
            <div class="chart-header-row">
                <div class="section-title">🌧️ Vývoj Zrážok a Hyetograf</div>
                <button class="btn-bubble-toggle active" id="btnToggleRainTooltip" onclick="toggleRainTooltip()" title="Zapnúť / Vypnúť bubliny">
                    💬 Bubliny <span id="lblRainTooltip">ZAP</span>
                </button>
            </div>
            <p style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 8px;">
                Stĺpcový prehľad zrážkových úhrnov v čase (15 min / 1 h), kumulatívna denná čiara a analýza intenzity dažďa.
            </p>

            <!-- Control Bar (View Mode & Timeframe Selector) -->
            <div class="chart-controls">
                <div class="btn-group">
                    <button class="btn-view active" id="btnRainModeBars" onclick="setRainViewMode('bars')">📊 Úhrny (mm)</button>
                    <button class="btn-view" id="btnRainModeRate" onclick="setRainViewMode('rate')">⚡ Intenzita (mm/h)</button>
                    <button class="btn-view" id="btnRainModeCumulative" onclick="setRainViewMode('cumulative')">📈 Kumulatívne</button>
                </div>

                <div class="btn-group">
                    <button class="btn-period active" id="btnRainPeriodLive" onclick="setRainPeriod('live')">⚡ Živé</button>
                    <button class="btn-period" id="btnRainPeriod24h" onclick="setRainPeriod('24h')">📅 24h</button>
                    <button class="btn-period" id="btnRainPeriod3d" onclick="setRainPeriod('3d')">📆 3d</button>
                    <button class="btn-period" id="btnRainPeriod7d" onclick="setRainPeriod('7d')">🗓️ 7d</button>
                </div>
            </div>

            <div id="rainChartLoading" style="display: none; font-size: 0.82rem; color: var(--primary); text-align: center; margin: 8px 0; font-weight: 500;">
                ⏳ Načítavam dáta zrážok z ThingSpeak API...
            </div>
            <div id="rainChartNotice" style="display: none; font-size: 0.82rem; color: #fbbf24; text-align: center; margin: 8px 0; background: rgba(251, 191, 36, 0.1); padding: 6px 12px; border-radius: 8px;">
            </div>

            <div class="chart-line-wrapper">
                <canvas id="rainBarChart"></canvas>
            </div>
            
            <div class="chart-stats-grid">
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Zrážky Dnes</span>
                    <span class="stat-badge-val" id="statRainToday" style="color: #38bdf8;">0.00 mm</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Posledných 15 min</span>
                    <span class="stat-badge-val" id="statRain15m" style="color: #7dd3fc;">0.00 mm</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Max Intenzita</span>
                    <span class="stat-badge-val" id="statRainPeakRate" style="color: #818cf8;">0.00 mm/h</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Počet Preklopení</span>
                    <span class="stat-badge-val" id="statRainTips">0 tipov</span>
                </div>
            </div>
        </div>

        <!-- Wind Speed & Gusts History Section (SHMÚ ALADIN style) -->
        <div class="chart-box">
            <div class="chart-header-row">
                <div class="section-title">💨 Vývoj Rýchlosti a Nárazov Vetra</div>
                <button class="btn-bubble-toggle active" id="btnToggleWindSpeedTooltip" onclick="toggleWindSpeedTooltip()" title="Zapnúť / Vypnúť bubliny">
                    💬 Bubliny <span id="lblWindSpeedTooltip">ZAP</span>
                </button>
            </div>
            <p style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 8px;">
                Časový priebeh priemernej rýchlosti vetra (čiara) s vloženými stĺpcami významných nárazov (štýl ALADIN / SHMÚ).
            </p>

            <!-- Control Bar (Gust Filter & Timeframe Selector) -->
            <div class="chart-controls">
                <div class="btn-group">
                    <button class="btn-view active" id="btnGustThresholdSig" onclick="setGustFilter('significant')">⚡ Výrazné nárazy (&gt;1.3×)</button>
                    <button class="btn-view" id="btnGustThresholdHigh" onclick="setGustFilter('high')">🔥 Silné (&gt;1.5×)</button>
                    <button class="btn-view" id="btnGustThresholdAll" onclick="setGustFilter('all')">📊 Všetky nárazy</button>
                    <button class="btn-view" id="btnGustThresholdNone" onclick="setGustFilter('none')">🚫 Skryť nárazy</button>
                </div>

                <div class="btn-group">
                    <button class="btn-period active" id="btnWindSpeedPeriodLive" onclick="setWindSpeedPeriod('live')">⚡ Živé</button>
                    <button class="btn-period" id="btnWindSpeedPeriod24h" onclick="setWindSpeedPeriod('24h')">📅 24h</button>
                    <button class="btn-period" id="btnWindSpeedPeriod3d" onclick="setWindSpeedPeriod('3d')">📆 3d</button>
                    <button class="btn-period" id="btnWindSpeedPeriod7d" onclick="setWindSpeedPeriod('7d')">🗓️ 7d</button>
                </div>
            </div>

            <div id="windSpeedChartLoading" style="display: none; font-size: 0.82rem; color: var(--primary); text-align: center; margin: 8px 0; font-weight: 500;">
                ⏳ Načítavam dáta rýchlosti vetra z ThingSpeak API...
            </div>
            <div id="windSpeedChartNotice" style="display: none; font-size: 0.82rem; color: #fbbf24; text-align: center; margin: 8px 0; background: rgba(251, 191, 36, 0.1); padding: 6px 12px; border-radius: 8px;">
            </div>

            <div class="chart-line-wrapper">
                <canvas id="windSpeedChart"></canvas>
            </div>
            
            <div class="chart-stats-grid">
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Aktuálna Rýchlosť</span>
                    <span class="stat-badge-val" id="statSpeedCurrent" style="color: #38bdf8;">-- m/s</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Priemerná Rýchlosť</span>
                    <span class="stat-badge-val" id="statSpeedAvg" style="color: #38bdf8;">-- m/s</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Max Náraz (GUST)</span>
                    <span class="stat-badge-val" id="statSpeedMaxGust" style="color: #fb7185;">-- m/s</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Výrazné Nárazy</span>
                    <span class="stat-badge-val" id="statSpeedGustCount" style="color: #fbbf24;">0</span>
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

        <!-- Wind Direction Timeline Section -->
        <div class="chart-box">
            <div class="chart-header-row">
                <div class="section-title">🧭 Časový Vývoj Smeru Vetra</div>
                <button class="btn-bubble-toggle active" id="btnToggleWindTimelineTooltip" onclick="toggleWindTimelineTooltip()" title="Zapnúť / Vypnúť bubliny">
                    💬 Bubliny <span id="lblWindTimelineTooltip">ZAP</span>
                </button>
            </div>
            <p style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 8px;">
                Chronologický priebeh stáčania smeru vetra v čase s farebným odlíšením rýchlosti (štýl SHMÚ).
            </p>

            <!-- Control Bar (Timeframe & Speed Legend) -->
            <div class="chart-controls">
                <div class="btn-group">
                    <button class="btn-period active" id="btnWindTimelineLive" onclick="setWindTimelinePeriod('live')">⚡ Živé</button>
                    <button class="btn-period" id="btnWindTimeline24h" onclick="setWindTimelinePeriod('24h')">📅 24h</button>
                    <button class="btn-period" id="btnWindTimeline3d" onclick="setWindTimelinePeriod('3d')">📆 3d</button>
                    <button class="btn-period" id="btnWindTimeline7d" onclick="setWindTimelinePeriod('7d')">🗓️ 7d</button>
                </div>

                <div class="wind-speed-legend">
                    <span class="legend-pill" style="border-color: #38bdf8;"><span class="dot" style="background:#38bdf8;"></span> 0.3–1.5 (Vánok)</span>
                    <span class="legend-pill" style="border-color: #22c55e;"><span class="dot" style="background:#22c55e;"></span> 1.5–4.0 (Mierny)</span>
                    <span class="legend-pill" style="border-color: #facc15;"><span class="dot" style="background:#facc15;"></span> 4.0–8.0 (Čerstvý)</span>
                    <span class="legend-pill" style="border-color: #f43f5e;"><span class="dot" style="background:#f43f5e;"></span> &gt;8.0 m/s (Silný)</span>
                </div>
            </div>

            <div id="windTimelineLoading" style="display: none; font-size: 0.82rem; color: var(--primary); text-align: center; margin: 8px 0; font-weight: 500;">
                ⏳ Načítavam dáta smeru vetra z ThingSpeak API...
            </div>
            <div id="windTimelineNotice" style="display: none; font-size: 0.82rem; color: #fbbf24; text-align: center; margin: 8px 0; background: rgba(251, 191, 36, 0.1); padding: 6px 12px; border-radius: 8px;">
            </div>

            <div class="chart-line-wrapper">
                <canvas id="windTimelineChart"></canvas>
            </div>
            
            <div class="chart-stats-grid">
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Aktuálny Smer</span>
                    <span class="stat-badge-val" id="statTimelineLastDir">--</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Priemerný Smer</span>
                    <span class="stat-badge-val" id="statTimelineAvgDir">--</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Max Rýchlosť</span>
                    <span class="stat-badge-val" id="statTimelineMaxSpeed">-- m/s</span>
                </div>
                <div class="stat-badge">
                    <span class="stat-badge-lbl">Počet Záznamov</span>
                    <span class="stat-badge-val" id="statTimelineSamples">0</span>
                </div>
            </div>
        </div>

        <footer>
            <div style="display: flex; flex-wrap: wrap; justify-content: center; align-items: center; gap: 12px; margin-bottom: 12px;">
                <button type="button" id="btnSimRain" onclick="simulateRainTip()" style="background: rgba(56, 189, 248, 0.12); border: 1px solid rgba(56, 189, 248, 0.35); color: var(--primary); padding: 6px 14px; border-radius: 12px; cursor: pointer; font-size: 0.82rem; font-weight: 600; display: inline-flex; align-items: center; gap: 6px; transition: all 0.2s ease;">
                    💧 Simulovať preklop zrážkomera (+1 tip)
                </button>
            </div>
            <div style="margin-bottom: 6px; font-weight: 500;">
                wtrStat-02 • ESP32 Weather Station Firmware • <a href="/update" style="color: var(--primary); text-decoration: none;">⚙️ OTA Update</a>
            </div>
            <div style="font-size: 0.78rem; opacity: 0.85; margin-top: 4px;" id="footerBuildInfo">
                Firmware: <span id="fwVersion" style="color: var(--primary); font-weight: 600;">v)rawliteral" WTRSTAT_FIRMWARE_VERSION R"rawliteral(</span>
                • Zostavené: <span id="fwBuild" style="font-weight: 600;">)rawliteral" __DATE__ " " __TIME__ R"rawliteral(</span>
            </div>
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
        let liveLightData = [5, 12, 35, 60, 75, 88, 92, 85, 70, 45, 20, 10];

        let activeTempLabels = [...liveTempLabels];
        let activeTempIn = [...liveTempIn];
        let activeTempOut = [...liveTempOut];
        let activeTempLight = [...liveLightData];
        let showTempLightOverlay = false;

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
                            yAxisID: 'y',
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
                            yAxisID: 'y',
                            borderColor: '#38bdf8',
                            backgroundColor: 'rgba(56, 189, 248, 0.12)',
                            fill: true,
                            tension: 0.35,
                            borderWidth: 2.2,
                            pointRadius: 2,
                            pointHoverRadius: 6,
                            pointBackgroundColor: '#38bdf8'
                        },
                        {
                            label: 'Svetlo na pozadí',
                            data: activeTempLight,
                            yAxisID: 'yLight',
                            borderColor: '#f59e0b',
                            backgroundColor: 'rgba(251, 191, 36, 0.18)',
                            fill: true,
                            tension: 0.35,
                            borderWidth: 1.5,
                            pointRadius: 0,
                            pointHoverRadius: 4,
                            pointBackgroundColor: '#fbbf24',
                            hidden: !showTempLightOverlay
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
                        },
                        yLight: {
                            type: 'linear',
                            position: 'right',
                            min: 0,
                            max: 100,
                            display: showTempLightOverlay,
                            grid: { drawOnChartArea: false },
                            ticks: {
                                color: '#fbbf24',
                                font: { size: 10, family: 'Inter' },
                                callback: function(val) { return val + '%'; }
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
                                    if (context.dataset.yAxisID === 'yLight') {
                                        return ` ${context.dataset.label}: ${parseFloat(context.parsed.y).toFixed(0)} %`;
                                    }
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

        function toggleTempLightOverlay() {
            showTempLightOverlay = !showTempLightOverlay;
            const btn = document.getElementById('btnTempLightToggle');
            if (btn) btn.classList.toggle('active', showTempLightOverlay);
            if (tempLineChartInstance) {
                tempLineChartInstance.setDatasetVisibility(2, showTempLightOverlay);
                tempLineChartInstance.options.scales.yLight.display = showTempLightOverlay;
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
                activeTempLight = [...liveLightData];
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
            tempLineChartInstance.data.datasets[2].data = activeTempLight;

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

            const url = `https://api.thingspeak.com/channels/${chanId}/feeds.json?api_key=${cfg.key}&results=${resultsCount}&status=true`;

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
                let lightSeries = [];

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

                    let lVal = null;
                    if (feed.status) {
                        try {
                            const stObj = JSON.parse(feed.status);
                            if (stObj.light !== undefined && stObj.light !== null && stObj.light !== "") {
                                lVal = parseFloat(stObj.light);
                            }
                        } catch(e) {}
                    }

                    labels.push(label);
                    tinSeries.push(tIn);
                    toutSeries.push(tOut);
                    lightSeries.push(lVal !== null && !isNaN(lVal) ? lVal : 0);
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
                activeTempLight = lightSeries;
                renderTempChart();

            } catch (err) {
                loadingEl.style.display = 'none';
                noticeEl.innerText = `⚠️ Nepodarilo sa načítať teploty: ${err.message}`;
                noticeEl.style.display = 'block';
            }
        }

        function addLiveTempSample(timeLabel, tIn, tOut, lightVal) {
            // Plauzibilný filter pre živé dáta
            const validIn = (tIn !== null && !isNaN(tIn) && tIn >= -40.0 && tIn <= 60.0 && Math.abs(tIn - 85.0) > 0.1 && Math.abs(tIn + 127.0) > 0.1) ? tIn : null;
            const validOut = (tOut !== null && !isNaN(tOut) && tOut >= -40.0 && tOut <= 60.0 && Math.abs(tOut - 85.0) > 0.1 && Math.abs(tOut + 127.0) > 0.1) ? tOut : null;
            const validLight = (lightVal !== null && !isNaN(lightVal)) ? lightVal : 0;

            liveTempLabels.push(timeLabel);
            liveTempIn.push(validIn);
            liveTempOut.push(validOut);
            liveLightData.push(validLight);

            if (liveTempLabels.length > 25) {
                liveTempLabels.shift();
                liveTempIn.shift();
                liveTempOut.shift();
                liveLightData.shift();
            }

            if (tempPeriod === 'live') {
                activeTempLabels = [...liveTempLabels];
                activeTempIn = [...liveTempIn];
                activeTempOut = [...liveTempOut];
                activeTempLight = [...liveLightData];
                renderTempChart();
            }
        }

        // ================= GRAF INTENZITY OSVETLENIA A SLNEČNÉHO SVITU =================
        let lightViewMode = 'area'; // 'area' | 'timeline' | 'donut'
        let lightPeriod = 'live';   // 'live' | '24h' | '3d' | '7d'
        let lightTooltipEnabled = true;

        let liveLightLabels = ['08:00', '08:30', '09:00', '09:30', '10:00', '10:30', '11:00', '11:30', '12:00', '12:30', '13:00', '13:15'];
        let liveLightSeries = [5, 12, 35, 60, 75, 88, 92, 85, 70, 45, 20, 10]; // %

        let activeLightLabels = [...liveLightLabels];
        let activeLightData = [...liveLightSeries];

        let lightAreaChartInstance = null;
        let lightDonutChartInstance = null;

        function initLightAreaChart() {
            const canvas = document.getElementById('lightAreaChart');
            if (!canvas) return;
            const ctx = canvas.getContext('2d');

            const grad = ctx.createLinearGradient(0, 0, 0, 280);
            grad.addColorStop(0, 'rgba(245, 158, 11, 0.45)');
            grad.addColorStop(0.5, 'rgba(253, 224, 71, 0.18)');
            grad.addColorStop(1, 'rgba(245, 158, 11, 0.00)');

            lightAreaChartInstance = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: activeLightLabels,
                    datasets: [
                        {
                            label: 'Intenzita Svetla',
                            data: activeLightData,
                            borderColor: '#f59e0b',
                            backgroundColor: grad,
                            fill: true,
                            tension: 0.35,
                            borderWidth: 2.4,
                            pointRadius: 2,
                            pointHoverRadius: 6,
                            pointBackgroundColor: '#fbbf24'
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
                            grid: { color: 'rgba(255, 255, 255, 0.05)' },
                            ticks: {
                                color: '#94a3b8',
                                font: { size: 10, family: 'Inter' },
                                maxRotation: 0,
                                autoSkip: true,
                                maxTicksLimit: 10
                            }
                        },
                        y: {
                            min: 0,
                            max: 100,
                            grid: { color: 'rgba(255, 255, 255, 0.06)' },
                            ticks: {
                                color: '#fbbf24',
                                font: { size: 11, family: 'Inter' },
                                callback: function(val) { return val + ' %'; }
                            }
                        }
                    },
                    plugins: {
                        legend: { display: false },
                        tooltip: {
                            enabled: lightTooltipEnabled,
                            backgroundColor: 'rgba(15, 23, 42, 0.92)',
                            titleColor: '#fbbf24',
                            titleFont: { weight: '700', size: 12 },
                            bodyFont: { size: 12 },
                            borderColor: 'rgba(251, 191, 36, 0.3)',
                            borderWidth: 1,
                            padding: 10,
                            callbacks: {
                                label: function(context) {
                                    const val = parseFloat(context.parsed.y);
                                    let cond = 'Noc / Tma';
                                    if (val >= 60) cond = 'Priame slnko (Svit)';
                                    else if (val >= 35) cond = 'Polooblačno';
                                    else if (val >= 15) cond = 'Zamračené';
                                    else if (val >= 3) cond = 'Husto zamračené';
                                    return ` Intenzita: ${val.toFixed(1)} % (${cond})`;
                                }
                            }
                        }
                    }
                }
            });

            updateLightStats();
            updateSunshineTimeline(activeLightLabels, activeLightData);
        }

        function initLightDonutChart() {
            const canvas = document.getElementById('lightDonutChart');
            if (!canvas) return;
            const ctx = canvas.getContext('2d');

            lightDonutChartInstance = new Chart(ctx, {
                type: 'doughnut',
                data: {
                    labels: ['Priame slnko (Svit)', 'Polooblačno', 'Zamračené', 'Noc / Tma'],
                    datasets: [{
                        data: [25, 30, 20, 25],
                        backgroundColor: ['#f59e0b', '#fde047', '#94a3b8', '#1e293b'],
                        borderColor: '#0f172a',
                        borderWidth: 2
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: {
                            position: 'bottom',
                            labels: { color: '#94a3b8', font: { size: 11, family: 'Inter' }, boxWidth: 12 }
                        },
                        tooltip: {
                            backgroundColor: 'rgba(15, 23, 42, 0.92)',
                            callbacks: {
                                label: function(context) {
                                    return ` ${context.label}: ${context.parsed.toFixed(1)} %`;
                                }
                            }
                        }
                    }
                }
            });
        }

        function updateLightStats() {
            const valid = activeLightData.filter(v => v !== null && !isNaN(v));
            if (valid.length === 0) return;

            const max = Math.max(...valid).toFixed(0);
            const avg = (valid.reduce((a, b) => a + b, 0) / valid.length).toFixed(1);
            const last = valid[valid.length - 1];

            document.getElementById('statLightMax').innerText = `${max} %`;
            document.getElementById('statLightAvg').innerText = `${avg} %`;

            let currentCond = 'Noc / Tma';
            if (last >= 60) currentCond = 'Priame slnko';
            else if (last >= 35) currentCond = 'Polooblačno';
            else if (last >= 15) currentCond = 'Zamračené';
            else if (last >= 3) currentCond = 'Husto zamračené';
            document.getElementById('statLightCurrent').innerText = currentCond;

            // Aktualizácia koláčového grafu
            let countSun = 0, countCloudy = 0, countOvercast = 0, countNight = 0;
            valid.forEach(v => {
                if (v >= 60) countSun++;
                else if (v >= 35) countCloudy++;
                else if (v >= 15) countOvercast++;
                else countNight++;
            });
            const total = valid.length;
            if (lightDonutChartInstance) {
                lightDonutChartInstance.data.datasets[0].data = [
                    (countSun / total) * 100,
                    (countCloudy / total) * 100,
                    (countOvercast / total) * 100,
                    (countNight / total) * 100
                ];
                lightDonutChartInstance.update();
            }
        }

        function updateSunshineTimeline(labels, data) {
            const bar = document.getElementById('sunshineBar');
            if (!bar) return;
            bar.innerHTML = '';
            if (!data || data.length === 0) return;

            let sunCount = 0;
            data.forEach((val, idx) => {
                const seg = document.createElement('div');
                seg.className = 'sunshine-segment';
                let cond = 'night';
                let labelText = 'Noc / Tma';
                const v = (val !== null && !isNaN(val)) ? val : 0;

                if (v >= 60) {
                    cond = 'sunny';
                    labelText = 'Priame slnko';
                    sunCount++;
                } else if (v >= 35) {
                    cond = 'cloudy';
                    labelText = 'Polooblačno';
                } else if (v >= 15) {
                    cond = 'overcast';
                    labelText = 'Zamračené';
                } else if (v >= 3) {
                    cond = 'overcast-dark';
                    labelText = 'Husto zamračené';
                }

                seg.classList.add(cond);
                const timeStr = labels && labels[idx] ? labels[idx] : '';
                seg.title = `${timeStr}: ${v.toFixed(0)}% (${labelText})`;
                bar.appendChild(seg);
            });

            // Odhad celkového svitu
            const intervalMinutes = (labels && labels.length > 1) ? 15 : 15;
            const totalMinutes = sunCount * intervalMinutes;
            const hrs = Math.floor(totalMinutes / 60);
            const mins = totalMinutes % 60;
            const formatted = `${hrs}h ${mins.toString().padStart(2, '0')}m`;
            const totEl = document.getElementById('sunshineTimelineTotal');
            if (totEl) totEl.innerText = `Dnes: ${formatted}`;
            const statSun = document.getElementById('statLightSunshine');
            if (statSun) statSun.innerText = formatted;
        }

        function updateSolarGauge(percent, condition) {
            const arc = document.getElementById('solarGaugeArc');
            const icon = document.getElementById('solarGaugeIcon');
            if (!arc) return;
            const clamped = Math.max(0, Math.min(100, (percent !== null && !isNaN(percent)) ? percent : 0));
            // 119.4 is the arc length of radius 38 semicircle (PI * 38 = 119.38)
            const offset = 119.4 * (1 - clamped / 100);
            arc.style.strokeDashoffset = offset;
            
            if (icon) {
                if (clamped < 3) icon.textContent = '🌙';
                else if (clamped < 20) icon.textContent = '🌧️';
                else if (clamped < 40) icon.textContent = '☁️';
                else if (clamped < 65) icon.textContent = '⛅';
                else icon.textContent = '☀️';
            }
        }

        function setLightViewMode(mode) {
            lightViewMode = mode;
            ['btnLightModeArea', 'btnLightModeTimeline', 'btnLightModeDonut'].forEach(id => {
                document.getElementById(id).classList.remove('active');
            });

            const areaWrap = document.getElementById('lightAreaWrapper');
            const donutWrap = document.getElementById('lightDonutWrapper');
            const barWrap = document.getElementById('sunshineBarWrapper');

            if (mode === 'area') {
                document.getElementById('btnLightModeArea').classList.add('active');
                if (areaWrap) areaWrap.style.display = 'block';
                if (donutWrap) donutWrap.style.display = 'none';
                if (barWrap) barWrap.style.display = 'block';
            } else if (mode === 'timeline') {
                document.getElementById('btnLightModeTimeline').classList.add('active');
                if (areaWrap) areaWrap.style.display = 'none';
                if (donutWrap) donutWrap.style.display = 'none';
                if (barWrap) barWrap.style.display = 'block';
            } else if (mode === 'donut') {
                document.getElementById('btnLightModeDonut').classList.add('active');
                if (areaWrap) areaWrap.style.display = 'none';
                if (donutWrap) donutWrap.style.display = 'block';
                if (barWrap) barWrap.style.display = 'none';
            }
        }

        function toggleLightTooltip() {
            lightTooltipEnabled = !lightTooltipEnabled;
            const btn = document.getElementById('btnToggleLightTooltip');
            const lbl = document.getElementById('lblLightTooltip');
            if (btn) btn.classList.toggle('active', lightTooltipEnabled);
            if (lbl) lbl.innerText = lightTooltipEnabled ? 'ZAP' : 'VYP';
            if (lightAreaChartInstance) {
                lightAreaChartInstance.options.plugins.tooltip.enabled = lightTooltipEnabled;
                lightAreaChartInstance.update('none');
            }
        }

        function setLightPeriod(period) {
            lightPeriod = period;

            ['btnLightPeriodLive', 'btnLightPeriod24h', 'btnLightPeriod3d', 'btnLightPeriod7d'].forEach(id => {
                document.getElementById(id).classList.remove('active');
            });

            if (period === 'live') document.getElementById('btnLightPeriodLive').classList.add('active');
            if (period === '24h') document.getElementById('btnLightPeriod24h').classList.add('active');
            if (period === '3d') document.getElementById('btnLightPeriod3d').classList.add('active');
            if (period === '7d') document.getElementById('btnLightPeriod7d').classList.add('active');

            document.getElementById('lightChartNotice').style.display = 'none';

            if (period === 'live') {
                document.getElementById('lightChartLoading').style.display = 'none';
                activeLightLabels = [...liveLightLabels];
                activeLightData = [...liveLightSeries];
                renderLightChart();
            } else {
                let resultsCount = 96; // 24h
                if (period === '3d') resultsCount = 288;
                if (period === '7d') resultsCount = 672;
                fetchThingSpeakLightHistory(resultsCount);
            }
        }

        function renderLightChart() {
            if (lightAreaChartInstance) {
                lightAreaChartInstance.data.labels = activeLightLabels;
                lightAreaChartInstance.data.datasets[0].data = activeLightData;
                lightAreaChartInstance.update();
            }
            updateLightStats();
            updateSunshineTimeline(activeLightLabels, activeLightData);
        }

        async function fetchThingSpeakLightHistory(resultsCount) {
            const chanId = document.getElementById('selStation').value;
            const cfg = TS_CONFIG[chanId];
            if (!cfg) return;

            const loadingEl = document.getElementById('lightChartLoading');
            const noticeEl = document.getElementById('lightChartNotice');
            loadingEl.style.display = 'block';
            noticeEl.style.display = 'none';

            const url = `https://api.thingspeak.com/channels/${chanId}/feeds.json?api_key=${cfg.key}&results=${resultsCount}&status=true`;

            try {
                const res = await fetch(url);
                if (!res.ok) throw new Error("Chyba odpovede z ThingSpeak API");
                const data = await res.json();

                if (!data.feeds || data.feeds.length === 0) {
                    throw new Error("Žiadne záznamy.");
                }

                let labels = [];
                let lightSeries = [];

                for (const feed of data.feeds) {
                    if (!feed.created_at) continue;
                    
                    const d = new Date(feed.created_at);
                    let label = d.toLocaleTimeString('sk-SK', { hour: '2-digit', minute: '2-digit' });
                    if (resultsCount > 96) {
                        label = `${d.getDate()}.${d.getMonth() + 1}. ` + label;
                    }

                    let lVal = null;
                    if (feed.status) {
                        try {
                            const stObj = JSON.parse(feed.status);
                            if (stObj.light !== undefined && stObj.light !== null && stObj.light !== "") {
                                lVal = parseFloat(stObj.light);
                            }
                        } catch(e) {}
                    }

                    labels.push(label);
                    lightSeries.push(lVal !== null && !isNaN(lVal) ? lVal : 0);
                }

                loadingEl.style.display = 'none';

                if (labels.length === 0) {
                    noticeEl.innerText = `ℹ️ Stanica ${cfg.name} nemá v zvolenom období záznamy o svetle.`;
                    noticeEl.style.display = 'block';
                    return;
                }

                activeLightLabels = labels;
                activeLightData = lightSeries;
                renderLightChart();

            } catch (err) {
                loadingEl.style.display = 'none';
                noticeEl.innerText = `⚠️ Nepodarilo sa načítať dáta svetla: ${err.message}`;
                noticeEl.style.display = 'block';
            }
        }

        function addLiveLightSample(timeLabel, lightVal) {
            const val = (lightVal !== null && !isNaN(lightVal)) ? lightVal : 0;
            liveLightLabels.push(timeLabel);
            liveLightSeries.push(val);

            if (liveLightLabels.length > 25) {
                liveLightLabels.shift();
                liveLightSeries.shift();
            }

            if (lightPeriod === 'live') {
                activeLightLabels = [...liveLightLabels];
                activeLightData = [...liveLightSeries];
                renderLightChart();
            }
        }

        // ================= ZRÁŽKY A HYETOGRAF =================
        let rainViewMode = 'bars'; // 'bars' | 'rate' | 'cumulative'
        let rainPeriod = 'live';   // 'live' | '24h' | '3d' | '7d'
        let rainTooltipEnabled = true;

        let liveRainLabels = ['08:00', '08:30', '09:00', '09:30', '10:00', '10:30', '11:00', '11:30', '12:00', '12:30', '13:00', '13:15'];
        let liveRain15m = [0.0, 0.0, 0.28, 0.56, 1.12, 0.84, 0.28, 0.0, 0.0, 0.0, 0.0, 0.0];
        let liveRainCumulative = [0.0, 0.0, 0.28, 0.84, 1.96, 2.80, 3.08, 3.08, 3.08, 3.08, 3.08, 3.08];
        let liveRainRate = [0.0, 0.0, 1.12, 2.24, 4.48, 3.36, 1.12, 0.0, 0.0, 0.0, 0.0, 0.0];

        let activeRainLabels = [...liveRainLabels];
        let activeRain15m = [...liveRain15m];
        let activeRainCumulative = [...liveRainCumulative];
        let activeRainRate = [...liveRainRate];

        let rainBarChartInstance = null;

        function updateRainGauge(rainToday, rainIntensity) {
            const levelEl = document.getElementById('rainWaterLevel');
            const waveEl = document.getElementById('rainWaterWave');
            if (!levelEl) return;
            const val = (rainToday !== null && !isNaN(rainToday)) ? Math.max(0, rainToday) : 0;
            // Max scale 25 mm = 46px height
            const maxMm = 25.0;
            const ratio = Math.min(1.0, val / maxMm);
            const h = ratio * 46;
            const y = 50 - h;
            levelEl.setAttribute('y', y);
            levelEl.setAttribute('height', h);
            if (waveEl) {
                waveEl.setAttribute('d', `M 6 ${y} Q 11 ${y - 2} 17 ${y} T 28 ${y} L 28 50 L 6 50 Z`);
            }
        }

        function initRainBarChart() {
            const ctx = document.getElementById('rainBarChart');
            if (!ctx) return;

            rainBarChartInstance = new Chart(ctx.getContext('2d'), {
                type: 'bar',
                data: {
                    labels: activeRainLabels,
                    datasets: [
                        {
                            label: 'Zrážky (mm)',
                            data: activeRain15m,
                            backgroundColor: 'rgba(56, 189, 248, 0.65)',
                            borderColor: '#38bdf8',
                            borderWidth: 1.5,
                            borderRadius: 4,
                            yAxisID: 'y'
                        },
                        {
                            label: 'Kumulatívny úhrn (mm)',
                            data: activeRainCumulative,
                            type: 'line',
                            borderColor: '#818cf8',
                            backgroundColor: 'rgba(129, 140, 248, 0.15)',
                            borderWidth: 2.5,
                            pointRadius: 3,
                            pointBackgroundColor: '#818cf8',
                            tension: 0.25,
                            fill: false,
                            yAxisID: 'y'
                        }
                    ]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    interaction: { mode: 'index', intersect: false },
                    plugins: {
                        legend: {
                            display: true,
                            position: 'top',
                            labels: { color: '#94a3b8', font: { family: 'Inter', size: 11 }, usePointStyle: true, boxWidth: 8 }
                        },
                        tooltip: {
                            enabled: rainTooltipEnabled,
                            backgroundColor: 'rgba(15, 23, 42, 0.92)',
                            titleColor: '#f8fafc',
                            bodyColor: '#94a3b8',
                            borderColor: 'rgba(255, 255, 255, 0.1)',
                            borderWidth: 1,
                            padding: 10,
                            callbacks: {
                                label: function(ctx) {
                                    const val = ctx.parsed.y;
                                    return ` ${ctx.dataset.label}: ${val !== null ? val.toFixed(2) + ' mm' : '--'}`;
                                }
                            }
                        }
                    },
                    scales: {
                        x: {
                            grid: { color: 'rgba(255, 255, 255, 0.05)' },
                            ticks: { color: '#94a3b8', font: { size: 10 } }
                        },
                        y: {
                            beginAtZero: true,
                            grid: { color: 'rgba(255, 255, 255, 0.06)' },
                            ticks: {
                                color: '#94a3b8',
                                callback: val => val.toFixed(1) + ' mm'
                            },
                            title: { display: true, text: 'Zrážky (mm)', color: '#64748b', font: { size: 10 } }
                        }
                    }
                }
            });
            updateRainStats();
        }

        function toggleRainTooltip() {
            rainTooltipEnabled = !rainTooltipEnabled;
            const btn = document.getElementById('btnToggleRainTooltip');
            const lbl = document.getElementById('lblRainTooltip');
            if (rainTooltipEnabled) {
                btn.classList.add('active');
                lbl.innerText = 'ZAP';
            } else {
                btn.classList.remove('active');
                lbl.innerText = 'VYP';
            }
            if (rainBarChartInstance) {
                rainBarChartInstance.options.plugins.tooltip.enabled = rainTooltipEnabled;
                rainBarChartInstance.update('none');
            }
        }

        function setRainViewMode(mode) {
            rainViewMode = mode;
            ['btnRainModeBars', 'btnRainModeRate', 'btnRainModeCumulative'].forEach(id => {
                const el = document.getElementById(id);
                if (el) el.classList.remove('active');
            });

            if (mode === 'bars') {
                document.getElementById('btnRainModeBars').classList.add('active');
                if (rainBarChartInstance) {
                    rainBarChartInstance.data.datasets[0].hidden = false;
                    rainBarChartInstance.data.datasets[1].hidden = false;
                    rainBarChartInstance.data.datasets[0].label = 'Zrážky (mm)';
                    rainBarChartInstance.data.datasets[0].data = activeRain15m;
                    rainBarChartInstance.options.scales.y.title.text = 'Zrážky (mm)';
                }
            } else if (mode === 'rate') {
                document.getElementById('btnRainModeRate').classList.add('active');
                if (rainBarChartInstance) {
                    rainBarChartInstance.data.datasets[0].hidden = false;
                    rainBarChartInstance.data.datasets[1].hidden = true;
                    rainBarChartInstance.data.datasets[0].label = 'Intenzita (mm/h)';
                    rainBarChartInstance.data.datasets[0].data = activeRainRate;
                    rainBarChartInstance.options.scales.y.title.text = 'Intenzita (mm/h)';
                }
            } else if (mode === 'cumulative') {
                document.getElementById('btnRainModeCumulative').classList.add('active');
                if (rainBarChartInstance) {
                    rainBarChartInstance.data.datasets[0].hidden = true;
                    rainBarChartInstance.data.datasets[1].hidden = false;
                    rainBarChartInstance.data.datasets[1].label = 'Kumulatívny úhrn (mm)';
                    rainBarChartInstance.data.datasets[1].data = activeRainCumulative;
                    rainBarChartInstance.options.scales.y.title.text = 'Suma (mm)';
                }
            }
            if (rainBarChartInstance) rainBarChartInstance.update();
        }

        function setRainPeriod(period) {
            rainPeriod = period;
            ['btnRainPeriodLive', 'btnRainPeriod24h', 'btnRainPeriod3d', 'btnRainPeriod7d'].forEach(id => {
                const el = document.getElementById(id);
                if (el) el.classList.remove('active');
            });

            if (period === 'live') document.getElementById('btnRainPeriodLive').classList.add('active');
            if (period === '24h') document.getElementById('btnRainPeriod24h').classList.add('active');
            if (period === '3d') document.getElementById('btnRainPeriod3d').classList.add('active');
            if (period === '7d') document.getElementById('btnRainPeriod7d').classList.add('active');

            document.getElementById('rainChartNotice').style.display = 'none';

            if (period === 'live') {
                document.getElementById('rainChartLoading').style.display = 'none';
                activeRainLabels = [...liveRainLabels];
                activeRain15m = [...liveRain15m];
                activeRainCumulative = [...liveRainCumulative];
                activeRainRate = [...liveRainRate];
                renderRainChart();
            } else {
                let resultsCount = 96; // 24h
                if (period === '3d') resultsCount = 288;
                if (period === '7d') resultsCount = 672;
                fetchThingSpeakRainHistory(resultsCount);
            }
        }

        function renderRainChart() {
            if (!rainBarChartInstance) return;
            rainBarChartInstance.data.labels = activeRainLabels;
            rainBarChartInstance.data.datasets[0].data = (rainViewMode === 'rate') ? activeRainRate : activeRain15m;
            rainBarChartInstance.data.datasets[1].data = activeRainCumulative;
            rainBarChartInstance.update();
            updateRainStats();
        }

        function updateRainStats() {
            const sumToday = activeRain15m.reduce((acc, v) => acc + (v || 0), 0);
            const peakRate = Math.max(...activeRainRate.map(v => v || 0), 0);
            const last15m = (activeRain15m.length > 0 && activeRain15m[activeRain15m.length - 1] !== null) ? activeRain15m[activeRain15m.length - 1] : 0;
            const tips = Math.round(sumToday / 0.2794);

            const elToday = document.getElementById('statRainToday');
            if (elToday) elToday.innerText = sumToday.toFixed(2) + ' mm';
            const el15m = document.getElementById('statRain15m');
            if (el15m) el15m.innerText = last15m.toFixed(2) + ' mm';
            const elPeak = document.getElementById('statRainPeakRate');
            if (elPeak) elPeak.innerText = peakRate.toFixed(2) + ' mm/h';
            const elTips = document.getElementById('statRainTips');
            if (elTips) elTips.innerText = tips + ' tipov';
        }

        async function fetchThingSpeakRainHistory(resultsCount) {
            const chanId = document.getElementById('selStation').value;
            const cfg = TS_CONFIG[chanId];
            if (!cfg) return;

            const loadingEl = document.getElementById('rainChartLoading');
            const noticeEl = document.getElementById('rainChartNotice');
            loadingEl.style.display = 'block';
            noticeEl.style.display = 'none';

            const url = `https://api.thingspeak.com/channels/${chanId}/feeds.json?api_key=${cfg.key}&results=${resultsCount}&status=true`;

            try {
                const res = await fetch(url);
                if (!res.ok) throw new Error("Chyba odpovede z ThingSpeak API");
                const data = await res.json();

                if (!data.feeds || data.feeds.length === 0) {
                    throw new Error("Žiadne záznamy.");
                }

                let labels = [];
                let rain15mSeries = [];
                let rainCumulSeries = [];
                let rainRateSeries = [];
                let runningSum = 0;

                for (const feed of data.feeds) {
                    if (!feed.created_at) continue;
                    
                    const d = new Date(feed.created_at);
                    let label = d.toLocaleTimeString('sk-SK', { hour: '2-digit', minute: '2-digit' });
                    if (resultsCount > 96) {
                        label = `${d.getDate()}.${d.getMonth() + 1}. ` + label;
                    }

                    let r15 = (feed.field8 !== null && feed.field8 !== undefined && feed.field8 !== "") ? parseFloat(feed.field8) : 0.0;
                    if (isNaN(r15) || r15 < 0 || r15 > 500) r15 = 0.0;

                    runningSum += r15;
                    const rRate = r15 * 4.0; // 15-min úhrn * 4 = hodinová intenzita

                    labels.push(label);
                    rain15mSeries.push(r15);
                    rainCumulSeries.push(parseFloat(runningSum.toFixed(2)));
                    rainRateSeries.push(parseFloat(rRate.toFixed(2)));
                }

                loadingEl.style.display = 'none';

                if (labels.length === 0) {
                    noticeEl.innerText = `ℹ️ Stanica ${cfg.name} nemá v zvolenom období záznamy o zrážkach.`;
                    noticeEl.style.display = 'block';
                    return;
                }

                activeRainLabels = labels;
                activeRain15m = rain15mSeries;
                activeRainCumulative = rainCumulSeries;
                activeRainRate = rainRateSeries;
                renderRainChart();

            } catch (err) {
                loadingEl.style.display = 'none';
                noticeEl.innerText = `⚠️ Nepodarilo sa načítať dáta zrážok: ${err.message}`;
                noticeEl.style.display = 'block';
            }
        }

        function addLiveRainSample(timeLabel, rain15m, rainToday, rainRate) {
            const val15 = (rain15m !== null && !isNaN(rain15m)) ? rain15m : 0;
            const valCum = (rainToday !== null && !isNaN(rainToday)) ? rainToday : 0;
            const valRate = (rainRate !== null && !isNaN(rainRate)) ? rainRate : 0;

            liveRainLabels.push(timeLabel);
            liveRain15m.push(val15);
            liveRainCumulative.push(valCum);
            liveRainRate.push(valRate);

            if (liveRainLabels.length > 25) {
                liveRainLabels.shift();
                liveRain15m.shift();
                liveRainCumulative.shift();
                liveRainRate.shift();
            }

            if (rainPeriod === 'live') {
                activeRainLabels = [...liveRainLabels];
                activeRain15m = [...liveRain15m];
                activeRainCumulative = [...liveRainCumulative];
                activeRainRate = [...liveRainRate];
                renderRainChart();
            }
        }

        // ================= GRAF RÝCHLOSTÍ A NÁRAZOV VETRA (ALADIN / SHMÚ ŠTÝL) =================
        let windSpeedPeriod = 'live'; // 'live' | '24h' | '3d' | '7d'
        let gustFilterMode = 'significant'; // 'significant' (>1.3x) | 'high' (>1.5x) | 'all' | 'none'
        let windSpeedTooltipEnabled = true;

        let liveSpeedLabels = ['08:00', '08:30', '09:00', '09:30', '10:00', '10:30', '11:00', '11:30', '12:00', '12:30', '13:00', '13:15'];
        let liveSpeedAvg = [1.2, 2.0, 3.5, 4.2, 5.0, 6.5, 5.8, 4.0, 2.5, 1.8, 1.0, 0.8];
        let liveSpeedGust = [2.0, 3.2, 5.0, 6.0, 7.5, 9.2, 8.0, 5.5, 3.8, 2.6, 1.5, 1.2];

        let rawSpeedLabels = [...liveSpeedLabels];
        let rawSpeedAvg = [...liveSpeedAvg];
        let rawSpeedGust = [...liveSpeedGust];

        let windSpeedChartInstance = null;

        function getFilteredGustData(speeds, gusts, mode) {
            return gusts.map((g, i) => {
                const avg = speeds[i];
                if (g === null || isNaN(g) || g <= 0) return null;
                if (avg === null || isNaN(avg)) return g;
                if (mode === 'none') return null;
                if (mode === 'all') return g > avg ? g : null;
                if (mode === 'high') {
                    return (g >= avg * 1.5 && (g - avg) >= 2.0) ? g : null;
                }
                // default: 'significant' (SHMÚ ALADIN) - náraz > 1.3x priemer a rozdiel min 1.0 m/s, alebo pri vánku > 2.0 m/s
                return ((g >= avg * 1.3 && (g - avg) >= 1.0) || (avg < 1.0 && g >= 2.0)) ? g : null;
            });
        }

        function initWindSpeedChart() {
            const ctx = document.getElementById('windSpeedChart').getContext('2d');
            const filteredGusts = getFilteredGustData(rawSpeedAvg, rawSpeedGust, gustFilterMode);

            windSpeedChartInstance = new Chart(ctx, {
                data: {
                    labels: rawSpeedLabels,
                    datasets: [
                        {
                            type: 'bar',
                            label: 'Náraz vetra (GUST)',
                            data: filteredGusts,
                            backgroundColor: 'rgba(244, 63, 94, 0.65)',
                            borderColor: '#f43f5e',
                            borderWidth: 1.5,
                            borderRadius: { topLeft: 4, topRight: 4, bottomLeft: 0, bottomRight: 0 },
                            barPercentage: 0.35,
                            order: 2
                        },
                        {
                            type: 'line',
                            label: 'Priemerná rýchlosť',
                            data: rawSpeedAvg,
                            borderColor: '#38bdf8',
                            backgroundColor: 'rgba(56, 189, 248, 0.14)',
                            fill: true,
                            tension: 0.35,
                            borderWidth: 2.2,
                            pointRadius: 2,
                            pointHoverRadius: 6,
                            pointBackgroundColor: '#38bdf8',
                            order: 1
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
                            min: 0,
                            grace: '10%',
                            grid: {
                                color: 'rgba(255, 255, 255, 0.06)'
                            },
                            ticks: {
                                color: '#94a3b8',
                                font: { size: 11, family: 'Inter' },
                                callback: function(val) {
                                    return val.toFixed(1) + ' m/s';
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
                            enabled: windSpeedTooltipEnabled,
                            backgroundColor: 'rgba(15, 23, 42, 0.92)',
                            titleColor: '#38bdf8',
                            titleFont: { weight: '700', size: 12 },
                            bodyFont: { size: 12 },
                            borderColor: 'rgba(56, 189, 248, 0.3)',
                            borderWidth: 1,
                            padding: 10,
                            callbacks: {
                                label: function(context) {
                                    if (context.parsed.y === null || context.parsed.y === undefined) return null;
                                    const valMs = parseFloat(context.parsed.y);
                                    const valKmh = (valMs * 3.6).toFixed(1);
                                    return ` ${context.dataset.label}: ${valMs.toFixed(1)} m/s (${valKmh} km/h)`;
                                }
                            }
                        }
                    }
                }
            });

            updateWindSpeedStats();
        }

        function updateWindSpeedStats() {
            const validAvg = rawSpeedAvg.filter(v => v !== null && !isNaN(v) && v >= 0);
            const validGust = rawSpeedGust.filter(v => v !== null && !isNaN(v) && v >= 0);

            if (validAvg.length > 0) {
                const lastSpd = validAvg[validAvg.length - 1];
                const avgSpd = (validAvg.reduce((a, b) => a + b, 0) / validAvg.length).toFixed(1);
                document.getElementById('statSpeedCurrent').innerText = `${lastSpd.toFixed(1)} m/s`;
                document.getElementById('statSpeedAvg').innerText = `${avgSpd} m/s`;
            } else {
                document.getElementById('statSpeedCurrent').innerText = `-- m/s`;
                document.getElementById('statSpeedAvg').innerText = `-- m/s`;
            }

            if (validGust.length > 0) {
                const maxGustVal = Math.max(...validGust);
                const maxGustKmh = (maxGustVal * 3.6).toFixed(1);
                document.getElementById('statSpeedMaxGust').innerText = `${maxGustVal.toFixed(1)} m/s (${maxGustKmh} km/h)`;
            } else {
                document.getElementById('statSpeedMaxGust').innerText = `-- m/s`;
            }

            const filteredGusts = getFilteredGustData(rawSpeedAvg, rawSpeedGust, gustFilterMode);
            const gustCount = filteredGusts.filter(g => g !== null && g > 0).length;
            document.getElementById('statSpeedGustCount').innerText = `${gustCount}`;
        }

        function setGustFilter(mode) {
            gustFilterMode = mode;

            ['btnGustThresholdSig', 'btnGustThresholdHigh', 'btnGustThresholdAll', 'btnGustThresholdNone'].forEach(id => {
                const el = document.getElementById(id);
                if (el) el.classList.remove('active');
            });

            if (mode === 'significant') document.getElementById('btnGustThresholdSig')?.classList.add('active');
            if (mode === 'high') document.getElementById('btnGustThresholdHigh')?.classList.add('active');
            if (mode === 'all') document.getElementById('btnGustThresholdAll')?.classList.add('active');
            if (mode === 'none') document.getElementById('btnGustThresholdNone')?.classList.add('active');

            renderWindSpeedChart();
        }

        function toggleWindSpeedTooltip() {
            windSpeedTooltipEnabled = !windSpeedTooltipEnabled;
            const btn = document.getElementById('btnToggleWindSpeedTooltip');
            const lbl = document.getElementById('lblWindSpeedTooltip');
            if (btn) btn.classList.toggle('active', windSpeedTooltipEnabled);
            if (lbl) lbl.innerText = windSpeedTooltipEnabled ? 'ZAP' : 'VYP';
            if (windSpeedChartInstance) {
                windSpeedChartInstance.options.plugins.tooltip.enabled = windSpeedTooltipEnabled;
                windSpeedChartInstance.update('none');
            }
        }

        function setWindSpeedPeriod(period) {
            windSpeedPeriod = period;

            ['btnWindSpeedPeriodLive', 'btnWindSpeedPeriod24h', 'btnWindSpeedPeriod3d', 'btnWindSpeedPeriod7d'].forEach(id => {
                const el = document.getElementById(id);
                if (el) el.classList.remove('active');
            });

            if (period === 'live') document.getElementById('btnWindSpeedPeriodLive')?.classList.add('active');
            if (period === '24h') document.getElementById('btnWindSpeedPeriod24h')?.classList.add('active');
            if (period === '3d') document.getElementById('btnWindSpeedPeriod3d')?.classList.add('active');
            if (period === '7d') document.getElementById('btnWindSpeedPeriod7d')?.classList.add('active');

            document.getElementById('windSpeedChartNotice').style.display = 'none';

            if (period === 'live') {
                document.getElementById('windSpeedChartLoading').style.display = 'none';
                rawSpeedLabels = [...liveSpeedLabels];
                rawSpeedAvg = [...liveSpeedAvg];
                rawSpeedGust = [...liveSpeedGust];
                renderWindSpeedChart();
            } else {
                let resultsCount = 96; // 24h
                if (period === '3d') resultsCount = 288;
                if (period === '7d') resultsCount = 672;
                fetchThingSpeakWindSpeedHistory(resultsCount);
            }
        }

        function renderWindSpeedChart() {
            if (!windSpeedChartInstance) return;

            const filteredGusts = getFilteredGustData(rawSpeedAvg, rawSpeedGust, gustFilterMode);

            windSpeedChartInstance.data.labels = rawSpeedLabels;
            windSpeedChartInstance.data.datasets[0].data = filteredGusts;
            windSpeedChartInstance.data.datasets[1].data = rawSpeedAvg;

            windSpeedChartInstance.update();
            updateWindSpeedStats();
        }

        async function fetchThingSpeakWindSpeedHistory(resultsCount) {
            const chanId = document.getElementById('selStation').value;
            const cfg = TS_CONFIG[chanId];
            if (!cfg) return;

            const loadingEl = document.getElementById('windSpeedChartLoading');
            const noticeEl = document.getElementById('windSpeedChartNotice');
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
                let speedSeries = [];
                let gustSeries = [];

                for (const feed of data.feeds) {
                    if (!feed.created_at) continue;
                    
                    const d = new Date(feed.created_at);
                    let label = d.toLocaleTimeString('sk-SK', { hour: '2-digit', minute: '2-digit' });
                    if (resultsCount > 96) {
                        label = `${d.getDate()}.${d.getMonth() + 1}. ` + label;
                    }

                    let spd = (feed.field6 !== null && feed.field6 !== undefined && feed.field6 !== "") ? parseFloat(feed.field6) : null;
                    let gust = (feed.field7 !== null && feed.field7 !== undefined && feed.field7 !== "") ? parseFloat(feed.field7) : null;

                    // Plauzibilný filter rýchlostí (0.0 až 50.0 m/s)
                    if (spd !== null && (isNaN(spd) || spd < 0 || spd > 50.0)) spd = null;
                    if (gust !== null && (isNaN(gust) || gust < 0 || gust > 55.0)) gust = null;

                    // Ak nemáme gust, ale máme rýchlosť, nastavíme gust na rýchlosť
                    if (gust === null && spd !== null) gust = spd;

                    labels.push(label);
                    speedSeries.push(spd);
                    gustSeries.push(gust);
                }

                loadingEl.style.display = 'none';

                if (labels.length === 0) {
                    noticeEl.innerText = `ℹ️ Stanica ${cfg.name} nemá v zvolenom období záznamy o rýchlosti vetra.`;
                    noticeEl.style.display = 'block';
                    return;
                }

                rawSpeedLabels = labels;
                rawSpeedAvg = speedSeries;
                rawSpeedGust = gustSeries;
                renderWindSpeedChart();

            } catch (err) {
                loadingEl.style.display = 'none';
                noticeEl.innerText = `⚠️ Nepodarilo sa načítať rýchlosť vetra: ${err.message}`;
                noticeEl.style.display = 'block';
            }
        }

        function addLiveWindSpeedSample(timeLabel, speed, maxSpeed) {
            if (speed === null || isNaN(speed) || speed < 0 || speed > 50.0) return;
            const gust = (maxSpeed !== null && !isNaN(maxSpeed) && maxSpeed >= speed) ? maxSpeed : speed;

            liveSpeedLabels.push(timeLabel);
            liveSpeedAvg.push(speed);
            liveSpeedGust.push(gust);

            if (liveSpeedLabels.length > 25) {
                liveSpeedLabels.shift();
                liveSpeedAvg.shift();
                liveSpeedGust.shift();
            }

            if (windSpeedPeriod === 'live') {
                rawSpeedLabels = [...liveSpeedLabels];
                rawSpeedAvg = [...liveSpeedAvg];
                rawSpeedGust = [...liveSpeedGust];
                renderWindSpeedChart();
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
            // Plauzibilný filter: smer 0..360, rýchlosť min 0.3 m/s (bezvetrie nezapočítavame), max 40 m/s
            if (dirDeg === undefined || isNaN(dirDeg) || dirDeg < 0 || dirDeg > 360) return;
            if (speed === undefined || isNaN(speed) || speed < 0.3 || speed > 40.0) return;

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
            if (windTimelinePeriod !== 'live') {
                setWindTimelinePeriod(windTimelinePeriod);
            }
            if (windSpeedPeriod !== 'live') {
                setWindSpeedPeriod(windSpeedPeriod);
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

                    // Bezvetrie (< 0.3 m/s) podľa Beaufort 0 / WMO nezapočítavame do smerov
                    if (speed < 0.3 && speedMax < 0.3) continue;

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

        // ================= ČASOVÝ VÝVOJ SMERU VETRA (TIMELINE) =================
        let windTimelinePeriod = 'live'; // 'live' | '24h' | '3d' | '7d'
        let windTimelineTooltipEnabled = true;

        let liveTimelineLabels = ['08:00', '08:30', '09:00', '09:30', '10:00', '10:30', '11:00', '11:30', '12:00', '12:30', '13:00', '13:15'];
        let liveTimelineDegs = [225, 240, 260, 270, 290, 315, 330, 345, 0, 15, 45, 60];
        let liveTimelineSpeeds = [1.2, 2.0, 3.5, 4.2, 5.0, 6.5, 5.8, 4.0, 2.5, 1.8, 1.0, 0.8];
        let liveTimelineMaxSpeeds = [2.0, 3.2, 5.0, 6.0, 7.5, 9.2, 8.0, 5.5, 3.8, 2.6, 1.5, 1.2];

        let activeTimelineLabels = [...liveTimelineLabels];
        let activeTimelineDegs = [...liveTimelineDegs];
        let activeTimelineSpeeds = [...liveTimelineSpeeds];
        let activeTimelineMaxSpeeds = [...liveTimelineMaxSpeeds];

        let windTimelineChartInstance = null;

        const DIR_Y_TICKS = {
            0: 'S (0°)',
            45: 'SV (45°)',
            90: 'V (90°)',
            135: 'JV (135°)',
            180: 'J (180°)',
            225: 'JZ (225°)',
            270: 'Z (270°)',
            315: 'SZ (315°)',
            360: 'S (360°)'
        };

        function getDirNameFromDeg(deg) {
            if (deg === null || isNaN(deg)) return "--";
            const idx = Math.round(((deg % 360) / 22.5)) % 16;
            return ROSE_DIRS[idx];
        }

        function getSpeedColor(spd) {
            if (spd < 1.5) return '#38bdf8'; // svetlomodrá (vánok)
            if (spd < 4.0) return '#22c55e'; // jasná zelená (mierny)
            if (spd < 8.0) return '#facc15'; // žltá (čerstvý)
            return '#f43f5e';                // červená (silný)
        }

        function initWindTimelineChart() {
            const ctx = document.getElementById('windTimelineChart').getContext('2d');
            
            windTimelineChartInstance = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: activeTimelineLabels,
                    datasets: [{
                        label: 'Smer vetra (°)',
                        data: activeTimelineDegs,
                        showLine: true,
                        borderWidth: 1.2,
                        borderColor: 'rgba(255, 255, 255, 0.12)',
                        borderDash: [3, 4],
                        fill: false,
                        tension: 0.1,
                        pointBackgroundColor: function(context) {
                            const idx = context.dataIndex;
                            const spd = activeTimelineSpeeds[idx] !== undefined ? activeTimelineSpeeds[idx] : 0;
                            return getSpeedColor(spd);
                        },
                        pointBorderColor: '#0f172a',
                        pointBorderWidth: 1.5,
                        pointRadius: function(context) {
                            const idx = context.dataIndex;
                            const spd = activeTimelineSpeeds[idx] !== undefined ? activeTimelineSpeeds[idx] : 0;
                            return Math.max(4, Math.min(8, 4 + spd * 0.4));
                        },
                        pointHoverRadius: 7
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
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
                            min: 0,
                            max: 360,
                            grid: {
                                color: function(context) {
                                    if (context.tick && context.tick.value % 90 === 0) {
                                        return 'rgba(255, 255, 255, 0.14)';
                                    }
                                    return 'rgba(255, 255, 255, 0.04)';
                                }
                            },
                            ticks: {
                                stepSize: 45,
                                color: '#94a3b8',
                                font: { size: 11, family: 'Inter', weight: '500' },
                                callback: function(value) {
                                    return DIR_Y_TICKS[value] || (value + '°');
                                }
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            display: false
                        },
                        tooltip: {
                            enabled: windTimelineTooltipEnabled,
                            backgroundColor: 'rgba(15, 23, 42, 0.92)',
                            titleColor: '#38bdf8',
                            titleFont: { weight: '700', size: 12 },
                            bodyFont: { size: 12 },
                            borderColor: 'rgba(56, 189, 248, 0.3)',
                            borderWidth: 1,
                            padding: 8,
                            callbacks: {
                                title: function(items) {
                                    return `Čas: ${items[0].label}`;
                                },
                                label: function(context) {
                                    const idx = context.dataIndex;
                                    const deg = Math.round(context.parsed.y);
                                    const dirName = getDirNameFromDeg(deg);
                                    const spd = activeTimelineSpeeds[idx] !== undefined ? activeTimelineSpeeds[idx].toFixed(1) : '--';
                                    const maxSpd = activeTimelineMaxSpeeds[idx] !== undefined ? activeTimelineMaxSpeeds[idx].toFixed(1) : spd;
                                    return [
                                        ` Smer: ${dirName} (${deg}°)`,
                                        ` Rýchlosť: ${spd} m/s (Náraz: ${maxSpd} m/s)`
                                    ];
                                }
                            }
                        }
                    }
                }
            });

            updateWindTimelineStats();
        }

        function updateWindTimelineStats() {
            const validDegs = activeTimelineDegs.filter(d => d !== null && !isNaN(d));
            if (validDegs.length === 0) {
                document.getElementById('statTimelineLastDir').innerText = "--";
                document.getElementById('statTimelineAvgDir').innerText = "--";
                document.getElementById('statTimelineMaxSpeed').innerText = "-- m/s";
                document.getElementById('statTimelineSamples').innerText = "0";
                return;
            }

            const lastDeg = Math.round(validDegs[validDegs.length - 1]);
            document.getElementById('statTimelineLastDir').innerText = `${getDirNameFromDeg(lastDeg)} (${lastDeg}°)`;

            // Cirkulárny vektorový priemer smerov vetra
            let sinSum = 0, cosSum = 0;
            for (const d of validDegs) {
                const rad = (d * Math.PI) / 180;
                sinSum += Math.sin(rad);
                cosSum += Math.cos(rad);
            }
            let avgRad = Math.atan2(sinSum, cosSum);
            let avgDeg = Math.round((avgRad * 180) / Math.PI);
            if (avgDeg < 0) avgDeg += 360;
            document.getElementById('statTimelineAvgDir').innerText = `${getDirNameFromDeg(avgDeg)} (${avgDeg}°)`;

            const maxSpd = activeTimelineMaxSpeeds.length > 0 ? Math.max(...activeTimelineMaxSpeeds, 0) : 0;
            document.getElementById('statTimelineMaxSpeed').innerText = `${maxSpd.toFixed(1)} m/s`;
            document.getElementById('statTimelineSamples').innerText = validDegs.length;
        }

        function renderWindTimelineChart() {
            if (!windTimelineChartInstance) return;

            windTimelineChartInstance.data.labels = activeTimelineLabels;
            windTimelineChartInstance.data.datasets[0].data = activeTimelineDegs;
            windTimelineChartInstance.update();
            updateWindTimelineStats();
        }

        function toggleWindTimelineTooltip() {
            windTimelineTooltipEnabled = !windTimelineTooltipEnabled;
            const btn = document.getElementById('btnToggleWindTimelineTooltip');
            const lbl = document.getElementById('lblWindTimelineTooltip');
            if (btn) btn.classList.toggle('active', windTimelineTooltipEnabled);
            if (lbl) lbl.innerText = windTimelineTooltipEnabled ? 'ZAP' : 'VYP';
            if (windTimelineChartInstance) {
                windTimelineChartInstance.options.plugins.tooltip.enabled = windTimelineTooltipEnabled;
                windTimelineChartInstance.update('none');
            }
        }

        function setWindTimelinePeriod(period) {
            windTimelinePeriod = period;

            ['btnWindTimelineLive', 'btnWindTimeline24h', 'btnWindTimeline3d', 'btnWindTimeline7d'].forEach(id => {
                document.getElementById(id).classList.remove('active');
            });

            if (period === 'live') document.getElementById('btnWindTimelineLive').classList.add('active');
            if (period === '24h') document.getElementById('btnWindTimeline24h').classList.add('active');
            if (period === '3d') document.getElementById('btnWindTimeline3d').classList.add('active');
            if (period === '7d') document.getElementById('btnWindTimeline7d').classList.add('active');

            document.getElementById('windTimelineNotice').style.display = 'none';

            if (period === 'live') {
                document.getElementById('windTimelineLoading').style.display = 'none';
                activeTimelineLabels = [...liveTimelineLabels];
                activeTimelineDegs = [...liveTimelineDegs];
                activeTimelineSpeeds = [...liveTimelineSpeeds];
                activeTimelineMaxSpeeds = [...liveTimelineMaxSpeeds];
                renderWindTimelineChart();
            } else {
                let resultsCount = 96; // 24h
                if (period === '3d') resultsCount = 288;
                if (period === '7d') resultsCount = 672;
                fetchThingSpeakWindTimelineHistory(resultsCount);
            }
        }

        async function fetchThingSpeakWindTimelineHistory(resultsCount) {
            const chanId = document.getElementById('selStation').value;
            const cfg = TS_CONFIG[chanId];
            if (!cfg) return;

            const loadingEl = document.getElementById('windTimelineLoading');
            const noticeEl = document.getElementById('windTimelineNotice');
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
                let degs = [];
                let speeds = [];
                let maxSpeeds = [];

                for (const feed of data.feeds) {
                    if (!feed.created_at || feed.field5 === null || feed.field5 === undefined || feed.field5 === "") continue;
                    
                    const dirDeg = parseFloat(feed.field5);
                    if (isNaN(dirDeg) || dirDeg < 0 || dirDeg > 360) continue;

                    let speed = feed.field6 ? parseFloat(feed.field6) : 0.0;
                    let speedMax = feed.field7 ? parseFloat(feed.field7) : speed;
                    if (isNaN(speed) || speed < 0 || speed > 40.0) continue;
                    if (isNaN(speedMax) || speedMax < 0 || speedMax > 45.0) speedMax = speed;

                    // Filter bezvetria: Podľa WMO / Beaufort 0 je vietor < 0.3 m/s bezvetrie
                    // Mechanická smerovka pri bezvetrí len visí, preto smer nekreslíme
                    if (speed < 0.3 && speedMax < 0.3) continue;

                    const d = new Date(feed.created_at);
                    let label = d.toLocaleTimeString('sk-SK', { hour: '2-digit', minute: '2-digit' });
                    if (resultsCount > 96) {
                        label = `${d.getDate()}.${d.getMonth() + 1}. ` + label;
                    }

                    labels.push(label);
                    degs.push(dirDeg);
                    speeds.push(speed);
                    maxSpeeds.push(speedMax);
                }

                loadingEl.style.display = 'none';

                if (labels.length === 0) {
                    noticeEl.innerText = `ℹ️ Stanica ${cfg.name} nemá v zvolenom období záznamy o smere vetra (alebo pretrvávalo bezvetrie < 0.3 m/s).`;
                    noticeEl.style.display = 'block';
                    return;
                }

                activeTimelineLabels = labels;
                activeTimelineDegs = degs;
                activeTimelineSpeeds = speeds;
                activeTimelineMaxSpeeds = maxSpeeds;
                renderWindTimelineChart();

            } catch (err) {
                loadingEl.style.display = 'none';
                noticeEl.innerText = `⚠️ Nepodarilo sa načítať smer vetra: ${err.message}`;
                noticeEl.style.display = 'block';
            }
        }

        function addLiveWindTimelineSample(timeLabel, dirDeg, speed, maxSpeed) {
            if (dirDeg === null || isNaN(dirDeg) || dirDeg < 0 || dirDeg > 360) return;
            if (speed === null || isNaN(speed) || speed < 0 || speed > 40.0) return;
            // Pri bezvetrí (< 0.3 m/s) smer nekreslíme
            if (speed < 0.3 && (!maxSpeed || maxSpeed < 0.3)) return;

            liveTimelineLabels.push(timeLabel);
            liveTimelineDegs.push(dirDeg);
            liveTimelineSpeeds.push(speed);
            liveTimelineMaxSpeeds.push(maxSpeed || speed);

            if (liveTimelineLabels.length > 25) {
                liveTimelineLabels.shift();
                liveTimelineDegs.shift();
                liveTimelineSpeeds.shift();
                liveTimelineMaxSpeeds.shift();
            }

            if (windTimelinePeriod === 'live') {
                activeTimelineLabels = [...liveTimelineLabels];
                activeTimelineDegs = [...liveTimelineDegs];
                activeTimelineSpeeds = [...liveTimelineSpeeds];
                activeTimelineMaxSpeeds = [...liveTimelineMaxSpeeds];
                renderWindTimelineChart();
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

                if (data.lightPercent !== undefined) {
                    document.getElementById('lightPercent').innerHTML = data.lightPercent.toFixed(0) + ' <span class="unit">%</span>';
                    document.getElementById('skyConditionBadge').innerText = data.skyCondition || '--';
                    document.getElementById('sunshineDuration').innerText = 'Dnešný svit: ' + (data.sunshineDuration || '--');
                    updateSolarGauge(data.lightPercent, data.skyCondition);
                }

                if (data.rainToday !== undefined) {
                    document.getElementById('rainToday').innerHTML = data.rainToday.toFixed(2) + ' <span class="unit">mm</span>';
                    document.getElementById('rainIntensityBadge').innerText = data.rainIntensity || 'Bez zrážok';
                    document.getElementById('rainSub').innerText = '15 min: ' + data.rain15m.toFixed(2) + ' mm • ' + (data.rainPulsesToday !== undefined ? data.rainPulsesToday : 0) + ' tipov';
                    updateRainGauge(data.rainToday, data.rainIntensity);
                }

                // Table Update
                document.getElementById('tblTempIn').innerText = tempInFormatted;
                document.getElementById('tblTempOut').innerText = tempOutFormatted;
                document.getElementById('tblWindSpeed').innerText = speedFormatted;
                document.getElementById('tblWindDirDeg').innerText = degFormatted;
                document.getElementById('tblWindDirName').innerText = data.windDirName;
                if (document.getElementById('tblLight')) {
                    document.getElementById('tblLight').innerText = (data.lightPercent !== undefined) ? (data.lightPercent.toFixed(1) + ' % (' + data.skyCondition + ')') : '--';
                }
                if (document.getElementById('tblSun')) {
                    document.getElementById('tblSun').innerText = data.sunshineDuration || '--';
                }
                if (document.getElementById('tblRainToday')) {
                    document.getElementById('tblRainToday').innerText = (data.rainToday !== undefined) ? (data.rainToday.toFixed(2) + ' mm (' + data.rainPulsesToday + ' tipov)') : '--';
                }
                if (document.getElementById('tblRain15m')) {
                    document.getElementById('tblRain15m').innerText = (data.rain15m !== undefined) ? (data.rain15m.toFixed(2) + ' mm') : '--';
                }
                if (document.getElementById('tblRainRate')) {
                    document.getElementById('tblRainRate').innerText = (data.rainRate !== undefined) ? (data.rainRate.toFixed(2) + ' mm/h (' + data.rainIntensity + ')') : '--';
                }
                document.getElementById('tblWifiSSID').innerText = data.wifiSSID;
                document.getElementById('tblWifiRSSI').innerText = data.rssi + ' dBm';
                document.getElementById('tblIp').innerText = data.ip;
                document.getElementById('tblUptime').innerText = data.uptimeSec + ' s';

                if (data.version) {
                    if (document.getElementById('fwVersion')) {
                        document.getElementById('fwVersion').innerText = 'v' + data.version + (data.stationId ? ' (' + data.stationId + ')' : '');
                    }
                    if (document.getElementById('fwBuild') && data.buildDate) {
                        document.getElementById('fwBuild').innerText = data.buildDate + ' ' + (data.buildTime || '');
                    }
                    if (document.getElementById('appTitle')) {
                        document.getElementById('appTitle').innerHTML = '🌤️ wtrStat-02 <span style="font-size: 0.92rem; font-weight: 500; opacity: 0.8; vertical-align: middle;">v' + data.version + '</span>';
                    }
                    document.title = 'wtrStat-02 v' + data.version + ' • ' + (data.stationId || 'Meteostanica');
                }

                const timeLabel = new Date().toLocaleTimeString('sk-SK', { hour: '2-digit', minute: '2-digit', second: '2-digit' });

                // Wind Rose Update
                if (data.windSpeed !== undefined && data.windDirDeg !== undefined) {
                    addWindRoseSample(data.windDirDeg, data.windSpeed);
                }

                // Wind Direction Timeline Update
                if (data.windSpeed !== undefined && data.windDirDeg !== undefined) {
                    addLiveWindTimelineSample(timeLabel, data.windDirDeg, data.windSpeed, data.windSpeed);
                }

                // Wind Speed & Gusts Update
                if (data.windSpeed !== undefined) {
                    addLiveWindSpeedSample(timeLabel, data.windSpeed, data.windSpeedMax || data.windSpeed);
                }

                // Temp Line Chart Update (+Svetlo na pozadí)
                if (data.tempIn !== undefined && data.tempOut !== undefined) {
                    addLiveTempSample(timeLabel, data.tempIn, data.tempOut, data.lightPercent);
                }

                // Light Area Chart Update
                if (data.lightPercent !== undefined) {
                    addLiveLightSample(timeLabel, data.lightPercent);
                }

                // Rain Chart Update
                if (data.rainToday !== undefined) {
                    addLiveRainSample(timeLabel, data.rain15m, data.rainToday, data.rainRate);
                }

            } catch(e) {
                console.error("Fetch error:", e);
            }
        }

        async function simulateRainTip() {
            try {
                const btn = document.getElementById('btnSimRain');
                if (btn) btn.style.transform = 'scale(0.95)';
                const res = await fetch('/api/test/rain-tip', { method: 'POST' });
                setTimeout(() => { if (btn) btn.style.transform = 'none'; }, 150);
                if (res.ok) {
                    fetchLive();
                }
            } catch (e) {
                console.error("Simulation error:", e);
            }
        }

        window.addEventListener('DOMContentLoaded', () => {
            initTempLineChart();
            initLightAreaChart();
            initLightDonutChart();
            initRainBarChart();
            initWindSpeedChart();
            initWindRoseChart();
            initWindTimelineChart();
            fetchLive();
        });

        setInterval(fetchLive, 2000);
    </script>
</body>
</html>
)rawliteral";

static const char UPDATE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="sk">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>wtrStat-02 • Aktualizácia firmvéru</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap');
        :root {
            --bg-gradient: linear-gradient(135deg, #0b1329 0%, #101c3d 50%, #0d1527 100%);
            --card-bg: rgba(22, 33, 62, 0.75);
            --card-border: rgba(255, 255, 255, 0.08);
            --primary: #38bdf8;
            --primary-glow: rgba(56, 189, 248, 0.25);
            --accent: #f43f5e;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --success: #34d399;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Inter', system-ui, sans-serif; }
        body {
            background: var(--bg-gradient);
            color: var(--text-main);
            min-height: 100vh;
            padding: 24px 16px;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .container-box {
            width: 100%;
            max-width: 520px;
            display: flex;
            flex-direction: column;
            gap: 18px;
        }
        .card {
            background: var(--card-bg);
            backdrop-filter: blur(16px);
            border: 1px solid var(--card-border);
            border-radius: 20px;
            padding: 24px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.4);
        }
        h1 {
            font-size: 1.35rem;
            font-weight: 700;
            background: linear-gradient(135deg, #38bdf8 0%, #818cf8 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 4px;
        }
        .sub-desc { color: var(--text-muted); font-size: 0.82rem; margin-bottom: 14px; }
        .info-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 8px;
            background: rgba(255,255,255,0.03);
            border: 1px solid var(--card-border);
            border-radius: 12px;
            padding: 10px 14px;
            margin-bottom: 16px;
            font-size: 0.82rem;
        }
        .info-grid div span { color: var(--text-muted); }
        .info-grid div b { color: var(--primary); }
        
        .section-header {
            font-size: 0.95rem;
            font-weight: 600;
            color: var(--text-main);
            margin-bottom: 10px;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .file-box {
            border: 2px dashed rgba(56, 189, 248, 0.3);
            border-radius: 14px;
            padding: 16px;
            text-align: center;
            background: rgba(56, 189, 248, 0.03);
            cursor: pointer;
            margin-bottom: 14px;
            transition: all 0.2s ease;
        }
        .file-box:hover { border-color: var(--primary); background: rgba(56, 189, 248, 0.08); }
        input[type="file"] { display: none; }
        .file-label { font-size: 0.85rem; font-weight: 600; color: var(--primary); cursor: pointer; }
        .selected-file { font-size: 0.78rem; color: var(--text-muted); margin-top: 4px; word-break: break-all; }
        .btn-action {
            width: 100%;
            padding: 11px;
            background: linear-gradient(135deg, #38bdf8 0%, #2563eb 100%);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 0.9rem;
            font-weight: 600;
            cursor: pointer;
            transition: opacity 0.2s ease;
            box-shadow: 0 4px 15px var(--primary-glow);
        }
        .btn-action:disabled { opacity: 0.4; cursor: not-allowed; }
        .btn-action:hover:not(:disabled) { opacity: 0.9; }
        
        .btn-secondary {
            background: rgba(255, 255, 255, 0.08);
            border: 1px solid var(--card-border);
            box-shadow: none;
        }
        .btn-secondary:hover:not(:disabled) {
            background: rgba(255, 255, 255, 0.15);
            border-color: rgba(56, 189, 248, 0.4);
        }

        .github-box {
            background: rgba(255,255,255,0.02);
            border: 1px solid var(--card-border);
            border-radius: 14px;
            padding: 14px;
            margin-bottom: 14px;
        }
        .gh-status { font-size: 0.82rem; margin-top: 8px; line-height: 1.4; }
        .notes-box { font-size: 0.78rem; color: var(--text-muted); background: rgba(0,0,0,0.25); padding: 8px 10px; border-radius: 8px; margin: 8px 0; }

        .progress-bar-bg {
            background: rgba(255, 255, 255, 0.08);
            border-radius: 10px;
            height: 10px;
            overflow: hidden;
            margin-top: 14px;
            display: none;
        }
        .progress-bar-fill {
            background: linear-gradient(90deg, #38bdf8, #34d399);
            height: 100%;
            width: 0%;
            transition: width 0.2s ease;
        }
        .status-msg {
            margin-top: 12px;
            font-size: 0.82rem;
            text-align: center;
            font-weight: 500;
            display: none;
        }
        .back-link {
            display: block;
            text-align: center;
            margin-top: 6px;
            font-size: 0.82rem;
            color: var(--text-muted);
            text-decoration: none;
        }
        .back-link:hover { color: var(--primary); }
    </style>
</head>
<body>
    <div class="container-box">
        <div class="card">
            <h1>⚡ OTA Aktualizácia Firmvéru</h1>
            <p class="sub-desc">Správa a aktualizácia firmvéru ESP32 na diaľku alebo lokálne.</p>

            <div class="info-grid">
                <div><span>Stanica:</span> <b id="lblSite">--</b></div>
                <div><span>Verzia FW:</span> <b id="lblVer">v)rawliteral" WTRSTAT_FIRMWARE_VERSION R"rawliteral(</b></div>
                <div><span>Zostavené:</span> <b id="lblBuild">)rawliteral" __DATE__ " " __TIME__ R"rawliteral(</b></div>
            </div>

            <!-- GitHub Cloud OTA Section -->
            <div class="section-header">🌐 1. Vzdialený Update z GitHubu</div>
            <div class="github-box">
                <button type="button" class="btn-action btn-secondary" id="btnCheckGh" onclick="checkGitHubUpdate()">🔎 Skontrolovať novú verziu na GitHube</button>
                <div class="gh-status" id="ghStatus" style="display:none;"></div>
                <div id="ghActionBox" style="display:none; margin-top:10px;">
                    <div class="notes-box" id="ghNotes"></div>
                    <button type="button" class="btn-action" id="btnInstallGh" onclick="installGitHubUpdate()">🚀 Inštalovať z GitHubu</button>
                </div>
            </div>

            <!-- Local Upload Section -->
            <div class="section-header" style="margin-top:18px;">📁 2. Manuálne nahratie .bin súboru (z mobilu/PC)</div>
            <form id="uploadForm" enctype="multipart/form-data">
                <div class="file-box" onclick="document.getElementById('firmwareFile').click()">
                    <div class="file-label">📁 Kliknite pre výber lokálneho súboru .bin</div>
                    <div class="selected-file" id="fileName">Žiadny súbor nevybraný</div>
                    <input type="file" id="firmwareFile" accept=".bin" onchange="onFileSelected(this)">
                </div>

                <button type="button" class="btn-action" id="btnSubmitLocal" onclick="uploadLocalFirmware()" disabled>🚀 Nahrať lokálny súbor</button>
            </form>

            <div class="progress-bar-bg" id="pBarBg">
                <div class="progress-bar-fill" id="pBarFill"></div>
            </div>
            <div class="status-msg" id="statusMsg"></div>

            <a href="/" class="back-link">← Späť na Dashboard</a>
        </div>
    </div>

    <script>
        let latestDownloadUrl = '';

        // Načítanie základných informácií o stanici
        fetch('/api/live')
            .then(r => r.json())
            .then(d => {
                document.getElementById('lblSite').textContent = d.stationId || 'TEST';
                if (d.version) document.getElementById('lblVer').textContent = 'v' + d.version;
                if (d.buildDate && document.getElementById('lblBuild')) {
                    document.getElementById('lblBuild').textContent = d.buildDate + ' ' + (d.buildTime || '');
                }
            })
            .catch(() => {});

        fetch('/api/ota/check')
            .then(r => r.json())
            .then(d => {
                document.getElementById('lblVer').textContent = 'v' + (d.currentVersion || '2.0.0');
                if (d.stationId) document.getElementById('lblSite').textContent = d.stationId;
            })
            .catch(() => {});

        function checkGitHubUpdate() {
            const btn = document.getElementById('btnCheckGh');
            const status = document.getElementById('ghStatus');
            const actionBox = document.getElementById('ghActionBox');
            const notes = document.getElementById('ghNotes');

            btn.disabled = true;
            status.style.display = 'block';
            status.style.color = '#38bdf8';
            status.innerHTML = '⏳ Pripájam sa k GitHubu a overujem verziu...';
            actionBox.style.display = 'none';

            fetch('/api/ota/check')
                .then(r => r.json())
                .then(d => {
                    btn.disabled = false;
                    if (d.error && d.error.length > 0) {
                        status.style.color = '#f43f5e';
                        status.innerHTML = '❌ ' + d.error;
                    } else if (d.updateAvailable) {
                        status.style.color = '#34d399';
                        status.innerHTML = '🌟 <b>Dostupná nová verzia: v' + d.newVersion + '</b> (pre stanicu ' + d.stationId + ')';
                        notes.innerHTML = '<b>Poznámky k vydaniu:</b> ' + (d.notes || 'Bez popisu');
                        latestDownloadUrl = d.downloadUrl;
                        actionBox.style.display = 'block';
                    } else {
                        status.style.color = '#38bdf8';
                        status.innerHTML = '✅ <b>Firmvér je aktuálny!</b> (Máte najnovšiu verziu v' + d.currentVersion + ')';
                    }
                })
                .catch(err => {
                    btn.disabled = false;
                    status.style.color = '#f43f5e';
                    status.innerHTML = '❌ Chyba spojenia s ESP32.';
                });
        }

        function installGitHubUpdate() {
            if (!latestDownloadUrl) return;
            if (!confirm('Naozaj chcete spustiť aktualizáciu z GitHubu? ESP32 stiahne nový firmvér a reštartuje sa.')) return;

            const btnInstall = document.getElementById('btnInstallGh');
            const pBarBg = document.getElementById('pBarBg');
            const pBarFill = document.getElementById('pBarFill');
            const statusMsg = document.getElementById('statusMsg');

            btnInstall.disabled = true;
            pBarBg.style.display = 'block';
            pBarFill.style.width = '50%';
            statusMsg.style.display = 'block';
            statusMsg.style.color = '#38bdf8';
            statusMsg.innerHTML = '⏳ ESP32 sťahuje nový firmvér z GitHubu a zapisuje do flash...';

            fetch('/api/ota/cloud-update', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ url: latestDownloadUrl })
            })
            .then(r => r.json())
            .then(d => {
                if (d.status === 'started' || d.status === 'ok') {
                    pBarFill.style.width = '100%';
                    statusMsg.style.color = '#34d399';
                    statusMsg.innerHTML = '✅ <b>Aktualizácia prebieha!</b><br>ESP32 sa reštartuje s novou verziou. Presmerovanie za 15s...';
                    setTimeout(() => { window.location.href = '/'; }, 15000);
                } else {
                    statusMsg.style.color = '#f43f5e';
                    statusMsg.innerHTML = '❌ Aktualizácia zlyhala: ' + (d.error || 'Neznáma chyba');
                    btnInstall.disabled = false;
                }
            })
            .catch(() => {
                // Po reštarte ESP32 spojenie spadne, čo je normálne
                pBarFill.style.width = '100%';
                statusMsg.style.color = '#34d399';
                statusMsg.innerHTML = '✅ <b>Firmvér bol nahratý!</b><br>ESP32 sa reštartuje... Presmerovanie za 15s.';
                setTimeout(() => { window.location.href = '/'; }, 15000);
            });
        }

        function onFileSelected(input) {
            const btn = document.getElementById('btnSubmitLocal');
            const label = document.getElementById('fileName');
            if (input.files && input.files.length > 0) {
                const file = input.files[0];
                label.textContent = file.name + ' (' + (file.size / 1024).toFixed(1) + ' KB)';
                label.style.color = '#38bdf8';
                btn.disabled = false;
            } else {
                label.textContent = 'Žiadny súbor nevybraný';
                label.style.color = '#94a3b8';
                btn.disabled = true;
            }
        }

        function uploadLocalFirmware() {
            const input = document.getElementById('firmwareFile');
            if (!input.files || input.files.length === 0) return;

            const file = input.files[0];
            const formData = new FormData();
            formData.append('update', file);

            const btn = document.getElementById('btnSubmitLocal');
            const pBarBg = document.getElementById('pBarBg');
            const pBarFill = document.getElementById('pBarFill');
            const statusMsg = document.getElementById('statusMsg');

            btn.disabled = true;
            pBarBg.style.display = 'block';
            statusMsg.style.display = 'block';
            statusMsg.style.color = '#38bdf8';
            statusMsg.textContent = '⏳ Nahrávam firmvér do ESP32... 0%';

            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/update', true);

            xhr.upload.onprogress = function(e) {
                if (e.lengthComputable) {
                    const percent = Math.round((e.loaded / e.total) * 100);
                    pBarFill.style.width = percent + '%';
                    statusMsg.textContent = '⏳ Nahrávam do ESP32... ' + percent + '%';
                }
            };

            xhr.onload = function() {
                if (xhr.status === 200) {
                    pBarFill.style.width = '100%';
                    statusMsg.style.color = '#34d399';
                    statusMsg.innerHTML = '✅ <b>Úspešne nahraté!</b><br>ESP32 sa reštartuje... Presmerovanie za 12s.';
                    setTimeout(() => { window.location.href = '/'; }, 12000);
                } else {
                    statusMsg.style.color = '#f43f5e';
                    statusMsg.innerHTML = '❌ <b>Chyba pri nahrávaní!</b> (Kód: ' + xhr.status + ')';
                    btn.disabled = false;
                }
            };

            xhr.onerror = function() {
                statusMsg.style.color = '#f43f5e';
                statusMsg.innerHTML = '❌ <b>Chyba spojenia!</b> Skontrolujte WiFi signál.';
                btn.disabled = false;
            };

            xhr.send(formData);
        }
    </script>
</body>
</html>
)rawliteral";

WebServerManager::WebServerManager()
    : _server(80),
      _tempMgr(nullptr),
      _anemometer(nullptr),
      _windVane(nullptr),
      _lightSensor(nullptr),
      _rainGauge(nullptr),
      _wifiService(nullptr),
      _timeMgr(nullptr),
      _cloudOta(nullptr) {
}

void WebServerManager::begin(const TempSensorManager* tempMgr, const Anemometer* anemometer, const WindVane* windVane,
                             const WifiService* wifiService, const TimeManager* timeMgr, CloudOtaService* cloudOta,
                             const LightSensor* lightSensor, RainGauge* rainGauge) {
    _tempMgr = tempMgr;
    _anemometer = anemometer;
    _windVane = windVane;
    _lightSensor = lightSensor;
    _rainGauge = rainGauge;
    _wifiService = wifiService;
    _timeMgr = timeMgr;
    _cloudOta = cloudOta;

    _server.on("/", [this]() { handleRoot(); });
    _server.on("/api/live", [this]() { handleApiLive(); });
    _server.on("/api/calib/start", HTTP_POST, [this]() {
        if (_cloudOta) _cloudOta->setCalibMode(true);
        _server.send(200, "application/json", "{\"calibMode\":true}");
    });
    _server.on("/api/calib/stop", HTTP_POST, [this]() {
        if (_cloudOta) _cloudOta->setCalibMode(false);
        _server.send(200, "application/json", "{\"calibMode\":false}");
    });
    _server.on("/api/calib/toggle", HTTP_POST, [this]() {
        bool newState = false;
        if (_cloudOta) {
            newState = !_cloudOta->isCalibMode();
            _cloudOta->setCalibMode(newState);
        }
        _server.send(200, "application/json", String("{\"calibMode\":") + (newState ? "true" : "false") + "}");
    });
    _server.on("/api/test/rain-tip", [this]() { handleApiTestRainTip(); });
    _server.on("/update", HTTP_GET, [this]() { handleUpdatePage(); });
    _server.on("/update", HTTP_POST, [this]() { handleUpdateDone(); }, [this]() { handleUpdateUpload(); });
    _server.on("/api/ota/check", HTTP_GET, [this]() { handleApiOtaCheck(); });
    _server.on("/api/ota/cloud-update", HTTP_POST, [this]() { handleApiOtaCloudUpdate(); });
    _server.onNotFound([this]() { handleNotFound(); });

    _server.begin();
    Serial.println("[WebServer] HTTP Web Dashboard, Web OTA a Cloud OTA spustený na porte 80");
}

void WebServerManager::handleRoot() {
    _server.send_P(200, "text/html", INDEX_HTML);
}

void WebServerManager::handleNotFound() {
    _server.send(404, "text/plain", "404 Not Found");
}

void WebServerManager::handleUpdatePage() {
    _server.send_P(200, "text/html", UPDATE_HTML);
}

void WebServerManager::handleUpdateDone() {
    _server.sendHeader("Connection", "close");
    if (Update.hasError()) {
        _server.send(500, "text/plain", "OTA Update Failed");
    } else {
        _server.send(200, "text/plain", "OTA Update OK");
        delay(1000);
        ESP.restart();
    }
}

void WebServerManager::handleUpdateUpload() {
    HTTPUpload& upload = _server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("[OTA] Štart nahrávania súboru: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("[OTA] Úspešne nahratých %u bajtov. ESP32 sa reštartuje...\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.end();
        Serial.println("[OTA] Nahrávanie bolo prerušené!");
    }
}

void WebServerManager::handleApiOtaCheck() {
    StaticJsonDocument<512> doc;
    doc["stationId"] = Config::LOC_ID;
    doc["currentVersion"] = Config::FIRMWARE_VERSION;

    if (_cloudOta) {
        OtaCheckResult res = _cloudOta->checkVersion();
        doc["updateAvailable"] = res.updateAvailable;
        doc["newVersion"] = res.newVersion;
        doc["downloadUrl"] = res.downloadUrl;
        doc["notes"] = res.notes;
        doc["error"] = res.error;
    } else {
        doc["updateAvailable"] = false;
        doc["error"] = "CloudOtaService nie je inicializovaný";
    }

    String json;
    serializeJson(doc, json);
    _server.send(200, "application/json", json);
}

void WebServerManager::handleApiOtaCloudUpdate() {
    if (!_server.hasArg("plain")) {
        _server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, _server.arg("plain"));
    if (error) {
        _server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    String url = doc["url"] | "";
    if (url.isEmpty()) {
        _server.send(400, "application/json", "{\"error\":\"Missing URL\"}");
        return;
    }

    _server.send(200, "application/json", "{\"status\":\"started\"}");
    delay(500);

    if (_cloudOta) {
        _cloudOta->performUpdate(url);
    }
}

void WebServerManager::handleApiTestRainTip() {
    if (_rainGauge) {
        _rainGauge->simulatePulse();
        StaticJsonDocument<128> doc;
        doc["status"] = "ok";
        doc["rainToday"] = _rainGauge->getRainToday();
        doc["pulsesToday"] = _rainGauge->getPulsesToday();
        String json;
        serializeJson(doc, json);
        _server.send(200, "application/json", json);
    } else {
        _server.send(500, "application/json", "{\"error\":\"RainGauge not initialized\"}");
    }
}

void WebServerManager::handleApiLive() {
    if (!_tempMgr || !_anemometer || !_windVane || !_wifiService || !_timeMgr) {
        _server.send(500, "application/json", "{\"error\":\"Not initialized\"}");
        return;
    }

    StaticJsonDocument<1024> doc;
    doc["stationId"] = Config::LOC_ID;
    doc["version"] = Config::FIRMWARE_VERSION;
    doc["buildDate"] = __DATE__;
    doc["buildTime"] = __TIME__;
    doc["timestamp"] = _timeMgr->getFormattedCustom();
    doc["tempIn"] = _tempMgr->getTempIn();
    doc["tempOut"] = _tempMgr->getTempOut();
    doc["windSpeed"] = _anemometer->getWindSpeed();
    doc["windDirDeg"] = _windVane->getInstantAngle();
    doc["windDirName"] = _windVane->getInstantDirName();
    doc["vaneRatio"] = _windVane->getLastRatio();
    doc["wifiSSID"] = _wifiService->getConnectedSSID();
    doc["rssi"] = _wifiService->getRSSI();
    doc["ip"] = _wifiService->getIPAddress();
    doc["uptimeSec"] = millis() / 1000;

    if (_cloudOta) {
        doc["calibMode"] = _cloudOta->isCalibMode();
        doc["calibRemainingSec"] = _cloudOta->getCalibRemainingSec();
    }

    if (_lightSensor) {
        doc["lightPercent"] = _lightSensor->getBrightnessPercent();
        doc["lightMv"] = _lightSensor->getMilliVolts();
        doc["estimatedLux"] = _lightSensor->getEstimatedLux();
        doc["skyCondition"] = _lightSensor->getSkyCondition();
        doc["sunshineDuration"] = _lightSensor->getSunshineFormatted();
        doc["isDirectSun"] = _lightSensor->isDirectSun();
    }

    if (_rainGauge) {
        doc["rain15m"] = _rainGauge->getRain15Min();
        doc["rainToday"] = _rainGauge->getRainToday();
        doc["rainRate"] = _rainGauge->getRainRateMmH();
        doc["rainIntensity"] = _rainGauge->getRainIntensityDescription();
        doc["rainPulsesTotal"] = _rainGauge->getTotalPulses();
        doc["rainPulsesToday"] = _rainGauge->getPulsesToday();
    }

    String json;
    serializeJson(doc, json);
    _server.send(200, "application/json", json);
}

void WebServerManager::handleClient() {
    _server.handleClient();
}
