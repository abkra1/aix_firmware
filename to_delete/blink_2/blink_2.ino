
#define LED_PIN    14   // D7
#define RELAIS_PIN    13   // D5



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.flush();
  delay(100); // we need time to switch the port
  // if we have a problem, give me 5 secs to interrupt on startup
  printf("\n---------------------------------------------------------------\n");
  printf("        Version 1.0: led dummy\n");
  printf("---------------------------------------------------------------\n");
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAIS_PIN, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  printf("switch on/off\n");
  while (true) {
    printf("off\n");
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAIS_PIN, LOW);
    delay(5000);
    printf("on\n");
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(RELAIS_PIN, HIGH);
    delay(5000);
  }

}
