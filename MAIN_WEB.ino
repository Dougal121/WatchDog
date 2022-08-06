void SerialOutParams(){
String message ;
   
  message = "Web Request URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  Serial.println(message);
  message = "";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  Serial.println(message);
}

void SendHTTPHeader(){
  String strTmp = "" ;
  server.sendHeader(F("Server"),F("ESP8266-on-ice"),false);
  server.sendHeader(F("X-Powered-by"),F("Dougal-1.0"),false);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  String message = F("<!DOCTYPE HTML>");
  message += "<head><title>Team Trouble - Watch Dog Timer " + String(Toleo) + "</title>";
  message += F("<meta name=viewport content='width=320, auto inital-scale=1'>");
  message += F("</head><body><html lang='en'><center><h3>");   
  strTmp = String(mwdm.ADC_Value) + " " + String(ghks.ADC_Unit) ;
  message += String(WiFi.RSSI()) +" (dBm) <a title='click for home / refresh' href='/'>"+String(ghks.NodeName)+"</a> " + strTmp + "</h3>";
  server.sendContent(message) ;   
}



void SendHTTPPageFooter(){
  String message =  F("<br><a href='/?command=1'>Load Parameters from EEPROM</a><br><br><a href='/?command=667'>Reset Memory to Factory Default</a><br><a href='/?command=665'>Sync UTP Time</a><br><a href='/?command=668'>Clear Log Memory</a><br><a href='/stime'>Manual Time Set</a><br><a href='/scan'>I2C Scan</a><br><a href='/iolocal'>Local I/O Mapping</a><br>") ;     
  message += "<a href='/?reboot=" + String(lRebootCode) + "'>Reboot</a><br>";
  message += F("<a href='/?command=42'>Send Test Email</a><br>");
  message += F("<a href='/eeprom'>EEPROM Memory Contents</a><br>");
  message += F("<a href='/email'>EMail Setup</a><br>");
  message += F("<a href='/log1'>Data Logs Page</a><br>");
  message += F("<a href='/chart1'>Chart</a><br>");  
  message += F("<a href='/setup'>Node Setup</a><br>");
  message += F("<a href='/info'>Node Infomation</a><br>");
  if (!WiFi.isConnected()) {
    snprintf(buff, BUFF_MAX, "%u.%u.%u.%u", MyIPC[0],MyIPC[1],MyIPC[2],MyIPC[3]);
  }else{
    snprintf(buff, BUFF_MAX, "%u.%u.%u.%u", MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
  }
  message += "<a href='http://" + String(buff) + ":81/update'>OTA Firmware Update</a><br>";  
  message += "<a href='https://github.com/Dougal121/WatchDog'>Source at GitHub</a><br>";  
  message += "<a href='http://" + String(buff) + "/backup'>Backup / Restore Settings</a><br><br>";  
  snprintf(buff, BUFF_MAX, "%d:%02d:%02d",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));
  message += "WatchDog Uptime <b>"+String(buff)+"</b> (day:hr:min) <br>" ;
  
  message += F("</body></html>\r\n");
  server.sendContent(message) ;  
  message = "" ;     
}


void handleNotFound(){
  String message = F("Seriously - No way DUDE\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, F("text/plain"), message);
//  Serial.print(message);
}

 
void handleRoot() {
  boolean currentLineIsBlank = true;
  tmElements_t tm;
  long  i = 0 ;
  int ii  ;
  int iProgNum = 0;
  int j ;
  int k , kk , iTmp ;
  boolean bExtraValve = false ;
  uint8_t iPage = 0 ;
  int iAType = 0 ;
  boolean bDefault = true ;
//  int td[6];
  long lTmp ; 
  String MyCheck , MyColor , MyNum , MyCheck2 ;
  byte mac[6];
  String message ;
  String pinname ;
  String bgcolor ;
  
//  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 1:  // load values
          LoadParamsFromEEPROM(true);
//          Serial.println("Load from EEPROM");
        break;
        case 2: // Save values
          LoadParamsFromEEPROM(false);
//          Serial.println("Save to EEPROM");
        break;
        case 3: // valve setup
          bExtraValve = true ;  // makes the option active
          iPage = 1 ;
        break;
        case 4: // 
          bExtraValve = true ;  // makes the option active
          iPage = 2 ;
        break;
        case 5: // Fertigation
          iPage = 2 ;
        break;
        case 8: //  Cold Reboot
          ESP.reset() ;
        break;
        case 9: //  Warm Reboot
          ESP.restart() ;
        break;
        case 42:
          mwdm.bSendEmail = true ;
        break;
        case 667: // wipe the memory to factory default
          BackInTheBoxMemory();
        break;
        case 69: // wipe the memory to factory default
          BackInTheBoxMemory();
          LoadParamsFromEEPROM(false);
        break;
        case 665:
          sendNTPpacket(ghks.timeServer); // send an NTP packet to a time server  once and hour  
        break;
        case 668:
          clearPoweLog();
        break;
      }  
    }
    i = String(server.argName(j)).indexOf("rebootnow");  // the attached device
    if (i != -1){  // 
      mwdm.bDoReboot = true ;
    }
    i = String(server.argName(j)).indexOf("reboot");   // this device
    if (i != -1){  // 
      if (( lRebootCode == String(server.arg(j)).toInt() ) && (lRebootCode>0 )){  // stop the phone browser being a dick and retry resetting !!!!
        ESP.restart() ;        
      }
    }

    i = String(server.argName(j)).indexOf("stime");
    if (i != -1){  // 
      tm.Year = (String(server.arg(j)).substring(0,4).toInt()-1970) ;
      tm.Month =(String(server.arg(j)).substring(5,7).toInt()) ;
      tm.Day = (String(server.arg(j)).substring(8,10).toInt()) ;
      tm.Hour =(String(server.arg(j)).substring(11,13).toInt()) ;
      tm.Minute = (String(server.arg(j)).substring(14,16).toInt()) ;
      tm.Second = 0 ;
      setTime(makeTime(tm));    
      if ( hasRTC ){
        tc.sec = second();     
        tc.min = minute();     
        tc.hour = hour();   
        tc.wday = dayOfWeek(makeTime(tm));            
        tc.mday = day();  
        tc.mon = month();   
        tc.year = year();       
        DS3231_set(tc);                       // set the RTC as well
        rtc_status = DS3231_get_sreg();       // get the status
        DS3231_set_sreg(rtc_status & 0x7f ) ; // clear the clock fail bit when you set the time
      }
    }        
       
    i = String(server.argName(j)).indexOf("ndadd");
    if (i != -1){  // 
      ghks.lNodeAddress = String(server.arg(j)).toInt() ;
      ghks.lNodeAddress = constrain(ghks.lNodeAddress,0,32768);
    }        
    i = String(server.argName(j)).indexOf("tzone");
    if (i != -1){  // 
      ghks.fTimeZone = String(server.arg(j)).toFloat() ;
      ghks.fTimeZone = constrain(ghks.fTimeZone,-12,12);
      bDoTimeUpdate = true ; // trigger and update to fix the time
    }        
    i = String(server.argName(j)).indexOf("disop");
    if (i != -1){  // 
      ghks.lDisplayOptions = String(server.arg(j)).toInt() ;
      ghks.lDisplayOptions = constrain(ghks.lDisplayOptions,0,255);
    }  
    i = String(server.argName(j)).indexOf("netop");
    if (i != -1){  // 
      ghks.lNetworkOptions = String(server.arg(j)).toInt() ;
      ghks.lNetworkOptions = constrain(ghks.lNetworkOptions,0,255);
    }
    
    i = String(server.argName(j)).indexOf("rbpd");
    if (i != -1){  // 
      mwde.RebootInterval = String(server.arg(j)).toInt() ;
      mwde.RebootInterval = constrain(mwde.RebootInterval,0,10080);  // a weeks worth of minutes
    }

    i = String(server.argName(j)).indexOf("rbrt");
    if (i != -1){  // 
      mwde.MinRecycleTime = String(server.arg(j)).toInt() ;
      mwde.MinRecycleTime = constrain(mwde.MinRecycleTime,0,1440);  // a days worth of minutes
    }

    i = String(server.argName(j)).indexOf("rbro");
    if (i != -1){  // 
      mwde.ReBootOffTime = String(server.arg(j)).toInt() ;
      mwde.ReBootOffTime = constrain(mwde.ReBootOffTime,0,3600);  // off for an hour max
    }
    
    i = String(server.argName(j)).indexOf("rbrp");
    if (i != -1){  // 
      mwde.RelayPin = String(server.arg(j)).toInt() ;
      mwde.RelayPin = constrain(mwde.RelayPin,0,17);  
    }

    i = String(server.argName(j)).indexOf("atrs") ;
    if (i != -1){  // 
      mwde.ActiveValue = String(server.arg(j)).toInt() ;
      mwde.ActiveValue = constrain(mwde.ActiveValue,0,1);  
    }

    i = String(server.argName(j)).indexOf("rbdt");
    if (i != -1){  // 
      mwde.ReBoot_hour =(String(server.arg(j)).substring(0,2).toInt()) ;
      mwde.ReBoot_min = (String(server.arg(j)).substring(3,5).toInt()) ;
      mwde.ReBoot_sec = (String(server.arg(j)).substring(6,8).toInt()) ;
    }    

    i = String(server.argName(j)).indexOf("rbdw");
    if (i != -1){  // 
      mwde.ReBoot_wdays = String(server.arg(j)).toInt() ;
    }
    for ( kk = 0 ; kk < 8 ; kk++ ) {
      i = String(server.argName(j)).indexOf("rbdx"+String(kk));
      if (i != -1){  //  
        if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
          mwde.ReBoot_wdays |= ( 0x1 << kk )  ;
        }
      }                
    }    
    
    i = String(server.argName(j)).indexOf("pfrq");
    if (i != -1){  // 
      mwde.PingFreq = String(server.arg(j)).toInt() ;
    }
    
    i = String(server.argName(j)).indexOf("pmax");
    if (i != -1){  // 
      mwde.PingMax = String(server.arg(j)).toInt() ;
    }

    i = String(server.argName(j)).indexOf("serb");
    if (i != -1){  // 
      mwde.SelfReBoot = String(server.arg(j)).toInt() ;
    }
    
    i = String(server.argName(j)).indexOf("lpntp");
    if (i != -1){  // 
      ghks.localPort = String(server.arg(j)).toInt() ;
      ghks.localPort = constrain(ghks.localPort,1,65535);
    }        
    i = String(server.argName(j)).indexOf("lpctr");
    if (i != -1){  // 
      ghks.localPortCtrl = String(server.arg(j)).toInt() ;
      ghks.localPortCtrl = constrain(ghks.localPortCtrl,1,65535);
    }        
    i = String(server.argName(j)).indexOf("rpctr");
    if (i != -1){  // 
      ghks.RemotePortCtrl = String(server.arg(j)).toInt() ;
      ghks.RemotePortCtrl = constrain(ghks.RemotePortCtrl,1,65535);
    }        
    i = String(server.argName(j)).indexOf("dontp");
    if (i != -1){  // have a request to request a time update
      bDoTimeUpdate = true ;
    }

    i = String(server.argName(j)).indexOf("cname");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( ghks.NodeName , sizeof(ghks.NodeName)) ;
    }


    i = String(server.argName(j)).indexOf("smse");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( mwde.SMTP_Server , sizeof(mwde.SMTP_Server)) ;
    }
    i = String(server.argName(j)).indexOf("smus");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( mwde.SMTP_User , sizeof(mwde.SMTP_User)) ;
    }
    i = String(server.argName(j)).indexOf("smpa");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( mwde.SMTP_Password , sizeof(mwde.SMTP_Password)) ;
    }
    i = String(server.argName(j)).indexOf("smfr");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( mwde.SMTP_FROM , sizeof(mwde.SMTP_FROM)) ;
    }
    i = String(server.argName(j)).indexOf("smto");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( mwde.SMTP_TO , sizeof(mwde.SMTP_TO)) ;
    }
    i = String(server.argName(j)).indexOf("smcc");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( mwde.SMTP_CC , sizeof(mwde.SMTP_CC)) ;
    }
    i = String(server.argName(j)).indexOf("smbc");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( mwde.SMTP_BCC , sizeof(mwde.SMTP_BCC)) ;
    }
    i = String(server.argName(j)).indexOf("smmb");
    if (i != -1){  
     String(server.arg(j)).toCharArray( mwde.SMTP_Message , sizeof(mwde.SMTP_Message)) ;
    }
    i = String(server.argName(j)).indexOf("smsj");
    if (i != -1){  
     String(server.arg(j)).toCharArray( mwde.SMTP_Subject , sizeof(mwde.SMTP_Subject)) ;
    }
    
    
    i = String(server.argName(j)).indexOf("smpo");
    if (i != -1){  // 
      mwde.SMTP_Port = String(server.arg(j)).toInt() ;
      mwde.SMTP_Port = constrain(mwde.SMTP_Port,1,65535);
    }
    i = String(server.argName(j)).indexOf("smbz");
    if (i != -1){  //  
        mwde.SMTP_bSecure = false ;        
    }
    i = String(server.argName(j)).indexOf("smbs");
    if (i != -1){  //  
      if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
        mwde.SMTP_bSecure = true ;        
      }
    }  

    
    i = String(server.argName(j)).indexOf("rpcip");
    if (i != -1){  // have a request to request an IP address
      ghks.RCIP[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.RCIP[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.RCIP[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.RCIP[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("staip");
    if (i != -1){  // have a request to request an IP address
      ghks.IPStatic[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.IPStatic[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.IPStatic[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.IPStatic[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("gatip");
    if (i != -1){  // have a request to request an IP address
      ghks.IPGateway[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.IPGateway[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.IPGateway[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.IPGateway[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("mskip");
    if (i != -1){  // have a request to request an IP address
      ghks.IPMask[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.IPMask[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.IPMask[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.IPMask[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("dnsip");
    if (i != -1){  // have a request to request an IP address
      ghks.IPDNS[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.IPDNS[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.IPDNS[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.IPDNS[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("ipping");
    if (i != -1){  // have a request to request an IP address
      mwde.IPPing[0] = String(server.arg(j)).substring(0,3).toInt() ;
      mwde.IPPing[1] =String(server.arg(j)).substring(4,7).toInt() ;
      mwde.IPPing[2] = String(server.arg(j)).substring(8,11).toInt() ;
      mwde.IPPing[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    
    i = String(server.argName(j)).indexOf("atoff");
    if (i != -1){  // have a request to request a time update
      tm.Year = (String(server.arg(j)).substring(0,4).toInt()-1970) ;
      tm.Month =(String(server.arg(j)).substring(5,7).toInt()) ;
      tm.Day = (String(server.arg(j)).substring(8,10).toInt()) ;
      tm.Hour =(String(server.arg(j)).substring(11,13).toInt()) ;
      tm.Minute = (String(server.arg(j)).substring(14,16).toInt()) ;
      tm.Second = 0 ;
      ghks.AutoOff_t = makeTime(tm);
    }  
    i = String(server.argName(j)).indexOf("nssid");
    if (i != -1){                                    // SSID
 //    Serial.println("SookyLala 1 ") ;
     String(server.arg(j)).toCharArray( ghks.nssid , sizeof(ghks.nssid)) ;
    }
    
    i = String(server.argName(j)).indexOf("npass");
    if (i != -1){                                    // Password
     String(server.arg(j)).toCharArray( ghks.npassword , sizeof(ghks.npassword)) ;
    }
    
    i = String(server.argName(j)).indexOf("cpass");
    if (i != -1){                                    // Password
     String(server.arg(j)).toCharArray( ghks.cpassword , sizeof(ghks.cpassword)) ;
    }
    
    i = String(server.argName(j)).indexOf("timsv");
    if (i != -1){                                    // timesvr
     String(server.arg(j)).toCharArray( ghks.timeServer , sizeof(ghks.timeServer)) ;
    }

    
  }

  SendHTTPHeader();   //  ################### START OF THE RESPONSE  ######

  if ( bSaveReq != 0 ){
    server.sendContent(F("<blink>"));      
  }   
  server.sendContent(F("<a href='/?command=2'>Save Parameters to EEPROM</a><br>")) ;     
  if ( bSaveReq != 0 ){
    server.sendContent(F("</blink><font color='red'><b>Changes Have been made to settings.<br>Make sure you save if you want to keep them</b><br></font><br>")) ;     
  }
    
  snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());
  if (ghks.fTimeZone > 0 ) {
    server.sendContent("<b>"+ String(buff) + " UTC +" + String(ghks.fTimeZone,1) ) ;   
  }else{
    server.sendContent("<b>"+ String(buff) + " UTC " + String(ghks.fTimeZone,1) ) ;       
  }
  if ( year() < 2000 ) {
    server.sendContent(F("  --- CLOCK NOT SET ---")) ;
  }
  server.sendContent(F("</b><br>")) ;  
  if ( ghks.AutoOff_t > now() )  {
    snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(ghks.AutoOff_t), month(ghks.AutoOff_t), day(ghks.AutoOff_t) , hour(ghks.AutoOff_t), minute(ghks.AutoOff_t), second(ghks.AutoOff_t));
    server.sendContent(F("<b><font color=red>Automation OFFLINE Untill ")) ;  
    server.sendContent(String(buff)) ; 
    server.sendContent(F("</font></b><br>")) ; 
  }else{
    if ( year() > 2000 ) {
      server.sendContent(F("<b><font color=green>Automation ONLINE</font></b><br>")) ;  
    }else{
      server.sendContent(F("<b><font color=green>Automation OFFLINE Invalid time</font></b><br>")) ;        
    }
  }

  if (String(server.uri()).indexOf("stime")>0) {  // ################   SETUP TIME    #######################################
    bDefault = false ;
    snprintf(buff, BUFF_MAX, "%04d/%02d/%02d %02d:%02d", year(), month(), day() , hour(), minute());
    server.sendContent("<br><br><form method=post action=" + server.uri() + "><br>Set Current Time: <input type='text' name='stime' value='"+ String(buff) + "' size=12>");
    server.sendContent(F("<input type='submit' value='SET'><br><br></form>"));
  }

    
  if (String(server.uri()).indexOf("setup")>0) {  // ################  SETUP OF THE NODE #####################################
    bDefault = false ;
    server.sendContent("<form method=post action=" + server.uri() + "><table border=1 title='Node Settings'>");
    server.sendContent(F("<tr><th>Parameter</th><th>Value</th><th><input type='submit' value='SET'></th></tr>"));
  
    server.sendContent(F("<tr><td>Controler Name</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='cname' value='"+String(ghks.NodeName)+"' maxlength=15 size=12></td><td></td></tr>");
  
    snprintf(buff, BUFF_MAX, "%04d/%02d/%02d %02d:%02d", year(ghks.AutoOff_t), month(ghks.AutoOff_t), day(ghks.AutoOff_t) , hour(ghks.AutoOff_t), minute(ghks.AutoOff_t));
    if (ghks.AutoOff_t > now()){
      MyColor =  F("bgcolor=red") ;
    }else{
      MyColor =  "" ;
    }
    server.sendContent("<tr><td "+String(MyColor)+">Auto Off Until</td><td align=center>") ; 
    server.sendContent("<input type='text' name='atoff' value='"+ String(buff) + "' size=12></td><td>(yyyy/mm/dd)</td></tr>");
  
    server.sendContent(F("<tr><td>Node Address</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='ndadd' value='" + String(ghks.lNodeAddress) + "' size=12></td><td>"+String(ghks.lNodeAddress & 0xff)+"</td></tr>");
  
    server.sendContent(F("<tr><td>Time Zone</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='tzone' value='" + String(ghks.fTimeZone,1) + "' size=12></td><td>(Hours)</td></tr>");

    server.sendContent(F("<tr><td>Display Options</td><td align=center>")) ; 
    server.sendContent(F("<select name='disop'>")) ;
    if (ghks.lDisplayOptions == 0 ){
      server.sendContent(F("<option value='0' SELECTED>0 - Normal")); 
      server.sendContent(F("<option value='1'>1 - Invert")); 
    }else{
      server.sendContent(F("<option value='0'>0 - Normal")); 
      server.sendContent(F("<option value='1' SELECTED>1 - Invert")); 
    }
    server.sendContent(F("</select></td><td></td></tr>"));

    server.sendContent(F("</form>"));

    server.sendContent("<form method=post action=" + server.uri() + "><tr><td></td><td></td><td></td></tr>") ; 
  
    server.sendContent(F("<tr><td>Local UDP Port NTP</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='lpntp' value='" + String(ghks.localPort) + "' size=12></td><td><input type='submit' value='SET'></td></tr>");
  
    server.sendContent(F("<tr><td>Local UDP Port Control</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='lpctr' value='" + String(ghks.localPortCtrl) + "' size=12></td><td></td></tr>");
  
    server.sendContent(F("<tr><td>Remote UDP Port Control</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='rpctr' value='" + String(ghks.RemotePortCtrl) + "' size=12></td><td></td></tr>");

    server.sendContent(F("<tr><td>Network SSID</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='nssid' value='" + String(ghks.nssid) + "' maxlength=15 size=12></td><td></td></tr>");

    server.sendContent(F("<tr><td>Network Password</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='npass' value='" + String(ghks.npassword) + "' maxlength=15 size=12></td><td></td></tr>");

    server.sendContent(F("<tr><td>Configure Password</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='cpass' value='" + String(ghks.cpassword) + "' maxlength=15 size=12></td><td></td></tr>");

    server.sendContent(F("<tr><td>Time Server</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='timsv' value='" + String(ghks.timeServer) + "' maxlength=23 size=12></td><td></td></tr>");
 
    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.RCIP[0],ghks.RCIP[1],ghks.RCIP[2],ghks.RCIP[3]);
    server.sendContent(F("<tr><td>Remote IP Address Control</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='rpcip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr></form>");
    
    server.sendContent("<form method=post action=" + server.uri() + "><tr><td></td><td></td><td></td></tr>") ; 

    server.sendContent(F("<tr><td>Network Options</td><td align=center>")) ; 
    server.sendContent(F("<select name='netop'>")) ;
    if (ghks.lNetworkOptions == 0 ){
      server.sendContent(F("<option value='0' SELECTED>0 - DHCP")); 
      server.sendContent(F("<option value='1'>1 - Static")); 
    }else{
      server.sendContent(F("<option value='0'>0 - DHCP")); 
      server.sendContent(F("<option value='1' SELECTED>1 - Static IP")); 
    }
    server.sendContent(F("</select></td><td><input type='submit' value='SET'></td></tr>"));
    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPStatic[0],ghks.IPStatic[1],ghks.IPStatic[2],ghks.IPStatic[3]);
    server.sendContent(F("<tr><td>Static IP Address</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='staip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPGateway[0],ghks.IPGateway[1],ghks.IPGateway[2],ghks.IPGateway[3]);
    server.sendContent(F("<tr><td>Gateway IP Address</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='gatip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");
  
    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPMask[0],ghks.IPMask[1],ghks.IPMask[2],ghks.IPMask[3]);
    server.sendContent(F("<tr><td>IP Mask</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='mskip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPDNS[0],ghks.IPDNS[1],ghks.IPDNS[2],ghks.IPDNS[3]);
    server.sendContent(F("<tr><td>DNS IP Address</td><td align=center>")) ; 
    server.sendContent("<input type='text' name='dnsip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

    server.sendContent("<tr><td>Last Scan Speed</td><td align=center>" + String(lScanLast) + "</td><td>(per second)</td></tr>" ) ;    
    if( hasRTC ){
      rtc_status = DS3231_get_sreg();
      if (( rtc_status & 0x80 ) != 0 ){
        server.sendContent(F("<tr><td>RTC Battery</td><td align=center bgcolor='red'>DEPLETED</td><td></td></tr>")) ;            
      }else{
        server.sendContent(F("<tr><td>RTC Battery</td><td align=center bgcolor='green'>-- OK --</td><td></td></tr>")) ;                    
      }
      server.sendContent("<tr><td>RTC Temperature</td><td align=center>"+String(rtc_temp,1)+"</td><td>(C)</td></tr>") ;                    
    }
    server.sendContent(F("</form></table>"));
  }
  
  
  if (bDefault) {     // #####################################   default web page control and setup  ##############################################
    server.sendContent(F("<br><b>System Setup and Status</b>"));
    message = F("<table border=1 title='System Setup and Status'>") ;
    message += F("<tr><th>Parameter</th><th>Value</th><th>Units</th><th>.</th></tr>") ;          
    snprintf(buff, BUFF_MAX, "%d:%02d:%02d",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));
    message +=  "<tr><td>WatchDog Uptime</td><td align=center>"+String(buff)+"</td><td>(day:hr:min)</td><td>.</td></tr>" ;
    if (mwdm.MinSinceLastReboot > mwde.MinRecycleTime ){
      bgcolor =  "bgcolor='green'" ;
    }else{
      bgcolor =  "bgcolor='red'" ;
    }
    message += "<tr><td>Time Since Last Reboot </td><td align=center " + bgcolor + ">"+String(mwdm.MinSinceLastReboot)+"</td><td>(min)</td><td></td></tr>" ;
    message += "<tr><td>Time Since Last Ping </td><td align=center>"+String(mwdm.MinSinceLastPing)+"</td><td>(min)</td><td></td></tr>" ;
    if ( mwdm.iPingTime >= 0 ){
      if (mwdm.iPingTime > mwde.PingMax ){
        bgcolor =  "bgcolor='red'" ;
      }else{
        bgcolor =  "bgcolor='green'" ;
      }
      if (mwdm.iPingTime==0){
        bgcolor =  "bgcolor='yellow'" ;        
      }
      message += "<tr><td>Ping Last Ping Time </td><td align=center " + bgcolor + ">"+String(mwdm.iPingTime)+"</td><td>(ms)</td><td></td></tr>" ;
    }else{
      message += "<tr><td>Ping Last Ping Time </td><td align=center bgcolor='yellow'>## FAILED ##</td><td>(ms)</td><td></td></tr>" ;      
    }
    message += "<tr><form method=post action=" + server.uri() + "><td>Reboot Period </td><td align=center><input type='text' name='rbpd' value='"+String(mwde.RebootInterval)+"' size=12></td><td>(min)</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>Min Recycle Time </td><td align=center><input type='text' name='rbrt' value='"+String(mwde.MinRecycleTime)+"' size=12></td><td>(min)</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>ReBoot Off Time </td><td align=center><input type='text' name='rbro' value='"+String(mwde.ReBootOffTime)+"' size=12></td><td>(sec)</td><td><input type='submit' value='SET'></td></form></tr>" ;

    server.sendContent(message);
    message = "" ;
    message +="<tr><form method=get action=" + server.uri() + "><td>Relay Pin</td><td align=center><select name='rbrp'>";
    for (i = 0; i < 17; i++) {
      if (mwde.RelayPin == i ){
        bgcolor = F(" SELECTED ");
      }else{
        bgcolor = "";            
      }
      switch(i){
        case 0: pinname = F("GPIO 0 - D3") ; break;
        case 1: pinname = F("GPIO 1 - D1 TXD0") ; break;
        case 2: pinname = F("GPIO 2 - D9 BUILTIN LED") ; break;
        case 3: pinname = F("GPIO 3 - D0 - RXD0") ; break;
        case 4: pinname = F("GPIO 4 - I2C SDA - Avoid") ; break;
        case 5: pinname = F("GPIO 5 - I2C SCL - Avoid") ; break;
        case 6: pinname = F("GPIO 6 - SDCLK - NA Dont Use") ; break;
        case 7: pinname = F("GPIO 7 - SDD0 - NA Dont Use") ; break;
        case 8: pinname = F("GPIO 8 - SDD1 - NA Dont Use") ; break;
        case 9: pinname = F("GPIO 9 - SDD2 - NA ? ") ; break;
        case 10: pinname = F("GPIO 10 - SDD3 - NA ? ") ; break;
        case 11: pinname = F("GPIO 11 - SDCMD - NA Dont Use") ; break;
        case 12: pinname = F("GPIO 12 - D12 - ") ; break;
        case 13: pinname = F("GPIO 13 - D11 - RXD2") ; break;
        case 14: pinname = F("GPIO 14 - D13") ; break;
        case 15: pinname = F("GPIO 15 - D10 -  TXD2") ; break;
        case 16: pinname = F("GPIO 16 - D2 -  Wake") ; break;
      }
      message += "<option value="+String(i)+ bgcolor +">" + pinname ;          
    }
    server.sendContent(message);
    message = "" ;
    message += "</select></td><td></td><td><input type='submit' value='SET'></form></td></tr>";
    
    message += "<tr><form method=post action=" + server.uri() + "><td>Active State</td><td align=center>" ; 
    message += "<select name='atrs'>"  ;
    if (mwde.ActiveValue == 0 ){
      message += F("<option value='0' SELECTED>0 LOW"); 
      message += F("<option value='1'>1 HGH"); 
    }else{
      message += F("<option value='0'>0 LOW"); 
      message += F("<option value='1' SELECTED>1 HIGH"); 
    }
    message += F("</select></td><td></td><td><input type='submit' value='SET'></td></form></tr>");

    message += "<tr><form method=post action=" + server.uri() + "><td>ReBoot Options </td><td align=center><input type='text' name='rbro' value='"+String(mwde.ReBootOption)+"' size=12></td><td></td><td><input type='submit' value='SET'></td></form></tr>" ;
    if (!mwdm.bInReboot) {
      message += "<tr><form method=post action=" + server.uri() + "><td colspan=4 align=center bgcolor='green'><input type='submit' value='### ReBoot The connected device Now ###'><input type='hidden' name='rebootnow' value='true'></td></form></tr>" ;
    }else{
      message += "<tr><form method=post action=" + server.uri() + "><td colspan=4 align=center bgcolor='red'>###### RESETTING DEVICE NOW ######</td></form></tr>" ;
    }
    server.sendContent(message) ;
    message = "" ;

    snprintf(buff, BUFF_MAX, "%02d:%02d:%02d", mwde.ReBoot_hour , mwde.ReBoot_min , mwde.ReBoot_sec); 
    message += "<tr><form method=post action=" + server.uri() + "><td>Reboot Time</td><td align=center><input type='text' name='rbdt' value='"+String(buff)+"' size=12></td><td>(HH:MM:SS)</td><td><input type='submit' value='SET'></td></form></tr>" ;
//    message += "<tr><form method=post action=" + server.uri() + "><td>Reboot Days</td><td align=center><input type='text' name='rbdw' value='"+String(mwde.ReBoot_wdays)+"' size=12></td><td>(min)</td><td><input type='submit' value='SET'></td></form></tr>" ;

    message += "<tr><form method=post action=" + server.uri() + "><td>Reboot Days<input type='hidden' name='rbdw' value='0'></td><td>" ;
    for (k = 0 ; k < 8 ; k++){
      switch( k ){
        case 0: MyNum = "S" ; break ;
        case 1: MyNum = "M" ; break ;
        case 2: MyNum = "T" ; break ;
        case 3: MyNum = "W" ; break ;
        case 4: MyNum = "T" ; break ;
        case 5: MyNum = "F" ; break ;
        case 6: MyNum = "S" ; break ;
        case 7: MyNum = "E" ; break ;
      }
           
      MyColor =  ""   ;  
      if ( ( mwde.ReBoot_wdays & (0x01 << k)) != 0 ){
        MyCheck = F("CHECKED")  ;    
        if ((k != 7 ) && ((mwde.ReBoot_wdays & 0x80 ) == 0  ) ){ // 
          MyColor =  F("bgcolor=red")   ;  
        }          
      }else{
        MyCheck = ""    ;      
      }
      message += MyNum + "<input type='checkbox' name='rbdx" + String(k) + "' " + String(MyCheck)+ ">";    
    }
    message += "</td><td></td><td><input type='submit' value='SET'></td></form></tr>" ;


    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", mwde.IPPing[0],mwde.IPPing[1],mwde.IPPing[2],mwde.IPPing[3]);
    message += "<tr><form method=post action=" + server.uri() + "><td>Ping Address</td><td align=center>" ; 
    message += "<input type='text' name='ipping' value='" + String(buff) + "' maxlength=16 size=12></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>";

    message += "<tr><form method=post action=" + server.uri() + "><td>MaxPing </td><td align=center><input type='text' name='pmax' value='"+String(mwde.PingMax)+"' size=12></td><td>(ms)</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>Ping Period </td><td align=center><input type='text' name='pfrq' value='"+String(mwde.PingFreq)+"' size=12></td><td>(min)</td><td><input type='submit' value='SET'></td></form></tr>" ;
    
    message += "<tr><form method=post action=" + server.uri() + "><td>Self Reboot Period </td><td align=center><input type='text' name='serb' value='"+String(mwde.SelfReBoot)+"' size=12></td><td>(min)</td><td><input type='submit' value='SET'></td></form></tr>" ;
    server.sendContent(message) ;
    message = "" ;
    
    message += "<tr><td colspan=4>.</td></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Port</td><td align=center><input type='text' name='smpo' value='"+String(mwde.SMTP_Port)+"' size=12></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Server</td><td align=center><input type='text' name='smse' value='"+String(mwde.SMTP_Server)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP User</td><td align=center><input type='text' name='smus' value='"+String(mwde.SMTP_User)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Password</td><td align=center><input type='text' name='smpa' value='"+String(mwde.SMTP_Password)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP FROM</td><td align=center><input type='text' name='smfr' value='"+String(mwde.SMTP_FROM)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP TO</td><td align=center><input type='text' name='smto' value='"+String(mwde.SMTP_TO)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP CC</td><td align=center><input type='text' name='smcc' value='"+String(mwde.SMTP_CC)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP BCC</td><td align=center><input type='text' name='smbc' value='"+String(mwde.SMTP_BCC)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Subject</td><td align=center><input type='text' name='smsj' value='"+String(mwde.SMTP_Subject)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Message</td><td align=center><input type='text' name='smmb' value='"+String(mwde.SMTP_Message)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;

    if ( ( mwde.SMTP_bSecure ) != 0 ){
      MyCheck = F("CHECKED")  ;    
    }else{
      MyCheck = F("")  ;    
    }
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Secure<input type='hidden' name='smbz' value='0'></td><td align=center><input type='checkbox' name='smbs' " + String(MyCheck)+ "></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    

    
    message += F("</table>");
    server.sendContent(message) ;
  }
  SendHTTPPageFooter();

}

