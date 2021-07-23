#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <Adafruit_GFX.h>
#include "ESP32RGBmatrixPanel.h"
#include "img.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <main.cpp>
#include <ArduinoOTA.h>

WebServer server(80);
File fsUploadFile;              // a File object to temporarily store the received file

const char* ssid1 = "CiPhone";
const char* password1 = "2222";
const char* ssid2 = "manting";
const char* password2 = "12345678";
const char* host = "esp32";
#define DBG_OUTPUT_PORT Serial

TaskHandle_t Task1;
TimerHandle_t Timer_Function1;

//G1	R1 |
//GND	B1 |
//G2	R2 |
//GND	B2  |
//B		A   |
//D		C  |
//LAT	CLK|
//GND	OE |


//ESP32RGBmatrixPanel matrix(23, 22, 03, 17, 16, 04, 00, 02, 15, 21, 19, 18, 5); //Flexible connection

//Default connection
//uint8 OE = 23;
//uint8 CLK = 22;
//uint8 LAT = 03;
//uint8 CH_A = 21;
//uint8 CH_B = 19;
//uint8 CH_C = 18;
//uint8 CH_D = 5;
//uint8 CH_E = 25;
//uint8 R1 = 17;
//uint8 G1 = 16;
//uint8 BL1 = 4;
//uint8 R2 = 0;
//uint8 G2 = 2;
//uint8 BL2 = 15;

const int dim = ROWS * COLUMNS * 3;
uint8_t DemoImg[dim];

ESP32RGBmatrixPanel matrix;


unsigned long oldLoopTime = 0;
int loopDelay = 10;
int loop1 = -32;

void MyTimer(TimerHandle_t Timer_Function1) {
  matrix.update();
}

void getRGB(int hue, int sat, int val, int colors[3]) {
  int r;
  int g;
  int b;
  int base;

  if (sat == 0) { // Acromatic color (gray). Hue doesn't mind.
    colors[0] = val;
    colors[1] = val;
    colors[2] = val;
  }
  else {

    base = ((255 - sat) * val) >> 8;

    switch (hue / 60) {
      case 0:
        r = val;
        g = (((val - base) * hue) / 60) + base;
        b = base;
        break;

      case 1:
        r = (((val - base) * (60 - (hue % 60))) / 60) + base;
        g = val;
        b = base;
        break;

      case 2:
        r = base;
        g = val;
        b = (((val - base) * (hue % 60)) / 60) + base;
        break;

      case 3:
        r = base;
        g = (((val - base) * (60 - (hue % 60))) / 60) + base;
        b = val;
        break;

      case 4:
        r = (((val - base) * (hue % 60)) / 60) + base;
        g = base;
        b = val;
        break;

      case 5:
        r = val;
        g = base;
        b = (((val - base) * (60 - (hue % 60))) / 60) + base;
        break;
    }

    colors[0] = r;
    colors[1] = g;
    colors[2] = b;
  }
}

auto str = "Hello from Vitali's ESP 32 @ Adafruit_GFX  4096 Colors/12 bit @ 50 fps";

void helloDemo()
{
  int pos = 64;
  matrix.setTextWrap(false);
  matrix.setTextSize(4);
  int colors[3];
  while (pos > -1800)
  {
    matrix.black();
    matrix.setCursor(pos, 2);
    getRGB(abs(pos / 4) % 255, 255, 255, colors);
    matrix.setTextColor(matrix.AdafruitColor(colors[0], colors[1], colors[2]));
    matrix.print(str);
    pos -= 1;
    vTaskDelay(10);
  }
}

void lavaDemo()
{
  int i = 0;
  int colors[3];
  while (i < 200)
  {
    for (int y = 0; y < ROWS; ++y)
    {
      for (int x = 0; x < COLUMNS; x++)
      {
        auto t = i / 20.0;
        auto val = sin(t + x / 20.0) * sin(t + y / 10.0) * 255.0;
        getRGB(abs(val), 255, 255, colors);
        matrix.drawPixel(x, y, colors[0], colors[1], colors[2]);
      }
    }
    i += 1;
    vTaskDelay(15);
  }
}

void wheaterDemo()
{
  byte br = 127;
  auto white = ESP32RGBmatrixPanel::AdafruitColor(br, br, br);
  auto red = ESP32RGBmatrixPanel::AdafruitColor(br, 0, 0);
  auto cyan = ESP32RGBmatrixPanel::AdafruitColor(0, br, br);
  auto blue = ESP32RGBmatrixPanel::AdafruitColor(0, 0, br);

  matrix.black();
  matrix.setTextWrap(false);
  matrix.setTextSize(2);
  matrix.setCursor(2, 0);
  matrix.setTextColor(white);
  matrix.print("18:36");

  matrix.setTextSize(1);

  matrix.setCursor(0, 16);
  matrix.setTextColor(blue);
  matrix.print("-11.3");

  matrix.setCursor(34, 16);
  matrix.setTextColor(red);
  matrix.print("+19.3");


  matrix.setCursor(1, 24);
  matrix.setTextColor(cyan);
  matrix.print("68%");
  matrix.setCursor(22, 24);
  matrix.print("1013hPa");
  vTaskDelay(6000);
}

void primitivesDemo()
{
  matrix.black();
  int speed = 100;
  for (int i = 0; i < 16; i += 2)
  {
    matrix.drawRect(i, i, 64 - 2 * i, 32 - 2 * i, Colors::RED);
    vTaskDelay(speed);
  }

  matrix.black();
  for (int i = 0; i < 90; i += 10)
  {
    matrix.drawLine(0, 0, 64, tan(i * PI / 180.0) * 64, Colors::CYAN);
    vTaskDelay(speed);
  }

  matrix.black();
  matrix.drawCircle(50, 20, 11, Colors::BLUE);
  vTaskDelay(speed);
  matrix.drawCircle(22, 11, 8, Colors::WHITE);
  vTaskDelay(speed);
  matrix.fillCircle(11, 22, 10, Colors::RED);
  vTaskDelay(speed);
  matrix.fillCircle(34, 11, 8, Colors::GREEN);
  vTaskDelay(speed * 10);

}

//runs faster then default void loop(). why? runs on other core?
void update_task(void *pvParameter)
{
  for (;;) {
    server.handleClient();
  }
}

/* create a hardware timer */
hw_timer_t* displayUpdateTimer = NULL;

void IRAM_ATTR onDisplayUpdate() {
  matrix.update();
}


void setup() {
  // Open DBG_OUTPUT_PORT communications and wait for port to open:
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.printf("\nLoop is running on %d core.\n", ARDUINO_RUNNING_CORE);
  DBG_OUTPUT_PORT.setDebugOutput(true);
  SPIFFS.begin();
  listDir(SPIFFS, "/", 0);

  int retryTime = 0;
  //WIFI INIT
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid1);
  if (String(WiFi.SSID()) != String(ssid1)) {
    WiFi.begin(ssid1, password1);
  }
  while (WiFi.status() != WL_CONNECTED && retryTime <= 5) {
    delay(1000);
    DBG_OUTPUT_PORT.print(".");
    retryTime++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    DBG_OUTPUT_PORT.printf("SSID1: %s connect failed.\nTry with SSID2: %s\n", ssid1, ssid2);
    if (String(WiFi.SSID()) != String(ssid2)) {
      WiFi.begin(ssid2, password2);
    }
    retryTime = 0;
    while (WiFi.status() != WL_CONNECTED && retryTime <= 10) {
      delay(1000);
      DBG_OUTPUT_PORT.print(".");
      retryTime++;
    }
  }


  if (WiFi.status() == WL_CONNECTED) {
    DBG_OUTPUT_PORT.println("");
    DBG_OUTPUT_PORT.print("Connected! IP address: ");
    DBG_OUTPUT_PORT.println(WiFi.localIP());
    MDNS.begin(host);
    DBG_OUTPUT_PORT.print("Open http://");
    DBG_OUTPUT_PORT.print(host);
    DBG_OUTPUT_PORT.println(".local/index.html to access this device");
  }

  //SERVER INIT
  server.on("/", HTTP_GET, []() {
    StopTimer();
    showMainPage();
    StartTimer();
  });
  server.on("/reset", []() {
    StopTimer();
    handleResetImage();
    StartTimer();
  });
  server.on("/test", []() {
    StopTimer();
    handleGenericArgs();
    StartTimer();
  });
  server.on("/brightness", []() {
    StopTimer();
    handleBrightness();
    StartTimer();
  });  
  server.on("/upload", HTTP_GET, []() {                 // if the client requests the upload page
    StopTimer();
    if (!handleFileRead("/upload.html"))                // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    StartTimer();
  });
  server.on("/upload", HTTP_POST, []() {                // if the client posts to the upload page
    server.send(200);                                   // Send status 200 (OK) to tell the client we are ready to receive
  },
  handleFileUpload );                                // Receive and save the file
  server.onNotFound([]() {
    StopTimer();
    if (!handleFileRead(server.uri())) {
      String message = "File Not Found\n\n";
      message += "URI: ";
      message += server.uri();
      message += "\nMethod: ";
      message += (server.method() == HTTP_GET) ? "GET" : "POST";
      message += "\nArguments: ";
      message += server.args();
      message += "\n";
      for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      }
      server.send(404, "text/plain", message);
    }
    StartTimer();
  });
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");

  matrix.setBrightness(2);
  handleResetImage();

  xTaskCreatePinnedToCore(
    &update_task,             /* Task function. */
    "update_task",            /* name of task. */
    8192 * 8,                 /* Stack size of task */
    NULL,                     /* parameter of the task */
    2,                        /* priority of the task */
    &Task1,                   /* Task handle to keep track of created task */
    0);                       /* Core. Arduino loops run on core 1. */

  vTaskDelay(1000); // wait for task created
  Serial.println("Task Created");

  /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  displayUpdateTimer = timerBegin(0, 80, true);
  timerAlarmWrite(displayUpdateTimer, 10, true);
  Serial.println("Timer Created");
  StartTimer();

  drawImg();
  Serial.println("I'm back");
}

void loop() {
  delay(1000);
  Serial.println("Loop");
  //  server.handleClient();
  //  delayMicroseconds(10);
}

volatile uint32_t isrCounter = 0; // for ISR use

void drawImg()
{
  uint8_t r, g, b;
  uint8_t r2, g2, b2;
  int x = 0, y = 0;
  for (unsigned int i = 0; i < ROWS * COLUMNS; ++ i)
  {
    r = DemoImg[i * 3 + 0];
    g = DemoImg[i * 3 + 1];
    b = DemoImg[i * 3 + 2];

    r2 = pgm_read_byte(&gamma[r]); // Gamma correction table maps
    g2 = pgm_read_byte(&gamma[g]); // 8-bit input to 4-bit output
    b2 = pgm_read_byte(&gamma[b]);
    matrix.drawPixel(i % COLUMNS, i / ROWS, r, g, b);
  }
}

void handleGenericArgs() { //Handler
  // In put http://172.20.10.9/test?abc=1&efg=2 on broweser URL
  // Then you will get result:
  //   Number of args received:2
  //   Arg No.0, Name:abc, content: 1
  //   Arg No.1, Name:efg, content: 2

  String message = "Number of args received:";
  message += server.args();            //Get number of parameters
  message += "\n";                            //Add a new line

  for (int i = 0; i < server.args(); i++) {

    message += "Arg No." + (String)i + ", Name:";   //Include the current iteration value
    message += server.argName(i) + ", content: ";     //Get the name of the parameter
    message += server.arg(i) + "\n";              //Get the value of the parameter

  }

  message += "\nplain:" + server.arg("plain");

  Serial.println(server.arg("plain"));
  server.send(200, "text/plain", message);       //Response to the HTTP request
}

void showMainPage()
{
  DBG_OUTPUT_PORT.println("Main!");
  if (!handleFileRead("/index.html")) server.send(404, "text/plain", "FileNotFound");
}

bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    returnFail("BAD ARGS");
    return;
  }
  String path = server.arg("dir");
  if (path != "/" && !SPIFFS.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }
  File dir = SPIFFS.open((char *)path.c_str());
  path = String();
  if (!dir.isDirectory()) {
    dir.close();
    returnFail("NOT DIR");
    return;
  }
  dir.rewindDirectory();

  String output = "[";
  for (int cnt = 0; true; ++cnt) {
    File entry = dir.openNextFile();
    if (!entry)
      break;

    if (cnt > 0)
      output += ',';

    output += "{\"type\":\"";
    output += (entry.isDirectory()) ? "dir" : "file";
    output += "\",\"name\":\"";
    // Ignore '/' prefix
    output += entry.name() + 1;
    output += "\"";
    output += "}";
    entry.close();
  }
  output += "]";
  server.send(200, "text/json", output);
  dir.close();
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  DBG_OUTPUT_PORT.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    DBG_OUTPUT_PORT.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    DBG_OUTPUT_PORT.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      DBG_OUTPUT_PORT.print("  DIR : ");
      DBG_OUTPUT_PORT.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      DBG_OUTPUT_PORT.print("  FILE: ");
      DBG_OUTPUT_PORT.print(file.name());
      DBG_OUTPUT_PORT.print("  SIZE: ");
      DBG_OUTPUT_PORT.println(file.size());
    }
    file = root.openNextFile();
  }
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}


void handleFileUpload() { // upload a new file to the SPIFFS
  StopTimer();
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    SPIFFS.remove(filename);                              // delete if the file exists
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {                                   // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      //server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      server.send(303);
      readBinToBuffer();
      StartTimer();
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void readBinToBuffer() {
  File bin = SPIFFS.open("/image.bin", "r");
  int binSize = bin.size();
  uint8_t buf[binSize];
  if (!bin) {
    Serial.println("image.bin open failed");
    return;
  }

  bin.read(buf, binSize);

#ifdef Debug
  for (int i = 0 ; i < binSize; i++)
    Serial.printf("%d, %d\n", i, char(buf[i]));

  //while (f.available()){
  //debugLogData += char(f.read());
  //Serial.printf("%d, %d", buf[i]);
  //}
#endif
  bin.close();

  memcpy(DemoImg, buf, ROWS * COLUMNS * 3);
  drawImg();
}

void StopTimer() {
  timerDetachInterrupt(displayUpdateTimer);
  //timerStop(displayUpdateTimer);
  timerAlarmDisable(displayUpdateTimer);
  //timerEnd(displayUpdateTimer);
  Serial.println("Timer Stopped!");
}

void StartTimer() {
  vTaskDelay(500);
  Serial.println("Timer Started!");
  /* Attach onTimer function to our timer */
  timerAttachInterrupt(displayUpdateTimer, &onDisplayUpdate, true);
  timerAlarmEnable(displayUpdateTimer);
}

void handleResetImage() {
  int i = server.arg("mode").toInt();
  if (i) readBinToBuffer();
  else ResetImage();
  String temp = "mode: " + String(i);
  server.send(200, "text/plain", "reset");
}

void ResetImage() {
  for (int i = 0; i < dim; i++) {
    DemoImg[i] = pgm_read_byte(&originImg[i]);
  }
  drawImg();
}

void handleBrightness(){
  int i = server.arg("brightness").toInt();
  Serial.print("HandleBrightness:");
  Serial.println(i);
  if (i<0) i = 0;
  if (i>8) i = 8;
  matrix.setBrightness(i);
  String temp = "the brightness is set to: " + String(i);
  server.send(200, "text/plain", temp);
}

