#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Replace with your network credentials
const char* ssid = "Father";
const char* password = "tata9866";

// Create a OneWire instance for DS18B20
OneWire oneWire(D3);
DallasTemperature sensors(&oneWire);

// Create a web server instance
ESP8266WebServer server(80);

// Maximum number of data points to store
const int maxDataPoints = 30;

// Arrays to store temperature data for each sensor
float temperatureData1[maxDataPoints];
float temperatureData2[maxDataPoints];
float temperatureData3[maxDataPoints];

// Last update time for each sensor
unsigned long lastUpdateTime1 = 0;
unsigned long lastUpdateTime2 = 0;
unsigned long lastUpdateTime3 = 0;

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize DS18B20
  sensors.begin();

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/temperature", handleTemperature);

  // Start web server
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();

  // Update temperature for each sensor every 5 seconds
  updateTemperature(0, temperatureData1, lastUpdateTime1);
  updateTemperature(1, temperatureData2, lastUpdateTime2);
  updateTemperature(2, temperatureData3, lastUpdateTime3);
}

void updateTemperature(int sensorIndex, float temperatureData[], unsigned long &lastUpdateTime) {
  if (millis() - lastUpdateTime >= 5000) {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(sensorIndex);

    // Shift temperature data in the array
    for (int i = maxDataPoints - 1; i > 0; i--) {
      temperatureData[i] = temperatureData[i - 1];
    }
    temperatureData[0] = tempC; // Store latest temperature data
    lastUpdateTime = millis();
  }
}

void handleRoot() {
  // Send HTML page with JavaScript for temperature graph
  String html = "<html><head>";
  html += "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>";
  html += "<style>body { font-family: Arial, sans-serif; }</style>";
  html += "</head><body>";
  html += "<h1>Temperature Graph</h1>";
  html += "<div id='plot'></div>";
  html += "<script>";
  html += "var time = [];";
  html += "var temperature1 = [];";
  html += "var temperature2 = [];";
  html += "var temperature3 = [];";
  html += "function updateTemperature() {";
  html += "  var xhttp = new XMLHttpRequest();";
  html += "  xhttp.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      var temps = this.responseText.split(',').map(parseFloat);";
  html += "      time.unshift(new Date());"; // Add new timestamp at the beginning
  html += "      temperature1.unshift(temps[0]);"; // Add temperature1 at the beginning
  html += "      temperature2.unshift(temps[1]);"; // Add temperature2 at the beginning
  html += "      temperature3.unshift(temps[2]);"; // Add temperature3 at the beginning
  html += "      if (time.length > " + String(maxDataPoints) + ") {";
  html += "        time.pop();"; // Remove last timestamp if exceeding maxDataPoints
  html += "        temperature1.pop();"; // Remove last temperature1 if exceeding maxDataPoints
  html += "        temperature2.pop();"; // Remove last temperature2 if exceeding maxDataPoints
  html += "        temperature3.pop();"; // Remove last temperature3 if exceeding maxDataPoints
  html += "      }";
  html += "      var data = [";
  html += "        { x: time, y: temperature1, type: 'scatter', mode: 'lines+markers', name: 'Sensor 1' },";
  html += "        { x: time, y: temperature2, type: 'scatter', mode: 'lines+markers', name: 'Sensor 2' },";
  html += "        { x: time, y: temperature3, type: 'scatter', mode: 'lines+markers', name: 'Sensor 3' }";
  html += "      ];";
  html += "      Plotly.newPlot('plot', data);";
  html += "    }";
  html += "  };";
  html += "  xhttp.open('GET', '/temperature', true);";
  html += "  xhttp.send();";
  html += "}";
  html += "setInterval(updateTemperature, 5000);";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleTemperature() {
  // Send temperature for each sensor as comma-separated plain text
  String response = String(temperatureData1[0]) + "," + String(temperatureData2[0]) + "," + String(temperatureData3[0]);
  server.send(200, "text/plain", response);
}
