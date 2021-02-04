#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "DHT.h"
#include "SPIFFS.h"
#include "time.h"

#define DHTPIN 5 
#define DHTTYPE DHT11
#define ONBOARD_LED  2
#define DATA_RECORD_SECONDS 60

AsyncWebServer server(80);
Adafruit_BME280 bme;
DHT dht(DHTPIN, DHTTYPE);
File dataFile;
int hDay = -1;
int secondsCounter = DATA_RECORD_SECONDS;

const char* ssid = "subZero";
const char* password = "nikola9696a";
const String fileName = "/data.text";
const String historyfileName = "/history.text";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7200;
const int daylightOffset_sec = 3600;
const int voltagePin = 32;
const int currentPin = 4;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html id="htmlTag">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    .p-font { font-size: 2.4rem; }
    ul {
     list-style-type: none;
     margin: 0;
     padding: 0;
     overflow: hidden;
     background-color: #333;
    }
   li {  float: left; }
   li p {
     display: block;
     color: white;
     text-align: center;
     padding: 14px 16px;
     text-decoration: none;
    }
    li p:hover {  background-color: #111; }
    .units { font-size: 1.2rem; }
    .dht-labels
    {
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <ul>
    <li><p id="navLive">PV Station</p></li>
    <li><p id="navFile">File Data</p></li>
  </ul>
  <h2>PV Station</h2>
  <p class="p-font">
    <i class="fas fa-solar-panel" style="color:#68b0ab;"></i> 
    <span class="dht-labels">PV Voltage</span> 
    <span id="vpanel">%VPANEL%</span>
    <sup class="units">V</sup>
  </p>
   <p class="p-font">
    <i class="fas fa-solar-panel" style="color:#68b0ab;"></i> 
    <span class="dht-labels">PV Current</span> 
    <span id="cpanel">%CPANEL%</span>
    <sup class="units">A</sup>
  </p>
  <p class="p-font">
    <i class="fas fa-solar-panel" style="color:#68b0ab;"></i> 
    <span class="dht-labels">PV Power</span> 
    <span id="wpanel">%WPANEL%</span>
    <sup class="units">W</sup>
  </p>
  <p class="p-font">
    <i class="fas fa-solar-panel" style="color:#68b0ab;"></i>
    <i class="fas fa-thermometer-half" style="color:#68b0ab;"></i> 
    <span class="dht-labels">PV Temprature</span> 
    <span id="tpanel">%TPANEL%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p class="p-font">
    <i class="fas fa-temperature-high" style="color:#ff7e67;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p class="p-font">
    <i class="fas fa-tint" style="color:#006a71;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p> 
  <div>
    <button id="dataPage"><b>Show File Data</b></button>
 </div>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 1000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 1000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tpanel").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/tpanel", true);
  xhttp.send();
}, 1000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("vpanel").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/vpanel", true);
  xhttp.send();
}, 1000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("cpanel").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/cpanel", true);
  xhttp.send();
}, 1000);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("wpanel").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/wpanel", true);
  xhttp.send();
}, 1000);

var pageBtn = document.getElementById("dataPage");
var navLive = document.getElementById("navLive");
var navFile = document.getElementById("navFile");

pageBtn.addEventListener( "click", function() {
  window.open(location.protocol + "//" + location.host +  "/data","_self");
});

navFile.addEventListener( "click", function() {
  window.open(location.protocol + "//" + location.host +  "/data","_self");
});

navLive.addEventListener( "click", function() {
  window.open(location.protocol + "//" + location.host,"_self");
});
</script>
</html>)rawliteral";

const char data_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    td, th {
      padding: 5px;
      border: 1px solid black;
    }
    table {
      margin-left: auto;      
      margin-right: auto;     
      border-collapse: collapse;
      border: 1px solid black;
    }
    ul {
      list-style-type: none;
      margin: 0;
      padding: 0;
      overflow: hidden;
      background-color: #333;
    }
    li {  float: left; }
    li p {
      display: block;
      color: white;
      text-align: center;
      padding: 14px 16px;
      text-decoration: none;
    }
    li p:hover {  background-color: #111; }
    button{ margin: 10px; }
  </style>
</head>
<body>
  <ul>
    <li><p id="navLive">Live Data</p></li>
    <li><p id="navFile">File Data</p></li>
  </ul>
  <h2>File Data</h2>
  <div>
    <button id="refreshButton"><i class="fas fa-sync" style="color:#68b0ab;"></i></button>
    <button id="deleteButton"><i class="far fa-trash-alt" style="color:#ff0000;"></i></button>
  </div>
  <p id="ptable"> </p>   
  <p>
    <button id="backButton"><b>Back to Solar Data</b></button>
  </p>
</body>
<script>
var refreshBtn = document.getElementById("refreshButton");
var deleteBtn = document.getElementById("deleteButton");
var backBtn = document.getElementById("backButton");
var navLive = document.getElementById("navLive");
var navFile = document.getElementById("navFile");

refreshBtn.addEventListener( "click", function() {
  var xhttp = new XMLHttpRequest(); 
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ptable").innerHTML = this.responseText;
    }
  };
  xhttp.open( "GET", "/refresh", true);
  xhttp.send();
});

deleteBtn.addEventListener( "click", function() {
  var xhttp = new XMLHttpRequest(); 
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ptable").innerHTML = this.responseText;
    }
  };
  xhttp.open( "GET", "/delete", true);
  xhttp.send();
});

backBtn.addEventListener( "click", function() {
  window.open(location.protocol + "//" + location.host,"_self");
});

navFile.addEventListener( "click", function() {
  window.open(location.protocol + "//" + location.host +  "/data","_self");
});

navLive.addEventListener( "click", function() {
  window.open(location.protocol + "//" + location.host,"_self");
});
</script>
</html>)rawliteral";

// PROTOTYPES REGION --------------------------------------
void fileConfiguration();
void AddTableRowToFile();
String createHtmlTable(String hRows, String dRows);
String addTableRecord(String times, float temp, float humidity, float pt, float pv, float pa, float pw);
void printLocalTime(String hour, String date);
String getDate();
String getTime();
String replacer(const String& var);
void addContentToFile(String fName, String content);
String getFileContent(String fName);
void printFileContent();
void deleteFileContent(String fName);
void verifyFileExistence(String fName);
void verifyHistoryTime();
bool isUpdateNeeded();
String getFullTime();
float getVolage();
float getCurrent();
float getPower();
void connectionLed(int ledPin);
// PROTOTYPES REGION END ----------------------------------

void setup()
{
  pinMode(ONBOARD_LED,OUTPUT);
  Serial.begin(115200);
  
  bme.begin(0x76);  
  dht.begin();  

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("Connecting to WiFi..");
    connectionLed(ONBOARD_LED);
  }
  
  Serial.println(WiFi.localIP());
  digitalWrite (ONBOARD_LED, HIGH);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", index_html, replacer);
  });  
  
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", data_html);
  });  
  
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(bme.readTemperature()).c_str());
  });
  
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(bme.readHumidity()).c_str());
  });
  
  server.on("/vpanel", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(getVolage()).c_str());
  });
  
  server.on("/cpanel", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(getCurrent()).c_str());
  });

  server.on("/wpanel", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(getPower()).c_str());
  });

  server.on("/tpanel", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(dht.readTemperature()).c_str());
  });
  
  server.on("/refresh", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String table = createHtmlTable(getFileContent(historyfileName),getFileContent(fileName));
    Serial.println("Table Refresh");  
    request->send_P(200, "text/plain", table.c_str());
  });

  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request)
  { 
    deleteFileContent(fileName);
    deleteFileContent(historyfileName);
    Serial.println("Files Deleted");  
    request->send_P(200, "text/plain", String(createHtmlTable(getFileContent(historyfileName),getFileContent(fileName))).c_str());
  });
  
  server.begin();
  
  fileConfiguration();   
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  verifyHistoryTime();
}
 
void loop()
{
  if(secondsCounter == DATA_RECORD_SECONDS)
  {    
    AddTableRowToFile();
    secondsCounter = 0;
  }
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("Connecting to WiFi..");
    connectionLed(ONBOARD_LED);
  }

  if(WiFi.status() == WL_CONNECTED)
  { 
    digitalWrite (ONBOARD_LED, HIGH);
  }
  
  secondsCounter += 1;
  float v = getVolage();
  Serial.print("Voltage: ");
  Serial.println(v);
  delay(1000);
}

// FILE REGION -----------------------------------
void addContentToFile(String fName, String content)
{
  dataFile = SPIFFS.open(fName,"a");
  dataFile.println(content);
  dataFile.close();
}

String getFileContent(String fName)
{  
  String fileData = String();
  dataFile = SPIFFS.open(fName,"r");  
  while (dataFile.available()) 
  {
    char symbol = dataFile.read();
    fileData += symbol;
  }
  
  dataFile.close();
  return fileData;
}

void fileConfiguration()
{
  if(!SPIFFS.begin(true))
  {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
  }

  verifyFileExistence(fileName);
  verifyFileExistence(historyfileName);  
}

void AddTableRowToFile()
{ 
  if(isUpdateNeeded())
  {
    deleteFileContent(historyfileName);
    addContentToFile(historyfileName, getFileContent(fileName));
    deleteFileContent(fileName);
    verifyHistoryTime();
  }
  
  String row = addTableRecord(getFullTime(),bme.readTemperature(),bme.readHumidity(),dht.readTemperature(),getVolage(),getCurrent(),getPower());
  dataFile = SPIFFS.open(fileName,"a");
  dataFile.println(row);
  dataFile.close();
}

String createHtmlTable(String hRows, String dRows)
{
  String table = "<table>";  
  table += "<tr>";
  table += "<th>Time</th>";
  table += "<th>Temperature</th>";
  table += "<th>Hunidity</th>";
  table += "<th>PV Temperature</th>";
  table += "<th>PV Voltage</th>";
  table += "<th>PV Current</th>";
  table += "<th>PV Power</th>";
  table += "</tr>";
  table += hRows;
  table += dRows;
  table += "</table>"; 

  return table;
}

String addTableRecord(String times, float temp, float humidity, float pt, float pv, float pa, float pw)
{
  String tableRow = String();
  tableRow += "<tr>";
  tableRow += "<td>";
  tableRow += times;
  tableRow += "</td>";
  tableRow += "<td>";
  tableRow += temp;
  tableRow += "<td>";
  tableRow += humidity;
  tableRow += "</td>";
  tableRow += "<td>";
  tableRow += pt;
  tableRow += "</td>";
  tableRow += "<td>";
  tableRow += pv;
  tableRow += "</td>";
  tableRow += "<td>";
  tableRow += pa;
  tableRow += "</td>";
  tableRow += "<td>";
  tableRow += pw;
  tableRow += "</td>";
  tableRow += "</tr>";
  tableRow += "\n";
  
  return tableRow;
}

void printFileContent()
{ 
  Serial.println("File Content:");
  dataFile = SPIFFS.open(fileName,"r");  
  while (dataFile.available()) 
  {
    Serial.write(dataFile.read());
  }
  dataFile.close();
}

void deleteFileContent(String fName)
{
  dataFile = SPIFFS.open(fName,"w+");
  dataFile.close();
}

void verifyFileExistence(String fName)
{
  File file = SPIFFS.open(fName,"r");
  if (!file)
  {    
    file = SPIFFS.open(fName,"w+");
    Serial.print("The File is created");
    Serial.println(fName);
  }
  else
  {    
    Serial.print("FileConfiguration: File is ready - ");
    Serial.println(fName);
  }
  
  file.close();
}
// FILE REGION END -----------------------------------
// DATE TIME REGION ----------------------------------
void printLocalTime(String hour, String date)
{
  Serial.println(hour);
  Serial.println(date);
}

String getDate()
{
  struct tm timeinfo;
  String date = String();
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return "Failed to obtain time";
  }
  
  date += timeinfo.tm_mday;
  date += ".";
  if(timeinfo.tm_mon<10)
  {
    date += "0";
  }
  date += timeinfo.tm_mon + 1;
  
  return date;
}

String getTime()
{
  struct tm timeinfo;
  String hour = String();
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return "Failed to obtain time";
  }

  hour += timeinfo.tm_hour;
  hour +=":";
  if(timeinfo.tm_min < 10)
  {
    hour += "0";
  }
  hour += timeinfo.tm_min;
  
  return hour;
}

void verifyHistoryTime()
{
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  hDay = timeinfo.tm_mday;
  Serial.print("hDay = ");
  Serial.println(hDay);
}

bool isUpdateNeeded()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("isUpdateNeeded: Failed to obtain time");
    return false;
  }

  if(hDay != timeinfo.tm_mday)
  {
    Serial.println("isUpdateNeeded: TRUE");
    return true;
  }
  else
  {
    return false;
  }
}

String getFullTime()
{
  String fullTime = getDate();
  fullTime += " ";
  fullTime += getTime();

  return fullTime;
}
// DATE TIME REGION END -----------------------------------

// GET PANEL DATA REGION ----------------------------------
float getVolage()
{
  int voltage = analogRead(voltagePin);
  float volts = 0.00081 * voltage;
  return volts;
}

float getCurrent()
{
  return 1.0;
}

float getPower()
{
  float voltage = getVolage();
  float current = getCurrent();
  float power = voltage * current;
  return power;
}
// GET PANEL DATA REGION END ------------------------------
String replacer(const String& var)
{
  Serial.println(var);
  if(var == "TEMPERATURE")
  {
    return String(bme.readTemperature());
  }
  else if(var == "HUMIDITY")
  {
    return String(bme.readHumidity());
  }
  else if(var == "TPANEL")
  {
    return String(dht.readTemperature());
  }
  else if(var == "CPANEL")
  {
    return String(getCurrent());
  }
  else if(var == "VPANEL")
  {
    return String(getVolage());
  }
  else if(var == "WPANEL")
  {
    return String(getPower());
  }
  
  return String();
}

void connectionLed(int ledPin)
{
  digitalWrite (ledPin, HIGH);
  delay(500);
  digitalWrite (ledPin, LOW);  
  delay(500);
}
