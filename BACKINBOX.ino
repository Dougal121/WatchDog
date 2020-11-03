void BackInTheBoxMemory(){
  uint8_t i , j ;

  sprintf(ghks.nssid,"************\0");  // put your default credentials in here if you wish
  sprintf(ghks.npassword,"********\0");  // put your default credentials in here if you wish
  sprintf(ghks.NodeName,"Clever Dog Milo\0") ;
  


  sprintf(ghks.cpassword,"\0");
  
  ghks.fTimeZone = 10.0 ;
  ghks.lNodeAddress = ESP.getChipId() & 0xff ;
  sprintf(ghks.timeServer ,"au.pool.ntp.org\0"); 
  ghks.AutoOff_t = 0 ;
  ghks.localPortCtrl = 8088 ;
  ghks.RemotePortCtrl= 8088 ;
  ghks.lVersion = MYVER ;
  
/*  ghks.RCIP[0] = 192 ;
  ghks.RCIP[1] = 168 ; 
  ghks.RCIP[2] = 2 ;
  ghks.RCIP[3] = 255 ;*/
  sprintf(ghks.RCIP ,"192.168.2.255\0"); 
  
  ghks.lNetworkOptions = 0 ;     // DHCP 
  ghks.IPStatic[0] = 192 ;
  ghks.IPStatic[1] = 168 ;
  ghks.IPStatic[2] = 0 ;
  ghks.IPStatic[3] = 123 ;

  ghks.IPGateway[0] = 192 ;
  ghks.IPGateway[1] = 168 ;
  ghks.IPGateway[2] = 0 ;
  ghks.IPGateway[3] = 1 ;

  ghks.IPDNS = ghks.IPGateway ;

  ghks.IPMask[0] = 255 ;
  ghks.IPMask[1] = 255 ;
  ghks.IPMask[2] = 255 ;
  ghks.IPMask[3] = 0 ;

  mwde.RelayPin = 4 ;  // This is the relay
  mwde.ActiveValue = 1 ; // high
  mwde.ReBootOffTime = 20 ; // sec
  mwde.RebootInterval = 1440 ; // min
  mwde.ReBootOption = 0 ; 
  mwde.MinRecycleTime = 5 ; // min
  mwde.PingMax = 1500 ;  //ms
  mwde.PingFreq = 30 ; // min
  mwde.SelfReBoot = 10080 ; // self reboot every week

  mwde.ReBoot_sec = 0 ;
  mwde.ReBoot_min = 0 ;
  mwde.ReBoot_hour = 6 ;
  mwde.ReBoot_wdays = 255 ;
  
  mwde.IPPing[0] = 0 ;
  mwde.IPPing[1] = 0 ;
  mwde.IPPing[2] = 0 ;
  mwde.IPPing[3] = 0 ;

  mwde.SMTP_Port = 587 ;                // 25 , 465 , 2525 , 587
  sprintf(mwde.SMTP_Server,"\0") ;
  sprintf(mwde.SMTP_User,"\0") ;
  sprintf(mwde.SMTP_Password,"\0") ;
  sprintf(mwde.SMTP_TO,"\0") ;
  sprintf(mwde.SMTP_CC,"\0") ;
  sprintf(mwde.SMTP_BCC,"\0") ;
  sprintf(mwde.SMTP_Message,"\0") ;
  sprintf(mwde.SMTP_Subject,"Node %08X\0",ESP.getChipId()) ;
  mwde.SMTP_bSecure = true ;
  
}


void LoadParamsFromEEPROM(bool bLoad){
long lTmp ;  
int i ;
int j ;
int bofs ,ofs ;
int eeAddress ;

  if ( bLoad ) {
    EEPROM.get(0,ghks);
    eeAddress = sizeof(ghks) ;
    Serial.println("read - ghks structure size " +String(eeAddress));   

    ghks.lNodeAddress = constrain(ghks.lNodeAddress,0,32768);
    ghks.fTimeZone = constrain(ghks.fTimeZone,-12,12);
    ghks.localPort = constrain(ghks.localPort,1,65535);
    ghks.localPortCtrl = constrain(ghks.localPortCtrl,1,65535);
    ghks.RemotePortCtrl = constrain(ghks.RemotePortCtrl,1,65535);
    if ( year(ghks.AutoOff_t) < 2000 ){
       ghks.AutoOff_t = now();
    }

    ghks.lDisplayOptions = constrain(ghks.lDisplayOptions,0,1);

    eeAddress = PROG_BASE ;   // move us up so ghks can have wiggle room in future versions
    EEPROM.get(eeAddress,mwde);
    eeAddress += sizeof(mwde) ;
    
    Serial.println("Final VPFF EEPROM adress " +String(eeAddress));   
    
  }else{
    ghks.lVersion  = MYVER ;
    EEPROM.put(0,ghks);
    eeAddress = sizeof(ghks) ;
    Serial.println("write - ghks structure size " +String(eeAddress));   

    eeAddress = PROG_BASE ; // move us up so ghks can have wiggle room in future versions
    EEPROM.put(eeAddress,mwde);
    eeAddress += sizeof(mwde) ;
    
    Serial.println("Final EEPROM Save adress " +String(eeAddress));   

    EEPROM.commit();                                                       // save changes in one go ???
    bSaveReq = 0 ;
  }
}
