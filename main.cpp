#include <HardwareSerial.h>

HardwareSerial sim900(1);  // UART1 for GSM
HardwareSerial gpsSerial(2); // UART2 for GPS

#define RX_GSM 16  
#define TX_GSM 17  
#define RX_GPS 4  
#define TX_GPS 5  
#define SOUND_SENSOR_A0 34  

#define SAMPLE_TIME 1000  
#define AMPLITUDE_THRESHOLD 100  
#define FREQUENCY_THRESHOLD 1200  

void setup() {
    Serial.begin(115200);
    sim900.begin(9600, SERIAL_8N1, RX_GSM, TX_GSM);
    gpsSerial.begin(9600, SERIAL_8N1, RX_GPS, TX_GPS);
    analogReadResolution(12);

    Serial.println("Initializing GSM & GPS...");
    delay(2000);

    sendATCommand("AT");
    sendATCommand("AT+CMGF=1");
}

void loop() {
    int frequency = detectFrequency();
    Serial.print("Detected Frequency: ");
    Serial.println(frequency);

    if (frequency > FREQUENCY_THRESHOLD) {
        String gpsData = getGPS();
        sendSMS("Dog is in danger! " + gpsData);
        delay(5000);
    }

    delay(1000);
}

int detectFrequency() {
    unsigned long startTime = millis();
    int previousValue = analogRead(SOUND_SENSOR_A0);
    int crossings = 0;
    int minVal = 4096, maxVal = 0;

    while (millis() - startTime < SAMPLE_TIME) {
        int currentValue = analogRead(SOUND_SENSOR_A0);
        if (currentValue > maxVal) maxVal = currentValue;
        if (currentValue < minVal) minVal = currentValue;

        if ((previousValue < 2048 && currentValue >= 2048) || 
            (previousValue > 2048 && currentValue <= 2048)) {
            crossings++;
        }

        previousValue = currentValue;
    }

    int amplitude = maxVal - minVal;
    Serial.print("Signal Amplitude: ");
    Serial.println(amplitude);

    if (amplitude < AMPLITUDE_THRESHOLD) {
        Serial.println("Weak signal detected, returning 0 Hz.");
        return 0;
    }

    return (crossings / 2) * (1000 / SAMPLE_TIME);
}

String getGPS() {
    String gpsData = "";
    while (gpsSerial.available()) {
        char c = gpsSerial.read();
        gpsData += c;
    }
    if (gpsData.indexOf("$GPGGA") != -1) {
        return "GPS Data: " + gpsData;
    }
    return "GPS HAS No Signal";
}

void sendSMS(String message) {
    Serial.println("Sending SMS...");
    sendATCommand("AT+CMGS=\"+917620250840\"");
    sim900.println(message);
    delay(500);
    sim900.write(26);
    Serial.println("SMS Sent!");
}

void sendATCommand(String cmd) {
    Serial.print("Sending: ");
    Serial.println(cmd);
    sim900.println(cmd);
    delay(1000);
    while (sim900.available()) {
        Serial.write(sim900.read());
    }
    Serial.println();
}