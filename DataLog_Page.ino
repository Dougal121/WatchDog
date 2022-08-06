void datalog1_page(){
  int i , ii , iTmp , iX ;
  int j , k , kk ;
  String message ;  
  String MyNum ;  
  String MyColor ;
  String MyColor2 ;
  byte mac[6];
  time_t prev_time;
    
  SendHTTPHeader();

  server.sendContent(F("<br><center><b>Data Log</b><br>"));
  server.sendContent(F("<table border=1 title='Data Log'>"));
  server.sendContent("<tr><th>Date</th><th>Voltage</th><th>RSSI</th></tr>" ) ; 
  ii = (hour() * LOG_PER_HOUR) +  ( minute() / (60 / LOG_PER_HOUR ) )+1;
  prev_time = previousMidnight(now()) - (( MAX_LOG - ii ) * ( 60 / LOG_PER_HOUR ) * 60 );

  for ( i = 0 ; i < MAX_LOG ; i++ ) {
    j = (i + ii ) % MAX_LOG ;
    snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(prev_time), month(prev_time), day(prev_time) , hour(prev_time), minute(prev_time), second(prev_time));    
//    if ( !isnan(PowerLog[j].Volts) ){
      server.sendContent("<tr><td>" + String(buff) + "</td><td>" + String(PowerLog[j].Volts) + "</td><td>" + String(PowerLog[j].RSSI) + "</td></tr>" ) ; 
//    }
    prev_time += ((60/LOG_PER_HOUR) * 60 ) ;
  }
  server.sendContent(F("</table><br>"));    
  SendHTTPPageFooter();
}




void chart1_page(){
  int i , ii , iTmp , iX ;
  int j , k , kk ;
  String message ;  
  String MyNum ;  
  String MyColor ;
  String MyColor2 ;
  byte mac[6];
  time_t prev_time;
  
  SendHTTPHeader();

  if ( bSaveReq != 0 ){
    message += F("<blink>");      
  }   
  message += F("<a href='/?command=2'>Save Parameters to EEPROM</a><br>") ;     
  if ( bSaveReq != 0 ){
    message += F("</blink><font color='red'><b>Changes Have been made to settings.<br>Make sure you save if you want to keep them</b><br></font><br>") ;     
  }

  message += F("<center>\r\n<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>\r\n");  
  message += F("\r\n<script type=\"text/javascript\">\r\n");      
  message += F("google.charts.load('current', {'packages':['corechart']});\r\n");  // Load the Visualization API and the piechart package.    
  message += F("google.charts.setOnLoadCallback(drawChart);\r\n");                 // Set a callback to run when the Google Visualization API is loaded.
  message += F("function drawChart() {\r\n");
  message += F("var data = google.visualization.arrayToDataTable([[{label: 'Time', type: 'datetime'},{label: 'Battery Volatage', type: 'number'},{label: 'RSSI', type: 'number'}],\r\n");
  server.sendContent(message);
  message = "" ;

  ii = (hour() * LOG_PER_HOUR) +  ( minute() / (60 / LOG_PER_HOUR ) )+1;
  prev_time = previousMidnight(now()) - (( MAX_LOG - ii ) * (60 / LOG_PER_HOUR ) * 60 );
  
  for ( i = 0 ; i < MAX_LOG ; i++ ) {
    j = (i + ii ) % MAX_LOG ;
    snprintf(buff, BUFF_MAX, "new Date(\'%4d-%02d-%02dT%02d:%02d:%02d\')", year(prev_time), month(prev_time), day(prev_time) , hour(prev_time), minute(prev_time), second(prev_time));    
    if ( !isnan(PowerLog[j].Volts)){
      message += "[ " + String(buff) + "," + String(PowerLog[j].Volts) + "," + String(PowerLog[j].RSSI) + " ] ,\r\n"  ; 
    }
    prev_time += (( 60 / LOG_PER_HOUR ) * 60 );
    if (( i % 10) == 9 ){
      server.sendContent(message);
      message = "" ;      
    }
  }
  message += F("]);\r\n");
  server.sendContent(message);
  message = "" ;
     
  message += F("var options = {title: 'System Logs 5 min intervals for last 24 Hours' , series: {0: {targetAxisIndex: 0},1: {targetAxisIndex: 1}}, vAxes: { 0: {title: 'Battery Volts', viewWindow:{ max: 20, min: 0}}, 1: {title: 'RSSI', viewWindow:{ max: 0, min: -120}}} , height: 700 , opacity:100 , interpolateNulls:true , colors: ['Blue','Red'], backgroundColor: '#FFFFFF', ");  // Set chart options
  message += F("  };\r\n");

  message += F("var chart = new google.visualization.LineChart(document.getElementById('linechart'));\r\n");
  message += F("chart.draw(data, options); } </script>\r\n");
  message += F("<div id='linechart'></div><br>\r\n");                                          //  style='width:1000; height:800'

  server.sendContent(message);
  message = "" ;
  
  SendHTTPPageFooter();
}

void clearPoweLog(){
  int i , ii , iTmp , iX ;
  for ( i = 0 ; i < MAX_LOG ; i++ ) {
    PowerLog[i].Volts = 0 ;
    PowerLog[i].RSSI = 0 ;
  }  
}

