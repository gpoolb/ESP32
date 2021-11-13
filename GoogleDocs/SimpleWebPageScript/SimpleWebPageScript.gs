
/*
Este script tiene el propósito de recibir la información que envía el cliente (sensor remoto)
y almacenarla en un archivo que contiene éste script. Adicionalmente, el script incluye 
una página web que muestra el último dato enviado por el sensor remoto (cliente). Mediante 
el ítem "tag" se elige la opción que desea gestionar en éste script.
*/

function doGet(e) {

  var tag = "",
    temp = "",
    hum = "",
    d = "";

  try {
    // Se extrae el valor de ítem "tag"
    tag = e.parameters.tag;
    
    // Se envía los archivos adjuntos al proyecto
    if (tag == null || tag == undefined){
      var path;
      if (e.pathInfo == null) path = "Index";
      return HtmlService.createTemplateFromFile(path).evaluate();
    }
    
    // Se guardan los datos en el archivo actual
    if (tag == "Update"){
      temp = e.parameters.Temp;
      hum = e.parameters.Hum;
    // save the data to spreadsheet
    save_data(temp, hum);
    }

    // Se envia el response con los datos para mostrar en la página web
    if (tag == "temperature"){
      d = new Date();
      var xmlContent = "<RESPONSE>\n"
        + read_data()
        + "<TIME>" + Utilities.formatDate(new Date(), "GMT-" + ((d.getTimezoneOffset()/60) + 1), "dd/MM/yyyy HH:mm:ss") +"</TIME>\n"
        xmlContent += "</RESPONSE>\n";
        return ContentService.createTextOutput(xmlContent).setMimeType(ContentService.MimeType.XML);
    }


    // Si nada de lo anterior se cumple, se envía los parametros recibidos
    return ContentService.createTextOutput("Wrote GET Method:\n  parametros: " + JSON.stringify(e) );

  } catch (error) {  // Se envia en caso de un error en el procesamiento anterior.
    Logger.log(error);
    return ContentService.createTextOutput("oops...." + error.message
      + "\n" + new Date()
      + "\nParametros: " +JSON.stringify(e));
  }
}

function doPost(e) {  // Esta función no está implementada
                      // sólo envía como respuesta 
                      // los parámetros que el cliente envió.
  
  try { 
    return ContentService.createTextOutput("Wrote POST Method:\n  parametros: " + JSON.stringify(e));
  } 
  catch(f){
    return ContentService.createTextOutput("Error in parsing request body: " + f.message);
  } 
  
}

// Este método guarda los datos enviados por el sensor en el actual archivo
function save_data(temp, hum) {
  var d;

//  Logger.log("--- save_data ---");
  try {

    var ssLogging = SpreadsheetApp.getActiveSpreadsheet();  // Abre la instancia del archivo actual para guardar los históricos
    var dataLoggerSheet = ssLogging. getActiveSheet(); // Abre la instancia de la hoja 1 en la hoja de calculo para guardar los históricos

    // Se obtiene el valor de la última fila del archivo en la hoja de cálculo que contiene los históricos
    var row = dataLoggerSheet.getLastRow() + 1;

    // Si el valor de la fila es menor que 3, indica que el archivo está recién creado, por lo que
    // se requiere crear el encabezado del archivo que contiene el histórico, crear el encabezado y
    // borrar el contenido (si lo hubiera) de la hoja de cálculo que contiene éste script.
    if (row < 3) {

      // Si el archivo está recién creado se bloquea el acceso a éste método para evitar
      // que este metodo se ejecute varias veces al mismo tiempo (Multitarea)
      var lock = LockService.getScriptLock();
      try {
        lock.waitLock(3000); // wait 3 seconds for others' use of the code section and lock to stop and then proceed
      } catch (e) {
        Logger.log('Could not obtain lock after 30 seconds.');
        return HtmlService.createHtmlOutput("<b> Server Busy please try after some time <p>")
      }

      row = 1;
      // Se crea el encabezado del archivo que lleva el histórico de las lecturas
      dataLoggerSheet.getRange("A" + row).setValue("ID"); // ID
      dataLoggerSheet.getRange("B" + row).setValue("Fecha\n (DD/MM/YYYY)"); // date
      dataLoggerSheet.getRange("C" + row).setValue("Hora"); // Time
      dataLoggerSheet.getRange("D" + row).setValue("Temperatura"); // value
      dataLoggerSheet.getRange("E" + row).setValue("Humedad"); // value

      // Se libera este método para que otros usuarios tengan acceso
      lock.releaseLock();

      row = 3;

    }

    // Se guardan los datos en el archivo que contiene el histórico de lecturas
    d = new Date();
    dataLoggerSheet.getRange("A" + row).setValue(row - 2); // ID
    dataLoggerSheet.getRange("B" + row).setValue(Utilities.formatDate(new Date(), "GMT-" + ((d.getTimezoneOffset()/60) + 1), "''dd/MM/yyyy")); // date
    dataLoggerSheet.getRange("C" + row).setValue(Utilities.formatDate(new Date(), "GMT-" + ((d.getTimezoneOffset()/60) + 1), "''HH:mm:ss")); // Time
    dataLoggerSheet.getRange("D" + row).setValue(temp); // temp
    dataLoggerSheet.getRange("E" + row).setValue(hum); // hum    

    SpreadsheetApp.flush();
	
  }
  catch (error) {
    Logger.log("Falló al guardar documento...");
  }

//  Logger.log("--- save_data end---");
}

// Metodo que lee los datos en el archivo que este script y,
// devuelve en formato xml el response de la última fila de datos.
function read_data() {

//  Logger.log("--- read_data ---");
  try {
    var ssLogging = SpreadsheetApp.getActiveSpreadsheet();  // Abre la instancia del archivo actual para guardar los históricos
    var dataLoggerSheet = ssLogging. getActiveSheet(); // Abre la instancia de la hoja 1 en la hoja de calculo para leer los históricos
    var row;
    var xmlContent = "\n";

    // Se obtiene el valor de la última fila del archivo en la hoja de cálculo que contiene los históricos
    row = dataLoggerSheet.getLastRow();
    
    // Si el valor de la fila es menor a 3, indica que el archivo es nuevo
    // y no contiene datos.
    if (row < 3)
      return xmlContent;

    // Se obtiene la hora del último dato enviado
    var value = dataLoggerSheet.getRange("B" + row).getValue();
    if(value != "")
      xmlContent = "<LASTTIME>" + value;

    // Se obtiene la fecha del último dato enviado
    value = dataLoggerSheet.getRange("C" + row).getValue();
    if (value != "")
      xmlContent += " " + value +"</LASTTIME>\n";

    // Se obtiene el valor de temperatura del último dato enviado
    value = dataLoggerSheet.getRange("D" + row).getValue();
    if (value != "") 
      xmlContent += "<TEMP>" + value +"</TEMP>\n";
    
    // Se obtiene el valor de la humedad del último dato enviado
    value = dataLoggerSheet.getRange("E" + row).getValue();
    if (value != "")
      xmlContent += "<HUM>" + value +"</HUM>\n";
    
  }

  catch (error) {
    Logger.log("Falló al leer documento...");
  }

//  Logger.log("--- read_data end---");
  // Se envia una sección del response al método solicitado
  return xmlContent;
}
/* Esta función se encarga de anexar las dependencias (archivos JS y SVG) de la página web
   para que dicha página web funcione correctamente. Google bloquea las dependencias que sean
   guardadas en el Drive u otro sitio (CORS). Los archivos que esta función envia son 
   Logo y JavaScript. Los archivos que Google no bloquea son los JSON, XML, HTML y texto plano.
*/
function include(filename) {
  return HtmlService.createHtmlOutputFromFile(filename).getContent();
}
