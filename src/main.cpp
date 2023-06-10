#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>

// kampus
const char *ssid = "iPhone Meliusa";           // sesuaikan dengan username wifi
const char *password = "anuanuanu";            // sesuaikan dengan password wifi
const char *mqtt_server = "broker.hivemq.com"; // isikan server broker

const int redPin = D5;   // Pin untuk menghubungkan LED merah
const int greenPin = D6; // Pin untuk menghubungkan LED hijau
const int bluePin = D7;  // Pin untuk menghubungkan LED biru

bool isRedOn = false;   // Status LED merah
bool isGreenOn = false; // Status LED hijau
bool isBlueOn = false;  // Status LED biru

WiFiClient espClient;
PubSubClient client(espClient);

SimpleDHT11 dht11(D1);

const int ldrPin = A0; // Pin untuk menghubungkan LDR

long now = millis();
long lastMeasure = 0;
String macAddr = "";

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
  macAddr = WiFi.macAddress();
  Serial.println(macAddr);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(macAddr.c_str()))
    {
      Serial.println("connected");
      client.subscribe("meliusa/rgbled");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void handleLedControl(char *payload, int length)
{
  // Mengonversi pesan MQTT menjadi string
  String message = "";
  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  // Mengendalikan LED berdasarkan pesan MQTT
  if (message.equals("red_on"))
  {
    digitalWrite(redPin, HIGH); // Menyalakan LED merah
    isRedOn = true;
  }
  else if (message.equals("red_off"))
  {
    digitalWrite(redPin, LOW); // Mematikan LED merah
    isRedOn = false;
  }
  else if (message.equals("green_on"))
  {
    digitalWrite(greenPin, HIGH); // Menyalakan LED hijau
    isGreenOn = true;
  }
  else if (message.equals("green_off"))
  {
    digitalWrite(greenPin, LOW); // Mematikan LED hijau
    isGreenOn = false;
  }
  else if (message.equals("blue_on"))
  {
    digitalWrite(bluePin, HIGH); // Menyalakan LED biru
    isBlueOn = true;
  }
  else if (message.equals("blue_off"))
  {
    digitalWrite(bluePin, LOW); // Mematikan LED biru
    isBlueOn = false;
  }
}

void callback(char *topic, byte *payload, int length)
{
  // Memanggil fungsi handleLedControl jika topik sesuai
  if (String(topic) == "meliusa/rgbled")
  {
    handleLedControl((char *)payload, length);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Mqtt Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Mengatur mode pin LED sebagai output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Inisialisasi status awal LED mati
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);

  pinMode(ldrPin, INPUT); // Mengatur pin LDR sebagai input
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect(macAddr.c_str());
  }
  now = millis();
  if (now - lastMeasure > 5000)
  {
    lastMeasure = now;
    int err = SimpleDHTErrSuccess;

    byte temperature = 0;
    byte humidity = 0;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
    {
      Serial.print("Pembacaan DHT11 gagal, err=");
      Serial.println(err);
      delay(1000);
      return;
    }
    static char temperatureTemp[7];
    dtostrf(temperature, 4, 2, temperatureTemp);
    Serial.print("Temperature: ");
    Serial.println(temperatureTemp);

    static char humidityTemp[7];
    dtostrf(humidity, 4, 2, humidityTemp);
    Serial.print("Humidity: ");
    Serial.println(humidityTemp);

    int lightLevel = analogRead(ldrPin); // Membaca nilai cahaya dari sensor LDR

    static char lightLevelTemp[7];
    dtostrf(lightLevel, 4, 2, lightLevelTemp);
    Serial.print("Light Level: ");
    Serial.println(lightLevelTemp);

    client.publish("meliusa/temperature", temperatureTemp);
    client.publish("meliusa/humidity", humidityTemp);
    client.publish("meliusa/light", lightLevelTemp);
  }
}
