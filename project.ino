//Namesh's Offical Code	
#include <ESP8266WiFi.h>  //ESP8266 Core WiFi Library (you most likely already have this in your sketch)	
#include <WiFiClient.h>	
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal	
#include <NTPClient.h>        //Time Manager	
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal	
#include <ESP8266HTTPClient.h>	

//  NTP TIME MODULE	
const long utcOffsetInSeconds = 19800;  // Srilankan 5 1/2 Hour offset

//Week Days	
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};	
//Month names	
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};	

// Define NTP Client to get time	
WiFiUDP ntpUDP;	
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);	

// WIFI ROUTER CREDENTIALS	
const char* ssid = "BELL49AC5"; 	
const char* password = "F5TBG236ADEce5fgrg@E2E";	

//SMTP RELAY CREDENTIALS	
//char relayServer[] = "smtp-relay.gmail.com"; 	

//ULTRASONIC SENSOR PIN IDs	
const int trigPin = D0;	
const int echoPin = D1;

// defines variables	
long duration;	
int distance;	
String page = "";	
double data; 	

// STOP Counter variables	
int stopTimeInMinutes = 1;	
int sensitivityMotionRange = 10;	
int currentDistance = 0;	
int lastDistance = 0;	
int count= 0;	
int isEmailSent = 0;	

ESP8266WebServer server(80);   //instantiate server at port 80 (http port)	
WiFiClient client;	

void setup(){	
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output	
  pinMode(echoPin, INPUT);	
  	
  Serial.begin(115200);	
  delay(10);	
  Serial.println("");	
  Serial.println("");	
  Serial.print("Connecting To: ");	
  Serial.println(ssid);	
  WiFi.begin(ssid, password); // WIFI CONNECTION	
  // Wait for connection	
  while (WiFi.status() != WL_CONNECTED){	
    delay(500);	
    Serial.print(".");	
  }	
  Serial.println("");	
  Serial.println("WiFi Connected.");	
  Serial.print("IP address: ");	
  Serial.println(WiFi.localIP());	
  Serial.println(WiFi.macAddress());	
  sendStartMail();	
  //ROUTE FOR ULTRASONIC SENSOR VALUE CHECK	
  server.on("/", [](){	
    page = "<h4>Namesh's</h4><h1>Sensor to Node MCU Web Server</h1><h3>ultrasonic distance:</h3> <h1>"+String(data)+"</14><head><META HTTP-EQUIV=\"refresh\" CONTENT=\"1\"></head>"; 	
    server.send(200, "text/html", page);	
  });	
  server.begin();	
  Serial.println("Web server started!");	
  	
  /*   TELNET SMTP RELAY TEST	
  Serial.println("Connecting to gmail...");	
  if (espClient.connect("smtp-relay.gmail.com", 587))  {	
    Serial.println("gmail Connected");	
    delay(500);	
  }	
  */	
}	

void loop(){	
  data = distance;	
  
  // Clears the trigPin	
  digitalWrite(trigPin, LOW);	
  delayMicroseconds(2);	
  digitalWrite(trigPin, HIGH);            // Sets the trigPin on HIGH state for 10 micro seconds	
  delayMicroseconds(10);	
  digitalWrite(trigPin, LOW);	
  duration = pulseIn(echoPin, HIGH);      // Reads the echoPin, returns the sound wave travel time in microseconds	
  
  // Calculating the distance	
  currentDistance= duration*0.034/2;	
  	
  Serial.print("Current Distance: ");     	
  Serial.println(currentDistance);        // Prints the distance on the Serial Monitor	
  delay(1000);                            //delay 1000 microseconds	
  server.handleClient();	
  if ((currentDistance >= (lastDistance - sensitivityMotionRange)) && (currentDistance <= (lastDistance + sensitivityMotionRange))) {
      count++;
      Serial.print("Number of Counts: ");
      Serial.println(count);
      if((count == (stopTimeInMinutes * 60) -1 )){
        Serial.println((String)" Same value got in " + stopTimeInMinutes  + " Minute");
        timeClient.update();
        String stoppedTime  = ((String) ""+ timeClient.getHours() +":"+ (timeClient.getMinutes() - stopTimeInMinutes) + ":" + timeClient.getSeconds());
        Serial.println((String)" Stopped at : " + stoppedTime + " ");
        sendEmail(stoppedTime);
        isEmailSent = 1;
      }else{
        }
        //count = 0;
  }else{
    if(isEmailSent == 1){
      Serial.println("Mail Sent,Restarted");
      sendRestartMail();
      isEmailSent=0;
      }
    count = 0;
   
    }
  lastDistance = currentDistance;
}

void sendEmail(String stoppedTime){	
  Serial.println("Email Triggering WebHook Started....");	
  HTTPClient http;                                //Declare an object of class HTTPClient	
  String url = ((String)"http://maker.ifttt.com/trigger/nodemcumailsender/with/key/ecAAy3yYkUkpGq2yzymKDS25vcsVmmfuCIiv?value1="+stoppedTime);	
  Serial.println(url);	
  http.begin(url); //Specify request destination	
  int httpCode = http.GET();                      //Send the request	
  if (httpCode > 0) {                             //Check the returning code	
      String payload = http.getString();          //Get the request response payload	
      Serial.println(payload);                    //Print the response payload	
  }	
  http.end();                                     //Close connection	
  Serial.println((String)"Email Triggering WebHook Completed....Http Code : "+ httpCode);	
}	
 

void sendStartMail(){
  Serial.println("Start Email Triggering WebHook Started....");
  HTTPClient http;                                //Declare an object of class HTTPClient
  timeClient.update();
  String startedTime  = ((String)  ""+  timeClient.getHours() +":"+ timeClient.getMinutes() + ":" + timeClient.getSeconds()+ " ");
  String url = ((String)"http://maker.ifttt.com/trigger/nodemcustartmail/with/key/ecjpGq2yzymKDS25vcsVmmfuCIiv?value1="+getCurrentTime()+ "&value2="+ startedTime);
  Serial.println(url);
  http.begin(url); //Specify request destination
  int httpCode = http.GET();                      //Send the request
  if (httpCode > 0) {                             //Check the returning code
      String payload = http.getString();          //Get the request response payload
      Serial.println(payload);                    //Print the response payload
  }
  http.end();                                     //Close connection
  Serial.println((String)"Start Email Triggering WebHook Completed....Http Code : "+ httpCode);
}

void sendRestartMail(){
  Serial.println("Start Email Triggering WebHook Started....");
  HTTPClient http;                                //Declare an object of class HTTPClient
  timeClient.update();
  String startedTime  = ((String) ""+ timeClient.getHours() +":"+ timeClient.getMinutes() + ":" + timeClient.getSeconds()+ " ");
  Serial.println((String)" Module restarted at : " + getCurrentTime() + " ");
  String url = ((String)"http://maker.ifttt.com/trigger/MachineStarted/with/key/ecjBZ4QKkisy3yzymKDS25vcsVmmfuCIiv?value1="+getCurrentTime() +"&value2="+ startedTime);
  Serial.println(url);
  http.begin(url); //Specify request destination
  int httpCode = http.GET();                      //Send the request
  if (httpCode > 0) {                             //Check the returning code
      String payload = http.getString();          //Get the request response payload
      Serial.println(payload);                    //Print the response payload
  }
  http.end();                                     //Close connection
  Serial.println((String)"Restart Email Triggering WebHook Completed....Http Code : "+ httpCode);
}


String getCurrentTime(){
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
//  Serial.print("Epoch Time: ");
//  Serial.println(epochTime);
//  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime);
 
  int currentMonth = ptm->tm_mon+1;
  int monthDay = ptm->tm_mday;
  String daten =  (String)"" + (months[currentMonth-1]) + "-" + monthDay + "-" + (ptm->tm_year+1900 );
  return daten;
}


//=============================================================================	
//        WITH GMAIL RELAY	
//=============================================================================	
//void sendEmail(){	
//  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status	
//    HTTPClient httpsClient;    //Declare object of class HTTPClient	
//    http.begin("https://maker.ifttt.com/trigger/nodemcu_gmail_send/with/key/iwHWHSAXLwrCidHJnqdyvXt3WsRzVXt1dfwU04TGD");  //Specify request destination	
//    http.addHeader("Content-Type", "text/plain");  //Specify content-type header	
//    int httpCode = http.POST(" ");            //Send the request	
//    Serial.print(httpCode);	
//    if (httpCode > 0) {                    //Check the returning code	
//      String payload = http.getString();   //Get the request response payload	
//      Serial.println(payload);             //Print the response payload	
//    }	
//     http.end();   //Close connection	
//  }	
//	
//  httpsClient.print(String("POST ") + "/trigger/nodemcu_gmail_send/with/key/iwHWHSAXLwrCznidHt3WsRzVXt1dfwU04TGD" + " HTTP/1.1\r\n" +	
//               "Host: maker.ifttt.com "+ "\r\n" +	
//               "Access-Token: *************"+ "\r\n" +	
//               "Content-Type: application/json"+ "\r\n" +	
//               "Content-Length: 20"+ "\r\n" +	
//               "body: Hello World" + "\r\n\r\n");	
//  Serial.println("request sent");	
//}	
//byte sendEmail(){	
//  if (client.connect(relayServer, 25) == 1){	
//    Serial.println(F("connected"));	
//  }else{	
//    Serial.println(F("connection failed"));	
//    return 0;	
//  }	
//  if (!emailResp())	
//  return 0;	
//	
//  Serial.println(F("Sending EHLO"));	
//  client.println("EHLO www.example.com");	
//  if (!emailResp())	
//  return 0;	
//	
//  client.println("STARTTLS");	
//  if (!emailResp())	
//  return 0;	
//	
//  Serial.println(F("Sending auth login"));	
//  client.println("AUTH LOGIN");	
//  if (!emailResp())	
//  return 0;	
//	
//  Serial.println(F("Sending User"));	
//	
//  client.println("dGhlbWFpbFHNX3Vwcm9qZWN0QGdtYWlsLmNvbQ=="); 	
//  if (!emailResp())	
//  return 0;	
//	
//  Serial.println(F("Sending Password"));	
//	
//  client.println("YW13dmzzsDHFhdXB4c2ZubnN1aQ==");	
//  if (!emailResp())	
//  return 0;	
//	
//  Serial.println(F("Sending From"));	
//	
//  client.println(F("MAIL From: thzzzzroject@gmail.com"));	
//  if (!emailResp())	
//  return 0;	
//	
//  Serial.println(F("Sending To"));	
//  client.println(F("RCPT To: zzzzzzzzzzzzzzz@gmail.com"));	
//  if (!emailResp())	
//  return 0;	
//	
//  Serial.println(F("Sending DATA"));	
//  client.println(F("DATA"));	
//  if (!emailResp())	
//  return 0;	
//  Serial.println(F("Sending email"));	
//	
//  client.println(F("To: zzzzzzzzz@gmail.com"));	
//	
//  client.println(F("From: tzzzzzproject@gmail.com"));	
//  client.println(F("Subject: ESP8266 test e-mail\r\n"));	
//  client.println(F("This is is a test e-mail sent from ESP8266.\n"));	
//  client.println(F("Second line of the test e-mail."));	
//  client.println(F("Third line of the test e-mail."));	
//	
//  client.println(F("."));	
//  if (!emailResp())	
//  return 0;	
//	
//  Serial.println(F("Sending QUIT"));	
//  client.println(F("QUIT"));	
//  if (!emailResp())	
//  return 0;	
//	
//  client.stop();	
//  Serial.println(F("disconnected"));	
//  return 1;	
//}	
//byte emailResp(){	
//  byte responseCode;	
//  byte readByte;	
//  int loopCount = 0;	
//	
//  while (!client.available()){	
//    delay(1);	
//    loopCount++;	
//    if (loopCount > 20000){	
//      client.stop();	
//      Serial.println(F("\r\nTimeout"));	
//      return 0;	
//    }	
//  }	
//  responseCode = client.peek();	
//  while (client.available()){	
//    readByte = client.read();	
//    Serial.write(readByte);	
//  }	
//	
//  if (responseCode >= '4'){	
//    return 0;	
//  }	
//  return 1;	
//}	
