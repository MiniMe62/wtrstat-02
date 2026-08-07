/**
 * Google Apps Script pre ESP32 WeatherStation (wtrStat-02)
 * Spracováva HTTP POST requesty z ESP32 a zapisuje meteo dáta do Google Tabuľky.
 */

function doPost(e) {
  try {
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    var contents = JSON.parse(e.postData.contents);

    // Ak je tabuľka prázdna, vytvoríme hlavičku
    if (sheet.getLastRow() === 0) {
      sheet.appendRow([
        "Timestamp",
        "Location ID",
        "Temp In (°C)",
        "Temp Out (°C)",
        "Wind Speed Avg (m/s)",
        "Wind Speed Max (m/s)",
        "Wind Dir (°)",
        "Wind Dir Name"
      ]);
      sheet.getRange(1, 1, 1, 8).setFontWeight("bold").setBackground("#38bdf8");
    }

    // Extrakcia parametrov z JSON
    var timestamp = contents.timestamp || new Date().toLocaleString();
    var locid = contents.locid || "GO85";
    var tempIn = contents.tempIn !== undefined ? contents.tempIn : "";
    var tempOut = contents.tempOut !== undefined ? contents.tempOut : "";
    var windSpeed = contents.windSpeed !== undefined ? contents.windSpeed : "";
    var windSpeedMax = contents.windSpeedMax !== undefined ? contents.windSpeedMax : "";
    var windDirDeg = contents.windDirDeg !== undefined ? contents.windDirDeg : "";
    var windDirName = contents.windDirName || "";

    // Zapísanie nového riadku do Google Sheets
    sheet.appendRow([
      timestamp,
      locid,
      tempIn,
      tempOut,
      windSpeed,
      windSpeedMax,
      windDirDeg,
      windDirName
    ]);

    return ContentService.createTextOutput("Success")
      .setMimeType(ContentService.MimeType.TEXT);

  } catch (error) {
    return ContentService.createTextOutput("Error: " + error.toString())
      .setMimeType(ContentService.MimeType.TEXT);
  }
}

function doGet(e) {
  return ContentService.createTextOutput("wtrStat-02 Google Apps Script Endpoint Active")
    .setMimeType(ContentService.MimeType.TEXT);
}
