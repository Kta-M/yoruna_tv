#include <WioLTEforArduino.h>
#include <WioLTEClient.h>
#include <PubSubClient.h>		// https://github.com/SeeedJP/pubsubclient
#include <Ultrasonic.h>  
#include <stdio.h>
#include <string.h>

#define APN               "soracom.io"
#define USERNAME          "sora"
#define PASSWORD          "sora"

#define MQTT_SERVER_HOST  "beam.soracom.io"
#define MQTT_SERVER_PORT  (1883)

#define ID                "yoruna_tv"
#define OUT_TOPIC         "yoruna_tv/measured_value"
#define IN_TOPIC          "yoruna_tv/warning"

#define INTERVAL          (10000)

#define ULTRASONIC_PIN    (WIOLTE_D38)

#define BUZZER_PIN        (WIOLTE_D20)
#define BUZZER_ON_TIME    (100)
#define BUZZER_PAYLOAD    "buzzer"

WioLTE Wio;
WioLTEClient WioClient(&Wio);
PubSubClient MqttClient;
Ultrasonic UltrasonicRanger(ULTRASONIC_PIN);

void callback(char* topic, byte* payload, unsigned int length) {
  SerialUSB.print("Subscribe:");
  for (int i = 0; i < length; i++) SerialUSB.print((char)payload[i]);
  SerialUSB.println("");
  
  if(strncmp((const char*)payload, BUZZER_PAYLOAD, strlen(BUZZER_PAYLOAD)) == 0){ 
    digitalWrite(BUZZER_PIN, HIGH);
    delay(BUZZER_ON_TIME);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  Wio.PowerSupplyGrove(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connecting to \""APN"\".");
  if (!Wio.Activate(APN, USERNAME, PASSWORD)) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connecting to MQTT server \""MQTT_SERVER_HOST"\"");
  MqttClient.setServer(MQTT_SERVER_HOST, MQTT_SERVER_PORT);
  MqttClient.setCallback(callback);
  MqttClient.setClient(WioClient);
  if (!MqttClient.connect(ID)) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
  MqttClient.subscribe(IN_TOPIC);

  pinMode(ULTRASONIC_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  SerialUSB.println("### Setup completed.");
}

void loop() {
  char data[100];

  long distance = UltrasonicRanger.MeasureInCentimeters();
  sprintf(data,"{\"distance\":%d}", distance);

  SerialUSB.print("Publish:");
  SerialUSB.print(data);
  SerialUSB.println("");
  MqttClient.publish(OUT_TOPIC, data);
  
err:
  unsigned long next = millis();
  while (millis() < next + INTERVAL)
  {
    MqttClient.loop();
  }
}
