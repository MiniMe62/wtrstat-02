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

        footer {
            margin-top: 32px;
            text-align: center;
            font-size: 0.8rem;
            color: var(--text-muted);
        }
    </style>
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

        <footer>
            wtrStat-02 • ESP32 Weather Station Firmware • PlatformIO C++
        </footer>
    </div>

    <script>
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

            } catch(e) {
                console.error("Fetch error:", e);
            }
        }
        setInterval(fetchLive, 2000);
        fetchLive();
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
