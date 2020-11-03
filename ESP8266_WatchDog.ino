#include <ESP8266WiFi.h>
#include <WiFiUDP.h> 
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266httpUpdate.h>
//#include <DNSServer.h>
#include <TimeLib.h>
#include <Wire.h>
//#include <SPI.h>
#include <EEPROM.h>
#include <stdio.h>
#include "SSD1306.h"
//#include "SH1106.h"
#include "SH1106Wire.h"
#include "ds3231.h"
#include <ESP8266Ping.h>
#include <ESPMail.h>


#define BUFF_MAX 256
char buff[BUFF_MAX]; 
const byte LED = BUILTIN_LED ;  // = 16  4 ? D4
const byte MAX_WIFI_TRIES = 60 ;
const int MAX_EEPROM = 2000 ;
const int PROG_BASE = 256 ;   // where the program specific information starts in eeprom (above GHKS)
const byte MAX_VALVE =  16 ;   // these two easily changed just watch the memory 

#define MYVER 0x12435678     // change this if you change the structures that hold data that way it will force a "backinthebox" to get safe and sane values from eeprom
const int NTP_PACKET_SIZE = 48;       // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];   // buffer to hold incoming and outgoing packets

#define HAVE_OLED 1
#if defined(HAVE_OLED)
SSD1306 display(0x3c, 5, 4);   // GPIO 5 = D1, GPIO 4 = D2   - onboard display 0.96" 
//SH1106Wire display(0x3c, 4, 5);   // arse about ??? GPIO 5 = D1, GPIO 4 = D2  -- external ones 1.3"
#endif

// GPIO5  is the input
// GPIO4  is the output
//
//
//  Terminal Strip
//  ============================
//  1 -   Relay NO
//  2 -   Relay Common
//  3 -   Relay NC
//  4 -   +5V Supply
//  5 -   + LED (opto isolator )
//  6 -   - LED (opto isolator )



int bSaveReq = 0 ;
int iUploadPos = 0 ;
bool bDoTimeUpdate = false ;
long  MyCheckSum ;
long  MyTestSum ;
long lTimePrev ;
long lTimePrev2 ;
long lMinUpTime = 0 ;
bool bConfig = false ;
bool hasRTC = false ;

IPAddress MyIP ;
IPAddress MyIPC  ;

WiFiUDP ntpudp;
WiFiUDP ctrludp;
ESPMail WDmail;

ESP8266WebServer server(80) ;
ESP8266WebServer OTAWebServer(81) ;
ESP8266HTTPUpdateServer OTAWebUpdater ;
time_t chiptime ; 

byte rtc_sec = 0 ;
byte rtc_min = 0 ;
byte rtc_hour = 0  ;
uint8_t rtc_status ;
float rtc_temp ;
long lScanCtr = 0 ;
long lScanLast = 0 ;

long lRebootCode = 0 ;
struct ts tc;  
bool bPrevConnectionStatus = false;
unsigned long lTimeNext = 0 ;           // next network retry

char Toleo[10] = {"Ver 1.1\0"}  ;
char cssid[32] = {"WatchDog_XXXXXXXX\0"} ;
char *host = "WatchDog_00000000\0";                // overwrite these later with correct chip ID


typedef struct __attribute__((__packed__)) {     // eeprom stuff
  unsigned int localPort = 2390;          // 2 local port to listen for NTP UDP packets
  unsigned int localPortCtrl = 8666;      // 4 local port to listen for Control UDP packets
  unsigned int RemotePortCtrl = 8664;     // 6 local port to listen for Control UDP packets
  long lNodeAddress ;                     // 22 
  float fTimeZone ;                       // 26 
  char RCIP[18] ;                         // (192,168,2,255)  30
  char NodeName[32] ;                     // 46 
  char nssid[16] ;                        // 62  
  char npassword[16] ;                    // 78
  time_t AutoOff_t ;                      // 82     auto off until time > this date   
  uint8_t lDisplayOptions  ;              // 83 
  uint8_t lNetworkOptions  ;              // 84 
  uint8_t lSpare1  ;                      // 85 
  uint8_t lSpare2  ;                      // 86 
  char timeServer[24] ;                   // 110   = {"au.pool.ntp.org\0"}
  char cpassword[16] ;                    // 126
  long lVersion  ;                        // 130
  IPAddress IPStatic ;                    // (192,168,0,123)   
  IPAddress IPGateway ;                   // (192,168,0,1)    
  IPAddress IPMask ;                      // (255,255,255,0)   
  IPAddress IPDNS ;                       // (192,168,0,15)   
} general_housekeeping_stuff_t ;          // computer says it's 136 not 130 ??? is my maths crap ????

general_housekeeping_stuff_t ghks ;

typedef struct __attribute__((__packed__)) {     // eeprom stuff
  long RebootInterval ;                   // intervale in minutes between reboots
  long MinRecycleTime ;                   // Minimum interval in minutes between Reboots
  long ReBootOffTime ;                    // Off time in seconds of the reboot 
  uint8_t RelayPin  ;                      
  uint8_t ActiveValue  ;                      
  int  ReBootOption ;
  byte ReBoot_sec   ;
  byte ReBoot_min   ;
  byte ReBoot_hour   ;
  byte ReBoot_wdays   ;
  IPAddress IPPing  ;  
  long PingMax ;
  long PingFreq ;
  long SelfReBoot ;
  int  SMTP_Port;
  char SMTP_Server[48] ;
  char SMTP_User[48] ;
  char SMTP_Password[48] ;
  char SMTP_FROM[48] ;
  char SMTP_TO[48] ;
  char SMTP_CC[48] ;
  char SMTP_BCC[48] ;
  bool SMTP_bSecure ;
  char SMTP_Message[64] ;
  char SMTP_Subject[64] ;
} milo_watch_dog_e_t ;         

milo_watch_dog_e_t mwde ;

typedef struct __attribute__((__packed__)) {     // memory stuff
  time_t LastReboots[32] ;                 // date time stamps of 
  long MemoryIndex = 0 ;
  long MinSinceLastReboot = 0 ;
  long MinSinceLastPing = 0 ;
  long MinSinceLastSelfReboot = 0 ;
  long lSecOff = 0 ;
  bool bInReboot = false ;
  bool bDoReboot = false ;
  bool bInPing = false ;
  bool iPingNo ;
  int  iPingTime ;
  bool bSendEmail = false ;
} milo_watch_dog_m_t ;         

milo_watch_dog_m_t mwdm ;

void setup() {
int i , k , j = 0; 
  
  lRebootCode = random(1,+2147483640) ;  // want to change it straight away
  Serial.begin(115200);
  Serial.setDebugOutput(true);  
  Serial.println("");          // new line after the startup burst
 
  pinMode(BUILTIN_LED,OUTPUT);  //  D4 builtin LED
  EEPROM.begin(MAX_EEPROM);
  LoadParamsFromEEPROM(true);  

  Serial.println("Start Display...");

#if defined(HAVE_OLED)
  display.init();
  if (( ghks.lDisplayOptions & 0x01 ) != 0 ) {  // if bit one on then flip the display
    display.flipScreenVertically();
  }

  display.clear();                // show start screen 
  display.setTextAlignment(TEXT_ALIGN_CENTER);  
  display.setFont(ArialMT_Plain_16);
  display.drawString(63, 0, "Watch Dog");
  display.drawString(63, 16, "Relay");
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);  
  display.drawString(0, 40, "Copyright (c) 2020");
  display.drawString(0, 50, "Dougal Plummer");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);  
  display.drawString(127, 50, String(Toleo));
  display.display();

  display.setFont(ArialMT_Plain_10);
#endif

  if ( MYVER != ghks.lVersion ) {
//  if ( true ) {  //false
    BackInTheBoxMemory();         // load defaults if blank memory detected but dont save user can still restore from eeprom
    Serial.println("Loaded memory defaults...");
    delay(2000);
  }

#if defined(HAVE_OLED)
  if ( (mwde.RelayPin==4 ) || (mwde.RelayPin==5)){
    mwde.RelayPin = 12 ;
  }
#endif
  pinMode(mwde.RelayPin ,OUTPUT);  // Setup the Relay pin 

  WiFi.disconnect();
  Serial.println("Configuring soft access point...");
  WiFi.mode(WIFI_AP_STA);  // we are having our cake and eating it eee har
  sprintf(cssid,"WatchDog_%08X\0",ESP.getChipId());
  if ( cssid[0] == 0 || cssid[1] == 0 ){   // pick a default setup ssid if none
    sprintf(ghks.cpassword,"\0");
  }
  MyIPC = IPAddress (192, 168, 5 +(ESP.getChipId() & 0x7f ) , 1);
  WiFi.softAPConfig(MyIPC,MyIPC,IPAddress (255, 255, 255 , 0));  
  Serial.println("Starting access point...");
  Serial.print("SSID: ");
  Serial.println(cssid);
  Serial.print("Password: >");
  Serial.print(ghks.cpassword);
  Serial.println("< " + String(ghks.cpassword[0]));
  if (( ghks.cpassword[0] == 0 ) || ( ghks.cpassword[0] == 0xff)){
    WiFi.softAP((char*)cssid);                   // no passowrd
  }else{
    WiFi.softAP((char*)cssid,(char*) ghks.cpassword);
  }
  MyIPC = WiFi.softAPIP();  // get back the address to verify what happened
  Serial.print("Soft AP IP address: ");
  snprintf(buff, BUFF_MAX, ">> IP %03u.%03u.%03u.%03u <<", MyIPC[0],MyIPC[1],MyIPC[2],MyIPC[3]);      
  Serial.println(buff);
  
  bConfig = false ;   // are we in factory configuratin mode
  if ( ghks.lNetworkOptions != 0 ) {
    WiFi.config(ghks.IPStatic,ghks.IPGateway,ghks.IPMask,ghks.IPDNS ); 
  }
  if ( ghks.npassword[0] == 0 ){
    WiFi.begin((char*)ghks.nssid);                    // connect to unencrypted access point      
  }else{
    WiFi.begin((char*)ghks.nssid, (char*)ghks.npassword);  // connect to access point with encryption
  }
  while (( WiFi.status() != WL_CONNECTED ) && ( j < MAX_WIFI_TRIES )) {
    j = j + 1 ;
    delay(250);
#if defined(HAVE_OLED)
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Chip ID " + String(ESP.getChipId(), HEX) );
    display.drawString(0, 9, String("SSID:") );
    display.drawString(0, 18, String("Password:") );
    display.drawString(0, 36 , String(1.0*j/4) + String(" (s)" ));   
    display.drawString(42, 36 , String(ghks.NodeName));   
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128 , 0, String(WiFi.RSSI()));
    display.drawString(128, 9, String(ghks.nssid) );
    display.drawString(128, 18, String(ghks.npassword) );
    display.drawString(j*4, 27 , String(">") );
    snprintf(buff, BUFF_MAX, ">>  IP %03u.%03u.%03u.%03u <<", MyIPC[0],MyIPC[1],MyIPC[2],MyIPC[3]);            
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(63 , 54 ,  String(buff) );
    display.display();     
#endif    
    digitalWrite(BUILTIN_LED,!digitalRead(BUILTIN_LED));
  } 
  if ( j >= MAX_WIFI_TRIES ) {
     bConfig = true ;
     WiFi.disconnect();
     
  }else{
     Serial.println("");
     Serial.println("WiFi connected");  
     Serial.print("IP address: ");
     MyIP =  WiFi.localIP() ;
//     Serial.println(MyIP) ;
     snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", MyIP[0],MyIP[1],MyIP[2],MyIP[3]);            
     Serial.println(buff);
  }
  if (ghks.localPortCtrl == ghks.localPort ){             // bump the NTP port up if they ar the same
    ghks.localPort++ ;
  }
//    Serial.println("Starting UDP");
    ntpudp.begin(ghks.localPort);                      // this is the recieve on NTP port
//    Serial.print("NTP Local UDP port: ");
//    Serial.println(ntpudp.localPort());
    ctrludp.begin(ghks.localPortCtrl);                 // recieve on the control port
    Serial.print("Control Local UDP port: ");
    Serial.println(ctrludp.localPort());
                                                // end of the normal setup
 
  sprintf(host,"WatchDog_%08X\0",ESP.getChipId());
  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
//    Serial.println("MDNS responder started");
//    Serial.print("You can now connect to http://");
//    Serial.print(host);
//    Serial.println(".local");
  }


  Serial.println("HTTP server starting...");
  server.on("/", handleRoot);
  server.on("/setup", handleRoot);
  server.on("/scan", i2cScan);
  server.on("/stime", handleRoot);
  server.on("/info", handleInfo);
  server.on("/eeprom", DisplayEEPROM);
  server.on("/backup", HTTP_GET , handleBackup);
  server.on("/backup.txt", HTTP_GET , handleBackup);
  server.on("/backup.txt", HTTP_POST,  handleRoot, handleFileUpload);
  server.onNotFound(handleNotFound);  
  server.begin();
  Serial.println("HTTP server UP");

  tc.mon = 0 ;
  tc.wday = 0 ;
  DS3231_init(DS3231_INTCN); // look for a rtc
  DS3231_get(&tc);
  rtc_status = DS3231_get_sreg();
  if (((tc.mon < 1 )|| (tc.mon > 12 ))&& (tc.wday>8)){  // no rtc to load off
    Serial.println("What NO DS3231 RTC ?");
  }else{
    setTime((int)tc.hour,(int)tc.min,(int)tc.sec,(int)tc.mday,(int)tc.mon,(int)tc.year ) ; // set the internal RTC
    hasRTC = true ;
    Serial.println("Look like it has DS3231 RTC ?");
    rtc_temp = DS3231_get_treg(); 
    rtc_hour = hour();  // dont need to updte the time if we have an RTC onboard --- assume its working ok
  }
 
  rtc_min = minute();
  rtc_sec = second();
  OTAWebUpdater.setup(&OTAWebServer);
  OTAWebServer.begin();  

  randomSeed(now());                       // now we prolly have a good time setting use this to roll the dice for reboot code
  lRebootCode = random(1,+2147483640) ;
  WDmail.begin();
}

void loop() {
long lTime ;  
long lRet ;
int i , j , k  ;
int x , y ;
bool bSendCtrlPacket ;
bool bDirty = false ;
bool bDirty2 = false ;
long lTD ;  

  server.handleClient();
  OTAWebServer.handleClient();
  lScanCtr++ ;
  if (second() != rtc_sec) {                                // do onlyonce a second
//    if ( bComsIn ){
      digitalWrite(LED,!digitalRead(LED));
//      bComsIn= false ;
//    }

    if (mwdm.bInReboot ) {
      if (mwdm.lSecOff > 0 )
        mwdm.lSecOff--  ;
      if (mwdm.lSecOff == 0 ) {
        mwdm.MinSinceLastReboot = 0 ; 
        mwdm.bInReboot = false ;
      }
      if (mwde.ActiveValue == 0 ){
        digitalWrite( mwde.RelayPin , HIGH);
      }else{
        digitalWrite( mwde.RelayPin , LOW);        
      }
    }else{
      if (mwde.ActiveValue == 0 ){
        digitalWrite( mwde.RelayPin , LOW);
      }else{
        digitalWrite( mwde.RelayPin , HIGH);        
      }  
    }
#if defined(HAVE_OLED)
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());
    display.drawString(0 , 0, String(buff) );
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(127 , 0, String(WiFi.RSSI()));
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    switch (rtc_sec & 0x03) {
      case 1:
        snprintf(buff, BUFF_MAX, "IP %03u.%03u.%03u.%03u", MyIP[0], MyIP[1], MyIP[2], MyIP[3]);
        break;
      case 2:
        snprintf(buff, BUFF_MAX, ">>  IP %03u.%03u.%03u.%03u <<", MyIPC[0], MyIPC[1], MyIPC[2], MyIPC[3]);
        break;
      default:
        snprintf(buff, BUFF_MAX, "%s", cssid );
        break;
    }
    display.drawString(64 , 53 ,  String(buff) );

    display.setTextAlignment(TEXT_ALIGN_CENTER);
    snprintf(buff, BUFF_MAX, "%d:%02d:%02d",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));    
    display.drawString(64 , 44, "UpTime "+ String(buff));

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    if (mwdm.iPingTime>=0){
      display.drawString(0 , 10, "Last Ping " + String(mwdm.iPingTime) + " (ms)");
    }else{
      display.fillRect(0, 11, 128, 11);
      display.setColor(INVERSE);
      display.drawString(0 , 10, "Last Ping -> FAILED <-");
      display.setColor(WHITE);
      
    }

//    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0 , 20, "Next Ping " + String(mwde.PingFreq - mwdm.MinSinceLastPing) + " (min)");

//    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0 , 30, "Next Reboot " + String(mwde.RebootInterval - mwdm.MinSinceLastReboot) + " (min)");
    
    display.display();
#endif
    
    rtc_sec = second();
    lScanLast = lScanCtr ;
    lScanCtr = 0 ;
  }

  if ( rtc_min != minute()){
    lMinUpTime++ ;
    if (mwde.PingFreq > 0){
      if ( !mwdm.bInPing ){
        mwdm.MinSinceLastPing++ ;
      }
    }
    if ( !mwdm.bInReboot ){
      mwdm.MinSinceLastReboot++ ;
    }    
     mwdm.MinSinceLastSelfReboot++ ;
    if (( year() < 1980 )|| (bDoTimeUpdate)) {  // not the correct time try to fix every minute
      if ( !bConfig ) { // ie we have a network
        sendNTPpacket(ghks.timeServer); // send an NTP packet to a time server  
        bDoTimeUpdate = false ;
      }
    }
    if ( hasRTC ){
      rtc_temp = DS3231_get_treg(); 
    }
    rtc_min = minute() ;
    if (WiFi.isConnected())  {
      MyIP =  WiFi.localIP() ;
    }
  }
  
  if (( mwdm.MinSinceLastSelfReboot > mwde.SelfReBoot ) && ( mwde.SelfReBoot > 0 )) {
    ESP.restart() ;
  }

  if ( !mwdm.bInReboot ){
    if (( mwde.RebootInterval != 0 ) && ( mwdm.MinSinceLastReboot > mwde.RebootInterval )&&( mwdm.MinSinceLastReboot > mwde.MinRecycleTime ))   {
      mwdm.bDoReboot = true ;
    }
  }
  
  if (mwdm.bInPing ) {
    if (WiFi.isConnected())  { // dont ping if no wifi
      if ((mwde.IPPing[0] != 0) && (mwde.IPPing[1] != 0) && (mwde.IPPing[2] != 0) && (mwde.IPPing[3] != 0)) {
        if ( Ping.ping(mwde.IPPing,4) ){
          mwdm.iPingTime = Ping.averageTime() ;
          if (mwdm.iPingTime > mwde.PingMax  ) {
            mwdm.bDoReboot = true ;            
          }
        }else{
          mwdm.iPingTime = -1 ; 
          mwdm.bDoReboot = true ;      
        }
      }
    }else{
      mwdm.iPingTime = -1 ;
    }
    mwdm.bInPing = false ;
    mwdm.MinSinceLastPing = 0 ;
  }else{
    if (( mwdm.MinSinceLastPing > mwde.PingFreq ) && ( mwde.PingFreq != 0 )){
      mwdm.bInPing = true ;
      mwdm.iPingTime = 0 ;
    }    
  }
  
  if (( year() > 2000 ) && ( now() > ghks.AutoOff_t )){                     // dont do this if time is like ... BULLSHIT --switch off if time is crap (allow manual mode only
    k = dayOfWeek(now()) ;
    if ((( mwde.ReBoot_wdays & ( 0x01 << (k-1) )) != 0  ) ) {   // check if enabled for these days          && (( mwde.ReBoot_wdays & ( 0x80 )) != 0  )
      if ((mwde.ReBoot_hour==hour())&&(mwde.ReBoot_min==minute())&&(mwde.ReBoot_sec==second())){
          mwdm.bDoReboot = true ;    
      }
    }
  }
       
  if ( mwdm.bDoReboot ){  // want to initial the process
    if ( mwdm.bInReboot ){
      mwdm.bDoReboot = false ;
    }else{
      mwdm.bInReboot = true ;
      mwdm.lSecOff = mwde.ReBootOffTime ;
    }  
  }

  lRet = ntpudp.parsePacket() ; // this is actually server not a client ie we are doing time
  if ( lRet != 0 ) {
    processNTPpacket();
  }

  if ( mwdm.bSendEmail && WiFi.isConnected())  {
    SendEmailToClient();
    mwdm.bSendEmail = false ;
  }

  if (!WiFi.isConnected())  {
    lTD = (long)lTimeNext-(long) millis() ;
    if (( abs(lTD)>40000)||(bPrevConnectionStatus)){ // trying to get roll over protection and a 30 second retry
      lTimeNext = millis() - 1 ;
/*      Serial.print(millis());
      Serial.print(" ");
      Serial.print(lTimeNext);
      Serial.print(" ");
      Serial.println(abs(lTD));*/
    }
    bPrevConnectionStatus = false;
    if ( lTimeNext < millis() ){
      Serial.println(String(buff )+ " Trying to reconnect WiFi ");
      WiFi.disconnect(false);
//      Serial.println("Connecting to WiFi...");
      WiFi.mode(WIFI_AP_STA);
      if ( ghks.lNetworkOptions != 0 ) {            // use ixed IP
        WiFi.config(ghks.IPStatic, ghks.IPGateway, ghks.IPMask, ghks.IPDNS );
      }
      if ( ghks.npassword[0] == 0 ) {
        WiFi.begin((char*)ghks.nssid);                    // connect to unencrypted access point
      } else {
        WiFi.begin((char*)ghks.nssid, (char*)ghks.npassword);  // connect to access point with encryption
      }
      lTimeNext = millis() + 30000 ;
    }
  }else{
    if ( !bPrevConnectionStatus  ){
      MyIP = WiFi.localIP() ;
      bPrevConnectionStatus = true ;
    }
  }
  
}
