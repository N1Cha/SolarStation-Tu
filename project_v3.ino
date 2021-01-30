//AUTO-REFRESH VARIANT.4
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "DHT.h"
#include "SPIFFS.h"
#include "time.h"

#define DHTPIN 5 
#define DHTTYPE DHT11 
//#define DHTTYPE DHT22
//#define DHTTYPE DHT21

AsyncWebServer server(80);
Adafruit_BME280 bme;
DHT dht(DHTPIN, DHTTYPE);
File dataFile;

const char* ssid = "subZero";
const char* password = "nikola9696a";
const String fileName = "/data.text";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
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
    p { font-size: 2.4rem; }
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
  <h2>Solar Station</h2>
  <p>
    <i class="fas fa-temperature-high" style="color:#ff7e67;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#006a71;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#68b0ab;"></i> 
    <span class="dht-labels">Solar Temprature</span> 
    <span id="tpanel">%TPANEL%</span>
    <sup class="units">&deg;C</sup>
  </p>
   <p>
    <i class="fas fa-solar-panel" style="color:#68b0ab;"></i> 
    <span class="dht-labels">Solar Voltage</span> 
    <span id="vpanel">%VPANEL%</span>
    <sup class="units">V</sup>
  </p>
   <p>
    <i class="fas fa-solar-panel" style="color:#68b0ab;"></i> 
    <span class="dht-labels">Solar Current</span> 
    <span id="cpanel">%CPANEL%</span>
    <sup class="units">A</sup>
  </p>
  <hr>
  <p>    
    <h3 class="dht-labels"><b>File Data</b></h3>
    <div>
    <button id="refreshButton"><i class="fas fa-sync" style="color:#68b0ab;"></i></button>
    <button id="deleteButton"><i class="far fa-trash-alt" style="color:#ff0000;"></i></button>
    </div>
    <p id="ptable" class="units" >%PTABLE%</p>  
  </p>
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
}, 1000) ;

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

var refreshBtn = document.getElementById("refreshButton");
var deleteBtn = document.getElementById("deleteButton");

refreshBtn.addEventListener( "click", function() {
  var xhttp = new XMLHttpRequest(); 
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ptable").innerHTML = this.responseText;
    }
  };
  xhttp.open( "GET", "/time", true);
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
</script>
</html>)rawliteral";

// PROTOTYPES REGION --------------------------------------
void fileConfiguration();
void AddTableRowToFile();
String createHtmlTable(String rows, String date);
String addTableRecord(String times, float temp, float humidity, float pt, float pv, float pa);
void printLocalTime(String hour, String date);
String getDate();
String getTime();
String processor(const String& var);
String getTableRows();
void printFileContent();
void deleteFileContent(String fName);
// PROTOTYPES REGION END ----------------------------------

void setup()
{
  Serial.begin(115200);
  
  bme.begin(0x76);  
  dht.begin();  

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", index_html, processor);
  });
  
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(bme.readTemperature()).c_str());
  });
  
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(bme.readHumidity()).c_str());
  });

  server.on("/tpanel", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(dht.readTemperature()).c_str());
  });

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request)
  {    
    String table  = String(createHtmlTable(getTableRows(),getDate()));
    Serial.println("Table Refresh");  
    request->send_P(200, "text/plain", String(table).c_str());
  });

  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request)
  { 
    deleteFileContent(fileName);
    Serial.println("File Deleted");  
    request->send_P(200, "text/plain", String(createHtmlTable(getTableRows(),getDate())).c_str());
  });
  
  server.begin();
  
  fileConfiguration();   
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
 
void loop()
{
  AddTableRowToFile();
  delay(60000);
}

// FILE REGION -----------------------------------
String getTableRows()
{  
  String fileData = String();
  dataFile = SPIFFS.open(fileName,"r");  
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
  
  dataFile = SPIFFS.open(fileName,"r");
  if (dataFile)
  {
    Serial.println("FileConfiguration: The file is ready!");
  }
  else
  {
    dataFile = SPIFFS.open(fileName,"w+");
  }
  
  dataFile.close();
}

void AddTableRowToFile()
{  
  String row = addTableRecord(getTime(),bme.readTemperature(),bme.readHumidity(),dht.readTemperature(),0,0);
  dataFile = SPIFFS.open(fileName,"a");
  dataFile.println(row);
  dataFile.close();
}

String createHtmlTable(String rows, String date)
{
  String table = "<table>";  
  table += "<tr>";
  table += "<th>Time ";
  table += date;
  table += "</th>";
  table += "<th>Temperature</th>";
  table += "<th>Hunidity</th>";
  table += "<th>Solar Temp</th>";
  table += "<th>Solar Voltage</th>";
  table += "<th>Solar Current</th>";
  table += "</tr>";
  table += rows;
  table += "</table>"; 

  return table;
}

String addTableRecord(String times, float temp, float humidity, float pt, float pv, float pa)
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
// DATE TIME REGION END -----------------------------------

String processor(const String& var)
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
  else if(var == "PTABLE");
  {
  }
  return String();
}
