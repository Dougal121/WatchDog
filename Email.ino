void SendEmailToClient(){

  if (( String(mwde.SMTP_FROM).length() >5 )&&( String(mwde.SMTP_TO).length() >5 ) && ( mwde.SMTP_Port > 0 )){
    snprintf(buff, BUFF_MAX, " %d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());

    if ( String(mwde.SMTP_FROM).length() < 6 ){
      snprintf(buff, BUFF_MAX, "Just power cycled device attached to node %08X" ,ESP.getChipId());
    }else{
      snprintf(buff, BUFF_MAX, "%s" , mwde.SMTP_FROM ) ;
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
  
    if ( String(mwde.SMTP_Message).length() > 5 ){
      snprintf(buff, BUFF_MAX, "%s \r\n %d/%02d/%02d %02d:%02d:%02d \r\nNode ID %08X\r\nIP %03u.%03u.%03u.%03u\r\n\r\n",mwde.SMTP_Message, year(), month(), day() , hour(), minute(), second(),ESP.getChipId(), MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
    }else{
      snprintf(buff, BUFF_MAX, "Just power cycled device \r\n %d/%02d/%02d %02d:%02d:%02d \r\nNode ID %08X\r\nIP %03u.%03u.%03u.%03u\r\n\r\n", year(), month(), day() , hour(), minute(), second(),ESP.getChipId(), MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
    }
    WDmail.setBody(buff);
    WDmail.enableDebugMode();
    if (WDmail.send(mwde.SMTP_Server , mwde.SMTP_Port , mwde.SMTP_User , mwde.SMTP_Password ) == 0){
      Serial.println("Mail send OK");
    }
  }else{
    Serial.println("Mail not set up proper like dude...");
  }
}

