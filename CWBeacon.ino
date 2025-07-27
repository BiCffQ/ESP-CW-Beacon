#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "BH5URN@438.5MHz";
const char* password = "123456789";
String mesaj = "***** DE 14318KHZ CW BEACON     ";
const int punct = 75;
const int PTT = 2;

AsyncWebServer server(80);

// CW发送状态变量
unsigned long lastSendTime = 0;
int currentCharIndex = 0;
int currentBitIndex = 0;
String currentTransmit = "";


void handleRoot(AsyncWebServerRequest *request) {
  String html = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
            <title>CW Beacon</title>
            <style>
                body { font-family: Arial; margin: 20px; }
                input { padding: 8px; margin: 5px; }
            </style>
        </head>
        <body>
            <h2>CW Beacon Control</h2>
            <form action="/display">
                <input type="text" name="message" pattern="[A-Z0-9 =/*]*"
                       title="Only uppercase letters, numbers, spaces, =, /, *"
                       placeholder="Enter CW message">
                <input type="submit" value="Send">
            </form>
            <p>Current message: )rawliteral" + mesaj + R"rawliteral(</p>
        </body>
        </html>
    )rawliteral";
  request->send(200, "text/html", html);
}

void handleDisplay(AsyncWebServerRequest *request) {
  if (request->hasParam("message")) {
    mesaj = request->getParam("message")->value();
    currentCharIndex = 0;  // 重置发送状态
    currentBitIndex = 0;
    currentTransmit = "";
  }
  Serial.println(mesaj);
  request->send(200, "text/plain", "Message set to: " + mesaj);
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/display", HTTP_GET, handleDisplay);
  server.begin();
  Serial.println("HTTP server started");
}

String cautaCaracter(char caracter) {
  String transmit;
  switch (caracter) {
    case 'A': transmit = "10111000"; break;
    case 'B': transmit = "111010101000"; break;
    case 'C': transmit = "11101011101000"; break;
    case 'D': transmit = "1110101000"; break;
    case 'E': transmit = "1000"; break;
    case 'F': transmit = "101011101000"; break;
    case 'G': transmit = "111011101000"; break;
    case 'H': transmit = "1010101000"; break;
    case 'I': transmit = "101000"; break;
    case 'J': transmit = "1011101110111000"; break;
    case 'K': transmit = "111010111000"; break;
    case 'L': transmit = "101110101000"; break;
    case 'M': transmit = "1110111000"; break;
    case 'N': transmit = "11101000"; break;
    case 'O': transmit = "11101110111000"; break;
    case 'P': transmit = "10111011101000"; break;
    case 'Q': transmit = "1110111010111000"; break;
    case 'R': transmit = "1011101000"; break;
    case 'S': transmit = "10101000"; break;
    case 'T': transmit = "111000"; break;
    case 'U': transmit = "1010111000"; break;
    case 'V': transmit = "101010111000"; break;
    case 'W': transmit = "101110111000"; break;
    case 'X': transmit = "11101010111000"; break;
    case 'Y': transmit = "1110101110111000"; break;
    case 'Z': transmit = "11101110101000"; break;
    case '0': transmit = "1110111011101110111000"; break;
    case '1': transmit = "10111011101110111000"; break;
    case '2': transmit = "101011101110111000"; break;
    case '3': transmit = "1010101110111000"; break;
    case '4': transmit = "10101010111000"; break;
    case '5': transmit = "1010101010000"; break;
    case '6': transmit = "11101010101000"; break;
    case '7': transmit = "1110111010101000"; break;
    case '8': transmit = "111011101110101000"; break;
    case '9': transmit = "11101110111011101000"; break;
    case '=': transmit = "1110101010111000"; break;
    case '/': transmit = "1110101011101000"; break;
    case ' ': transmit = "000"; break;
    case '*': transmit = "1111111111"; break;
    default: transmit = ""; break;
  }
  return transmit;
}

void sendNextBit() {
  if (millis() - lastSendTime < punct) return;

  // 如果当前没有正在发送的字符，准备下一个字符
  if (currentTransmit.isEmpty()) {
    if (currentCharIndex >= mesaj.length()) {
      currentCharIndex = 0;  // 循环到消息开头
      digitalWrite(PTT, LOW); // 确保PTT释放
      delay(punct); // 字符间延时（7个单位时间）
      return;
    }

    char caracter = mesaj.charAt(currentCharIndex);
    currentTransmit = cautaCaracter(caracter);
    currentCharIndex++;
    currentBitIndex = 0;
  }

  // 发送当前bit
  if (currentBitIndex < currentTransmit.length()) {
    char semn = currentTransmit.charAt(currentBitIndex);
    digitalWrite(PTT, (semn == '1') ? HIGH : LOW);
    currentBitIndex++;
  } else {
    // 当前字符发送完成
    currentTransmit = "";
    digitalWrite(PTT, LOW);  // 确保PTT释放
    delay(3 * punct); // 字符间延时（3个单位时间）
  }

  lastSendTime = millis();
}


void setup() {
  Serial.begin(115200);
  pinMode(PTT, OUTPUT);
  digitalWrite(PTT, LOW);
  WiFi.softAP(ssid, password);
  setupWebServer();
}

void loop() {
  sendNextBit();  // 非阻塞发送CW
}
