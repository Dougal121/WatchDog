#define MESSAGE_MAX 1024 



void SendEmailToClient(int iMessage){
char csTemp[MESSAGE_MAX] ;
int i ;
String strMyTmp ;
IPAddress MyIP;
bool bRoach = false ;

  if (( String(mwde.SMTP_FROM).length() >5 )&&( String(mwde.SMTP_TO).length() >5 ) && ( mwde.SMTP_Port > 0 )){
    snprintf(buff, BUFF_MAX, " %d/%02d/%02d %02d:%02d:%02d\0", year(), month(), day() , hour(), minute(), second());
    snprintf(csTemp,MESSAGE_MAX,"\0"  )  ;  

    if (iMessage == -1 ) {
        snprintf(buff, BUFF_MAX, "Testing Email on node %08X\0" ,ESP.getChipId());      
    }else{
      if ( String(mwde.SMTP_FROM).length() < 6 ){
        snprintf(buff, BUFF_MAX, "Just power cycled device attached to node %08X\0" ,ESP.getChipId());
      }else{
        snprintf(buff, BUFF_MAX, "%s\0" , mwde.SMTP_Subject ) ;
      }
    }
    
    if ( String(mwde.SMTP_FROM).length() < 6 ){
      WDmail.setSubject("doby@house.elf", buff);      
    }else{
      WDmail.setSubject( mwde.SMTP_FROM , buff );
    }
    WDmail.addTo( mwde.SMTP_TO );
    if ( String(mwde.SMTP_CC).length() > 5 ){
      WDmail.addCC(mwde.SMTP_CC);
    }
    if ( String(mwde.SMTP_BCC).length() > 5 ){
      WDmail.addBCC(mwde.SMTP_BCC );
    }

    switch (iMessage){
      case -1:
        snprintf(buff, BUFF_MAX, "Test Email sent at %d/%02d/%02d %02d:%02d:%02d \r\nNode ID %08X\r\nIP %03u.%03u.%03u.%03u\r\n\r\n\0", year(), month(), day() , hour(), minute(), second(),ESP.getChipId(), MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
      break;
      case 5:  // ADC value greater than Alarm 1
          snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) greater then Alarm 1 \0",ghks.NodeName,mwdm.ADC_Value, ghks.ADC_Unit )  ;  
      break ;
      case 6:  // ADC value greater than Alarm 2
          snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) greater then Alarm 2 \0",ghks.NodeName,mwdm.ADC_Value,ghks.ADC_Unit )  ;  
      break ;
      case 7:  // ADC value less than Alarm 1
          snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) less then Alarm 1 \0",ghks.NodeName,mwdm.ADC_Value,ghks.ADC_Unit  )  ;  
      break ;
      case 8:  // ADC value less than Alarm 2
          snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) less then Alarm 2 \0",ghks.NodeName,mwdm.ADC_Value,ghks.ADC_Unit  )  ;  
      break ;
      default:
          if ( String(mwde.SMTP_Message).length() > 5 ){
            snprintf(buff, BUFF_MAX, "%s \r\n %d/%02d/%02d %02d:%02d:%02d \r\nNode ID %08X\r\nIP %03u.%03u.%03u.%03u\r\n\r\n\0",mwde.SMTP_Message, year(), month(), day() , hour(), minute(), second(),ESP.getChipId(), MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
          }else{
            snprintf(buff, BUFF_MAX, "Just power cycled device \r\n %d/%02d/%02d %02d:%02d:%02d \r\nNode ID %08X\r\nIP %03u.%03u.%03u.%03u\r\n\r\n\0", year(), month(), day() , hour(), minute(), second(),ESP.getChipId(), MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
          }
      break;
    }

    snprintf(buff, BUFF_MAX, "\r\n %d/%02d/%02d %02d:%02d:%02d \r\n\0", year(), month(), day() , hour(), minute(), second());
    strcat(csTemp,buff) ;
    snprintf(buff, BUFF_MAX,"Node ID %08X\r\nIP %03u.%03u.%03u.%03u \r\n\0",ESP.getChipId(), MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
    strcat(csTemp,buff) ;
    snprintf(buff, BUFF_MAX, "CPU Uptime %d:%02d:%02d (Days:Hrs:Min)\r\n\r\n\0",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));
    strcat(csTemp,buff) ;
          
    WDmail.setBody(csTemp);
    WDmail.enableDebugMode();
    if ( mwde.SMTP_User[0] == 0 ){
      if (WDmail.send(mwde.SMTP_Server , mwde.SMTP_Port , NULL , NULL ) == 0){
        Serial.println("Mail send OK");
      }      
    }else{
      if (WDmail.send(mwde.SMTP_Server , mwde.SMTP_Port , mwde.SMTP_User , mwde.SMTP_Password ) == 0){
        Serial.println("Mail send OK");
      }      
    }
  }else{
    Serial.println("Mail not set up proper like dude...");
  }
}

void DisplayEmailSetup() {
  long  i = 0 ;
  int ii  ;
  int j ;
  int k , kk  ;
  String message ;
  String MyCheck ;
  SendHTTPHeader();   //  ################### START OF THE RESPONSE  ######
  
  for (uint8_t j=0; j<server.args(); j++){
    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 11:  // load values
          bSendTestEmail = true ;
        break;
        case 121:
          ResetSMTPInfo();
        break;          
      }
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
    i = String(server.argName(j)).indexOf("smpo");
    if (i != -1){  // 
      mwde.SMTP_Port = String(server.arg(j)).toInt() ;
      mwde.SMTP_Port = constrain(mwde.SMTP_Port,1,65535);
    }
    i = String(server.argName(j)).indexOf("smue");
    if (i != -1){  //  
      if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
        mwde.bUseEmail = true ;        
      }
    }     
    i = String(server.argName(j)).indexOf("smup");
    if (i != -1){  //  
      if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
        mwde.bSPARE = true ;        
      }
    }     

    i = String(server.argName(j)).indexOf("allu");
    if (i != -1){  
     String(server.arg(j)).toCharArray(ghks.ADC_Unit , sizeof(ghks.ADC_Unit)) ;
    }    
    i = String(server.argName(j)).indexOf("all1");
    if (i != -1){ 
      ghks.ADC_Alarm1 = String(server.arg(j)).toFloat() ;
    }          
    i = String(server.argName(j)).indexOf("all2");
    if (i != -1){ 
      ghks.ADC_Alarm2 = String(server.arg(j)).toFloat() ;
    }      
    i = String(server.argName(j)).indexOf("adcm");
    if (i != -1){ 
      ghks.ADC_Cal_Mul = String(server.arg(j)).toFloat() ;
    }      
    i = String(server.argName(j)).indexOf("adco");
    if (i != -1){ 
      ghks.ADC_Cal_Ofs = String(server.arg(j)).toFloat() ;
    }      
    i = String(server.argName(j)).indexOf("aldl");
    if (i != -1){ 
      ghks.ADC_Alarm_Delay = String(server.arg(j)).toInt() ;
    }      
    
    i = String(server.argName(j)).indexOf("almmc");
    if (i != -1){ 
      if ( String(server.arg(j)).toInt() == 0x00 ){
        ghks.ADC_Alarm_Mode &=  0x7f ;      
      }else{
        ghks.ADC_Alarm_Mode |=  0x80 ;              
      }
    }  

    i = String(server.argName(j)).indexOf("alme2");
    if (i != -1){ 
      switch ( String(server.arg(j)).toInt()  ){
        case 0:
          ghks.ADC_Alarm_Mode &=  0xcf ;      
        break;
        case 1:
          ghks.ADC_Alarm_Mode &=  0xcf ;      
          ghks.ADC_Alarm_Mode |=  0x10 ;      
        break;
        case 2:
          ghks.ADC_Alarm_Mode &=  0xcf ;      
          ghks.ADC_Alarm_Mode |=  0x20 ;      
        break;
        case 3:
          ghks.ADC_Alarm_Mode |=  0x30 ;      
        break;
      }  
    }

    i = String(server.argName(j)).indexOf("alme1");
    if (i != -1){ 
      switch ( String(server.arg(j)).toInt()  ){
        case 0:
          ghks.ADC_Alarm_Mode &=  0xf9 ;      
        break;
        case 1:
          ghks.ADC_Alarm_Mode &=  0xf9 ;      
          ghks.ADC_Alarm_Mode |=  0x02 ;      
        break;
        case 2:
          ghks.ADC_Alarm_Mode &=  0xf9 ;      
          ghks.ADC_Alarm_Mode |=  0x04 ;      
        break;
        case 3:
          ghks.ADC_Alarm_Mode |=  0x06 ;      
        break;
      }  
    }

    i = String(server.argName(j)).indexOf("alma2");
    if (i != -1){ 
      if ( String(server.arg(j)).toInt() == 0x00 ){
        ghks.ADC_Alarm_Mode &=  0xf7 ;      
      }else{
        ghks.ADC_Alarm_Mode |=  0x08 ;              
      }
    }  
    i = String(server.argName(j)).indexOf("alma1");
    if (i != -1){ 
      if ( String(server.arg(j)).toInt() == 0x00 ){
        ghks.ADC_Alarm_Mode &=  0xfe ;      
      }else{
        ghks.ADC_Alarm_Mode |=  0x01 ;              
      }
    }  

    
  }


    message = "";
    message = F("<table border=1 title='Email setup for WatchDog'>") ;
    message += F("<tr><th>Email Parameter</th><th>Value</th><th>Units</th><th>.</th></tr>") ;              
    message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Port</td><td align=center title='Popular Values 25 , 465 , 2525 , 587'><input type='text' name='smpo' value='"+String(mwde.SMTP_Port)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
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
    message += "<tr><td colspan=4>.</td></tr>" ;
    server.sendContent(message) ;
    message = "<tr><td>Last Email Status</td><td>"+String(lRet_Email)+"</td><td colspan=2>.</td></tr>" ;

    message += "<tr></td><td colspan=4>.</td></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>ADC Multiplier</td><td align=center><input title='The number of S.I. units per input Volt x 3.2' type='text' name='adcm' value='"+String(ghks.ADC_Cal_Mul)+"' size=30></td><td></td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>ADC Offset</td><td align=center><input title='The easy way to do this is use the zero cal button below' type='text' name='adco' value='"+String(ghks.ADC_Cal_Ofs)+"' size=30></td><td></td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>ADC SI Units</td><td align=center><input type='text' name='allu' value='"+String(ghks.ADC_Unit)+"' size=30 maxlength=5></td><td></td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td>ADC RAW <b> "+String(mwdm.ADC_Raw)+" </td><td>ADC Scaled Value <b> "+String(mwdm.ADC_Value,1)+" </td><td colspan=2><input type='hidden' name='adco' value='"+String(-1.0*mwdm.ADC_Raw)+"'><input title='Setup the transducer for Zero physical input quantity then press this button to calibrate the zero point on computer' type='submit' value='DO ZERO CAL'></td></form></tr>" ;
  
    message += "<tr><form method=post action=" + server.uri() + "><td>Master Alarm Enable</td><td> <select name='almmc'>" ;
    switch ( ghks.ADC_Alarm_Mode & 0x80 ){
      case 0:   
        message += F("<option value='0' SELECTED>Disabled") ;
        message += F("<option value='1' >Enabled") ;
      break;
      case 128:   
        message += F("<option value='0'>Disabled") ;
        message += F("<option value='1' SELECTED>Enabled") ;
      break;
    }
    message += "</td><td></td><td><input type='submit' value='SET'></td></form></tr>" ;
      
    message += "<tr><form method=post action=" + server.uri() + "><td colspan=3>Alarm 1 <select name='alme1'>" ;
    for ( j = 0 ; j < 4 ; j++ ){ 
      if ((( ghks.ADC_Alarm_Mode & 0x06 )>>1 ) == j ){
        MyCheck = "SELECTED" ;
      }else{
        MyCheck = "" ;
      }
      message += "<option value='"+String(j)+"' " + MyCheck +">" ;
      switch ( j ){
        case 0: message += F("Disabled") ;  break ;
//        case 1: message += F("On Master Enabled") ; break ;
//        case 2: message += F("Off Master Enabled") ; break ;
        case 3: message += F("Always Enabled") ; break ;
      }
    }
    message += "</select>" ;
    message += " Active State <select name='alma1'>" ;
    switch ( ghks.ADC_Alarm_Mode & 0x01 ){
      case 0:   
        message += F("<option value='0' SELECTED>Less Than") ;
        message += F("<option value='1' >Greater Than") ;
      break;
      case 1:   
        message += F("<option value='0'>Less Than") ;
        message += F("<option value='1' SELECTED>Greater Than") ;
      break;
    }
    message += "</select> <input type='text' name='all1' value='"+String(ghks.ADC_Alarm1)+"' size=6> ("+String(ghks.ADC_Unit)+")</td><td><input type='submit' value='SET'></td></form></tr>" ;
    
    message += "<tr><form method=post action=" + server.uri() + "><td colspan=3>Alarm 2 <select name='alme2'>" ;
    for ( j = 0 ; j < 4 ; j++ ){ 
      if ((( ghks.ADC_Alarm_Mode & 0x30 )>>4 ) == j ){
        MyCheck = "SELECTED" ;
      }else{
        MyCheck = "" ;
      }
      message += "<option value='"+String(j)+"' " + MyCheck +">" ;
      switch ( j ){
        case 0: message += F("Disabled") ;  break ;
//        case 1: message += F("On Master Enabled") ; break ;
//       case 2: message += F("Off Master Enabled") ; break ;
        case 3: message += F("Always Enabled") ; break ;
      }
    }
    message += "</select>" ;
    message += " Active State <select name='alma2'>" ;
    switch ( ghks.ADC_Alarm_Mode & 0x08 ){
      case 0:   
        message += F("<option value='0' SELECTED>Less Than") ;
        message += F("<option value='1' >Greater Than") ;
      break;
      case 8:   
        message += F("<option value='0'>Less Than") ;
        message += F("<option value='1' SELECTED>Greater Than") ;
      break;
    }
    message += "</select> <input type='text' name='all2' value='"+String(ghks.ADC_Alarm2)+"' size=6> ("+String(ghks.ADC_Unit)+")</td><td><input type='submit' value='SET'></td></form></tr>" ;
    
    message += "<tr><form method=post action=" + server.uri() + "><td>Alarm Delay</td><td align=center><input type='text' name='aldl' value='"+String(ghks.ADC_Alarm_Delay)+"' size=30></td><td>(s)</td><td><input type='submit' value='SET'></td></form></tr>" ;
    if ( ( mwde.bSPARE ) != 0 ){
      MyCheck = F("CHECKED")  ;    
    }else{
      MyCheck = F("")  ;    
    }
    message += "<tr><form method=post action=" + server.uri() + "><td title='SPAREemails'><input type='hidden' name='smuy' value='0'>SPARE Emails</td><td align=center><input type='checkbox' name='smup' " + String(MyCheck)+ "></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    if ( ( mwde.bUseEmail ) != 0 ){
      MyCheck = F("CHECKED")  ;    
    }else{
      MyCheck = F("")  ;    
    }
    message += "<tr><form method=post action=" + server.uri() + "><td><input type='hidden' name='smuz' value='0'>Use Email Alarm Messaging</td><td align=center><input type='checkbox' name='smue' " + String(MyCheck)+ "></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
    message += "<tr><form method=post action=" + server.uri() + "><td colspan=4 align='center'><input type='hidden' name='command' value='11'><input type='submit' value='### SEND TEST EMAIL ###'></td></form></tr>" ;
    message += "<tr><td colspan=4 align='center'><form method=post action=" + server.uri() + "><input type='hidden' name='command' value='121'><input type='submit' value='Clear / RESET EMAIL Settings'></form></td></tr>" ; 
    message += F("<table>"); 
    server.sendContent(message) ;
  
    SendHTTPPageFooter();
}


void ResetSMTPInfo(){
  mwde.SMTP_Port = 25 ;                // 25 , 465 , 2525 , 587
//  sprintf(SMTP.server,"203.36.137.241\0") ;   // smtp.telstrabusiness.com 
//  sprintf(SMTP.server,"121.200.0.25\0") ;   // mail.aussiebroadband.com.au
  sprintf(mwde.SMTP_Server,"203.0.178.192\0") ;   // mail.iinet.net.au
  sprintf(mwde.SMTP_User,"westernmurray@iinet.net.au\0") ;
  sprintf(mwde.SMTP_Password,"\0") ;
  sprintf(mwde.SMTP_FROM,"westernmurray@iinet.net.au\0") ;
  sprintf(mwde.SMTP_TO,"plummer@westernmurray.com.au\0") ;
  sprintf(mwde.SMTP_TO,"brandon@westernmurray.com.au\0") ;
  sprintf(mwde.SMTP_CC,"\0") ;
  sprintf(mwde.SMTP_BCC,"\0") ;
  sprintf(mwde.SMTP_Message,"Test Email\0") ;
  sprintf(mwde.SMTP_Subject,"Node %08X\0",ESP.getChipId()) ;
  mwde.SMTP_bSecure = false ;
  mwde.bUseEmail = false ;
 
}


