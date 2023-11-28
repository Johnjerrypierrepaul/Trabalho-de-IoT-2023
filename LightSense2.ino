#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <IOXhop_FirebaseESP32.h>
#include <ArduinoJson.h>

#define FIREBASE_HOST "https://lightsense-aa618-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "JhwnZNmvH5Bv3EdrkUpA2hjTWZFOxEnt58x8AxIq"

const int pinoPIR = 8; //PINO DIGITAL UTILIZADO PELO SENSOR
const int pinoLED = 12; //PINO DIGITAL UTILIZADO PELO LED
WiFiManager manager;




WiFiServer server(80);

// Constantes -------------------------------------------
const char*   ntpServer           = "pool.ntp.org";
const long    gmtOffset_sec       = -3 * 60 * 60;   // -3h*60min*60s = -10800s
const int     daylightOffset_sec  = 0;              // Fuso em horário de verão

// Variáveis globais ------------------------------------
time_t        nextNTPSync         = 0;
int value = 0;

String dateTimeStr(time_t t, int8_t tz = 0) {
  // Formata time_t como "aaaa-mm-dd hh:mm:ss"
  if (t == 0) {
    return "N/D";
  } else {
    t += tz * 3600;                               // Ajusta fuso horário
    struct tm *ptm;
    ptm = gmtime(&t);
    String s;
    s = ptm->tm_year + 1900;
    s += "-";
    if (ptm->tm_mon < 9) {
      s += "0";
    }
    s += ptm->tm_mon + 1;
    s += "-";
    if (ptm->tm_mday < 10) {
      s += "0";
    }
    s += ptm->tm_mday;
    s += " ";
    if (ptm->tm_hour < 10) {
      s += "0";
    }
    s += ptm->tm_hour;
    s += ":";
    if (ptm->tm_min < 10) {
      s += "0";
    }
    s += ptm->tm_min;
    s += ":";
    if (ptm->tm_sec < 10) {
      s += "0";
    }
    s += ptm->tm_sec;
    return s;
  }
}

String timeStatus() {
  // Obtém o status da sinronização
  if (nextNTPSync == 0) {
    return "não definida";
  } else if (time(NULL) < nextNTPSync) {
    return "atualizada";
  } else {
    return "atualização pendente";
  }
}


// Callback de sincronização
void ntpSync_cb(struct timeval *tv) {
  time_t t;
  t = time(NULL);
  // Data/Hora da próxima atualização
  nextNTPSync = t + (SNTP_UPDATE_DELAY / 1000) + 60;

  Serial.println("Sincronizou com NTP em " + dateTimeStr(t));
  Serial.println("Limite para próxima sincronização é " +
                  dateTimeStr(nextNTPSync));
}
  

void setup()
{
  Serial.begin(115200);

  WiFiManager wm;

  bool res;

  res = wm.autoConnect("LightSense");
  manager.setConnectTimeout(5);
  WiFiManager.autoConnect()

  if(!res) {
      Serial.println("Failed to connect");
  } 
  else {   
      Serial.println("connected...yeey :)");
  }
  
  if (!MDNS.begin("IoTConn")) {
     Serial.println("Error setting up MDNS responder!");
     while(1) {
         delay(1000);
     }
   }
  
  Serial.println("mDNS responder started");

  // Start TCP (HTTP) server
  server.begin();
  Serial.println("TCP server started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

  pinMode(pinoLED, OUTPUT); //DEFINE O PINO COMO SAÍDA
  pinMode(pinoPIR, INPUT); //DEFINE O PINO COMO ENTRADA

  // put your setup code here, to run once:
  delay(2000);

  Serial.begin(115200);
  //sntp_set_time_sync_notification_cb(ntpSync_cb);

  // Intervalo de sincronização - definido pela bibioteca lwIP
  Serial.printf("\n\nNTP sincroniza a cada %d segundos\n",
                SNTP_UPDATE_DELAY / 1000);

  // Função para inicializar o cliente NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    Serial.println("-------------------------- FIREBASE CONNECTION -------------------------");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    
  if (Firebase.failed()) {
    Serial.print("Falha na conexão com o Firebase. Código de erro: ");
    Serial.println(Firebase.error());
  } else {
    Serial.println("Conexão com o Firebase bem-sucedida...");
    Serial.println();
  }

}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

 if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
  if(digitalRead(pinoPIR) == HIGH){ //SE A LEITURA DO PINO FOR IGUAL A HIGH, FAZ
    digitalWrite(pinoLED, HIGH); //ACENDE O LED
 }else{ //SENÃO, FAZ
  digitalWrite(pinoLED, LOW); //APAGA O LED
 }
}