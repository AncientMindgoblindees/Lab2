#include "server.h"
#include "hardware.h"

bool RequestFlag = false;
const char* ssid = "ESP32TEST";
const char* password = "12345678";
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);

// Global pointer to hardware manager
static HardwareManager* g_hwManager = nullptr;

void server_setup(HardwareManager* hwManager) {
    // Store the hardware manager pointer
    g_hwManager = hwManager;
    
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);
    
    server.on("/", handle_OnConnect);
    server.on("/data", HTTP_GET, handle_request);
    server.on("/%", HTTP_POST, handle_BadCommand);
	server.on("/%", HTTP_GET, handle_BadCommand);
    server.onNotFound(handle_NotFound);
    server.begin();
    Serial.println("HTTP server started");
}

void server_loop() {
    server.handleClient();
    if (RequestFlag) {
        RequestFlag = false;
    }
}

void handle_OnConnect() {
    server.send(200, "text/html",
        "<!DOCTYPE html><html><body>"
        "<h1>ESP32 Web Server</h1>"
        "<p>Backend Active</p>"
        "<p id='lockstate'>Signal: ...</p>"
        "<script>"
        "function updateLock() {"
        "  fetch('/data').then(r => r.json()).then(d => {"
        "    let s = d.signal_lock === 1 ? 'LOCKED (440Hz detected)' : 'UNLOCKED (No 440Hz)';"
        "    document.getElementById('lockstate').innerText = 'Signal: ' + s;"
        "  });"
        "}"
        "setInterval(updateLock, 1000);"
        "updateLock();"
        "</script>"
        "</body></html>");
}

void handle_request() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    RequestFlag = true;
    // If hardware pointer is available, include ADC data and lock state
    String payload = "{";
    if (g_hwManager) {
        float v = g_hwManager->getLatestVoltage();
        float rms = g_hwManager->getBufferRMS();
        int lock = g_hwManager->getSignalLockState();
        payload += "\"adc_voltage\":" + String(v, 4) + ",";
        payload += "\"adc_rms\":" + String(rms, 4) + ",";
        payload += "\"signal_lock\":" + String(lock) + ",";
    }
    payload += "\"sending\":" + String(RequestFlag) + "}";
    server.send(200, "application/json", payload);
}

void handle_NotFound() {
    server.send(404, "text/plain", "Not found");
}

void handle_BadCommand() {
    server.send(400, "text/plain", "Bad Request");
}
