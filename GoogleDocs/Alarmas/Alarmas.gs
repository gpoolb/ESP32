function sendMessageToTelegram(){

  // Obtenga el token_id del bot de telegram
  var token_id = "YourBotTToken";
  // Obtenga el chat_id del grupo al que fué agregado el bot
  // utilice https://api.telegram.org/bot<YourBOTToken>/getUpdates
  var chat_id = "YourChatIdGroup";
  // Establezca la dirección de correo para enviar las alertas
  var mail = "my@email.com";
  // Escriba el mensaje que desea enviar al grupo de Telegram
  var message = "Falló la conexión con el sensor a las ";
  // Escriba el asunto que desea enviar por correo
  var asunto = "Fallo de conexión del sensor";
  // Establezca el lapso de tiempo entre los mensajes
  var lapsoDeTiempo = 1;  // Tiempo en minutos
  // Establezca el lapso de tiempo entre los email
  var lapsoDeTiempoEmail = 5;  // Tiempo en minutos

  d = new Date();
  var time = Utilities.formatDate(d, SpreadsheetApp.getActive().getSpreadsheetTimeZone(), "HH:mm:ss"); // Time
  
  var dataElapsedTime = verificaEnvioUltimoDato();

  if ((dataElapsedTime % lapsoDeTiempo) == 0 && (dataElapsedTime > lapsoDeTiempo)){
  // Make a GET request and log the returned content.
  var response = UrlFetchApp.fetch('https://api.telegram.org/bot' + token_id + '/sendMessage?chat_id=' + chat_id + '&text='+  message + time);
  //Logger.log(response.getContentText());
  
  try { 
    var jsonObj = JSON.parse(response.getContentText());
  } 
  catch(f){
    //Logger.log(f.message);
    return false;
  }

  if (jsonObj == undefined  || jsonObj == null) 
    return false;

  //Logger.log(jsonObj.result.text);
  //Logger.log(jsonObj.result.chat.id);

  if (jsonObj.result.text == message)
    return true;

  }

  if ((dataElapsedTime % lapsoDeTiempoEmail) == 0 && (dataElapsedTime > lapsoDeTiempo)){
    GmailApp.sendEmail(mail, asunto, message + time);
  }
  return false;
}

// Devuelve el tiempo transcurrido en minutos desde que se escribió el último dato
function verificaEnvioUltimoDato() {

  var ssGraphic = SpreadsheetApp.getActiveSpreadsheet();  // Abre la instancia del archivo que contiene el script actual
  var lastDataSheet = ssGraphic. getActiveSheet(); // Abre la instancia de la hoja 1 en la hoja de calculo del script actual ya que, ahí se guardan los históricos.

  // Se obtiene el valor de la última fila del archivo en la hoja de cálculo
  var row = lastDataSheet.getLastRow();
  // Se obtiene la hora del último dato enviado
  var timeLeido = lastDataSheet.getRange("C" + row).getValue();
  var dateLeido = lastDataSheet.getRange("B" + row).getValue();
  var d = new Date();
  
  var dateSplit = String(dateLeido).split("/");
  // Para usar la función new Date(), se requiere invertir el día y el mes, es decir, primero se declara el mes.
  var dateTimeLeido = new Date(dateSplit[1] + "/" + dateSplit[0] + "/" + dateSplit[2] + " " + timeLeido);
  var timeStampLeido = parseInt(dateTimeLeido.getTime()/1000);  // Se obtiene el tiempo leído en formato UNIX
                                                                // tiempo en segundos

  //var dateTimeActual = new Date(Utilities.formatDate(new Date(), "GMT-" + ((d.getTimezoneOffset()/60) + 1), " MM/dd/yyyy HH:mm:ss"));
  var dateTimeActual = new Date(Utilities.formatDate(new Date(), SpreadsheetApp.getActive().getSpreadsheetTimeZone(), " MM/dd/yyyy HH:mm:ss"));
/*
    Replace in save_data method in SimpleScript

    dataLoggerSheet.getRange("B" + row).setValue(Utilities.formatDate(new Date(), SpreadsheetApp.getActive().getSpreadsheetTimeZone(), "''dd/MM/yyyy")); // date
    dataLoggerSheet.getRange("C" + row).setValue(Utilities.formatDate(new Date(), SpreadsheetApp.getActive().getSpreadsheetTimeZone(), "''HH:mm:ss")); // Time

*/
  var timeStampActual = parseInt(dateTimeActual.getTime()/1000);  // Se obtiene el tiempo actual en formato UNIX
                                                                  // tiempo en segundos
 
  // Comparación entre tiempo transcurrido y tiempo actual
  var timeElapsed = parseInt((timeStampActual - timeStampLeido) / 60);

  //Logger.log("flag: " + flag);
  //Logger.log("row: " + row);
  //Logger.log("dateTimeLeido: " + dateTimeLeido);
  //Logger.log("dateTimeActual: " + dateTimeActual);
  
  return timeElapsed;
}
