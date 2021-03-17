/* Created by Gabriel Pool, 2021

 Read/Write to Google Sheets using REST API.
 Can be used with ESP8266 & other embedded IoT devices.
 
 Use this file with the ESP8266 library HTTPSRedirect
 
 doGet() and doPost() need the spreadsheet ID. Cannot use "active spreadsheet" here since
 the device can operate without the spreadsheet even being open.
 http://stackoverflow.com/questions/4024271/rest-api-best-practices-where-to-put-parameters
 http://trevorfox.com/2015/03/rest-api-with-google-apps-script

 Similar API docs:
 https://gspread.readthedocs.org/en/latest/
 https://smartsheet-platform.github.io/api-docs/#versioning-and-changes
 http://search.cpan.org/~jmcnamara/Excel-Writer-XLSX/lib/Excel/Writer/XLSX.pm

 http://forum.espruino.com/conversations/269510/
 http://stackoverflow.com/questions/34691425/difference-between-getvalue-and-getdisplayvalue-on-google-app-script
 http://ramblings.mcpher.com/Home/excelquirks/gooscript/optimize

 Things to remember with getValue() object format:
 1. Partial dates or times-only will be replaced with a full date + time, probably in the
    year 1989. Like this: Sat Dec 30 1899 08:09:00 GMT-0500 (EST)
 2. Dollar ($) currency symbol will be absent if cell contains currency.
    This may be locale-dependent.
 3. Scientific notation will be replaced by decimal numbers like this: 0.0000055

 Script examples
 https://developers.google.com/adwords/scripts/docs/examples/spreadsheetapp
*/
/*
Este script tiene el propósito de recibir la información que envía el cliente (sensor remoto)
y almacenarla en un archivo que tiene por nombre el mes actual + los cuatro dígitos
del año en curso, ubicada en la carpeta llamada "Logging". Adicionalmente, el script incluye 
una página web que muestra el último dato enviado por el sensor remoto (cliente), un control 
que selecciona el archivo del mes que se desea descargar y una gráfica que muestra los 24 datos 
recibidos a cada hora durante un día. Mediante el ítem "tag" se elige la opción que desea 
gestionar en éste script.
*/

function doGet(e) {

  var tag = "",
    temp = "",
    hum = "",
    d = "";
    var i = new Date().getMonth();

  try {
    // Se extrae el valor de ítem "tag"
    tag = e.parameters.tag;
    
    // Se envía los archivos adjuntos al proyecto
    if (tag == null || tag == undefined){
      var path;
      if (e.pathInfo == null) path = "Index";
      return HtmlService.createTemplateFromFile(path).evaluate();
    }
    
    // Se guardan los datos en la carpeta Logging
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

    // Se envia el response con los datos para mostrar en la gráfica
    if (tag == "graphicDataValues"){

      return ContentService.createTextOutput(loadGraphicDataValues()).setMimeType(ContentService.MimeType.XML);
    }

    // Se envian los datos habilitar / deshabilitar opciones de descarga 
    // de archivos que contiene el histórico de las lecturas almacenadas.
    if (tag == "optionValues"){

      return ContentService.createTextOutput(loadOptionValues()).setMimeType(ContentService.MimeType.XML);
    }

    // Se envía el archivo que contiene el histórico
    // del mes solicitado
    if (tag == "download"){
      nameFile = e.parameters.name;

      return downloadFile(nameFile); // and Hyde 
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

// Este método guarda los datos enviados por el sensor en el 
// archivo llamado con el nombre del mes actual + los cuatro dígitos del año en curso
// con el fin de guardar los históricos.
function save_data(temp, hum) {
  var d;

//  Logger.log("--- save_data ---");
  try {

    var ssLogging = SpreadsheetApp.openById(createNewFile());  // Abre la instancia del archivo que guarda los históricos
    var ssGraphic = SpreadsheetApp.getActiveSpreadsheet();  // Abre la instancia del archivo que contiene el script actual
    var dataLoggerSheet = ssLogging.getSheetByName("Hoja 1");  // Abre la instancia de la hoja 1 en la hoja de calculo para guardar los históricos
    var lastDataSheet = ssGraphic. getActiveSheet(); // Abre la instancia de la hoja 1 en la hoja de calculo del script actual ya que, ahí se guardan la posición de los datos en la hoja de cálculo que contiene los históricos.

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
      // Se crea el encabezado para el registro de los últimos 24 datos
      // recibidos al inicio de cada hora, en la Hoja 1 del archivo que 
      // contiene el script de éste proyecto
      lastDataSheet.getRange("A1").setValue("Ultimos 24 datos");  
      lastDataSheet.getRange("A2").setValue("Posición");  
      lastDataSheet.getRange("B2").setValue("Hora");  
      row = 4;
      // Se borran todos los datos que hacía referencia al archivo anterior (si lo hubiese)
      // estos datos guardan el número de fila que contiene el primer dato de cada hora que 
      // envia el sensor remoto (cliente) y sirve de referencia para obtener los datos de la gráfica
      while ( row < 28){
        lastDataSheet.getRange("A" + row).setValue("");
        lastDataSheet.getRange("B" + row).setValue("");
        row++;
      }
      // Se libera este método para que otros usuarios tengan acceso
      lock.releaseLock();
      row = 3;
    }

    // Se guardan los datos en el archivo que contiene el histórico de lecturas
    d=new Date();
    dataLoggerSheet.getRange("A" + row).setValue(row - 2); // ID
    dataLoggerSheet.getRange("B" + row).setValue(Utilities.formatDate(new Date(), "GMT-" + ((d.getTimezoneOffset()/60) + 1), "''dd/MM/yyyy")); // date
    dataLoggerSheet.getRange("C" + row).setValue(Utilities.formatDate(new Date(), "GMT-" + ((d.getTimezoneOffset()/60) + 1), "''HH:mm:ss")); // Time
    dataLoggerSheet.getRange("D" + row).setValue(temp); // temp
    dataLoggerSheet.getRange("E" + row).setValue(hum); // hum    

    // Se crea lista de la posición de los 24 últimos datos recibidos
    // al inicio de cada hora, columnas A y B en la Hoja 1 del archivo que 
    // contiene el script de éste proyecto.
    // Estos datos guardan el número de fila del archivo que contiene los históricos
    // que contiene el primer dato de cada hora que envia el sensor remoto
    // (cliente) y sirve de referencia para obtener los datos de la gráfica
    if (lastDataSheet.getRange("B4").getValue() != d.getHours()){
      lastDataSheet.insertRowsBefore (4,1);  // Se deslizan los datos hacia abajo
      // Se agrega el valor más nuevo
      lastDataSheet.getRange("A4").setValue(row); // Se guarda el valor de la fila actual de la hoja de calculo que contiene los históricos
      lastDataSheet.getRange("B4").setValue(d.getHours()); // Se guarda la hora
      lastDataSheet.deleteRow(29);  // Se borra el valor más antiguo
    }
    SpreadsheetApp.flush();
  }
  catch (error) {
    Logger.log("Falló al guardar documento...");
  }

//  Logger.log("--- save_data end---");
}

// Metodo que lee los datos en el archivo que contiene el histórico.
// Devuelve en formato xml el response de la última fila de datos.
function read_data() {

//  Logger.log("--- read_data ---");
  try {
    var ssLogging = SpreadsheetApp.openById(createNewFile()); // Abre la instancia del archivo que guarda los históricos
    var dataLoggerSheet = ssLogging.getSheetByName("Hoja 1"); // Abre la instancia de la hoja 1 en la hoja de calculo para leer los históricos
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
/* Este metodo devuelve un texto en formato xml que indica 
   cuáles son los meses que se encuentran almacenados en los históricos
*/
function loadOptionValues() {
  var j;
  var tagNameMonth = ["Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"];
  var xmlContent;
  var d = new Date();
  var i = new Date().getMonth();

  var j = 2; // Numero de meses
  xmlContent = "<RESPONSE>\n";
  while (j < 14){
    // Se verifica el archivo que corresponde a cada mes del año en los históricos
    (searchFile(tagNameMonth [i] + d.getUTCFullYear().toString()) != "-1") ? xmlContent += "<secciones>" + j.toString() +"</secciones>\n" : j = 14;  // Al primer ítem que no encuentre indica que no hay más registros.
    j++;
    (i != 0) ? i-- : i = 11;
  }

  xmlContent += "</RESPONSE>\n"
  // Se devuelve el texto en formato xml al metodo solicitado.
  return xmlContent;
}

/* Esta función se encarga de devolver los valores de cada 
   inicio de hora, de las últimas 24 horas para ser graficadas.

*/
function loadGraphicDataValues() {
  var i;

//  Logger.log("--- loadGraphicDataValues ---");
  try {

    var ssLogging = SpreadsheetApp.openById(createNewFile()); // Abre la instancia del archivo que guarda los históricos
    var ssGraphic = SpreadsheetApp.getActiveSpreadsheet(); // Abre la instancia del archivo que contiene el script actual
    var dataLoggerSheet = ssLogging.getSheetByName("Hoja 1"); // Abre la instancia de la hoja 1 en la hoja de calculo para leer los históricos
    var lastDataSheet = ssGraphic. getActiveSheet(); // Abre la instancia de la hoja 1 en la hoja de calculo del script actual ya que, ahí se guardan la posición de los datos en la hoja de cálculo que contiene los históricos.

    var xmlContent = "<RESPONSE>\n";
    var temp;
    var time;
    var hum;
    var row;
    
    i = 0;
    row = 0;

    while ( i < 24){
      // Se obtiene la posicion del dato que se tiene guardado en la hoja de calculo que contiene los históricos.
      row = lastDataSheet.getRange("A" + (i + 4)).getValue();

      if (row != ""){
        // Con la posición obtenida anteriormente, se extrae el dato que se guardan en los históricos
        temp = dataLoggerSheet.getRange("D" + row).getValue();
        hum = dataLoggerSheet.getRange("E" + row).getValue();
        time = dataLoggerSheet.getRange("C" + row).getValue();
      } 
      
      // Se verifica que todos los datos contengan información ya que no se puede enviar al cliente un valor nulo
      if (row == "" || temp == "" || hum == "" || time == ""){
        temp = "-N/A-";
        hum = "-N/A-";
        time = "-N/A-";
      }
      // Se inserta en el texto en formato xml el valor obtenido anteriormente.
      if (i > 9) {
        xmlContent += "<TEMP" + i + ">" + temp +"</TEMP" + i + ">\n";
        xmlContent += "<HUM" + i + ">" + hum +"</HUM" + i + ">\n";
        xmlContent += "<TIME" + i + ">" + time +"</TIME" + i + ">\n";
      } else {
        xmlContent += "<TEMP0" + i + ">" + temp +"</TEMP0" + i + ">\n";
        xmlContent += "<HUM0" + i + ">" + hum +"</HUM0" + i + ">\n";
        xmlContent += "<TIME0" + i + ">" + time +"</TIME0" + i + ">\n";
      }

      // Se actualiza el apuntador para extraer el siguiente dato.
      i++;
    }
    // Se cierra el texto en formato xml
    xmlContent += "</RESPONSE>\n";

//    Logger.log("--- loadGraphicDataValues --- end" + xmlContent);
    // Se devuelve el texto en formato xml al cliente.
    return xmlContent;

  }

  catch (error) {
    Logger.log("Falló al leer documento...");
  }

}

// Esta función encuentra el nombre de archivo solicitado
// y devuelve el Id si el archivo es encontrado, de lo contrario
// devuelve el valor "-1";
function searchFile(name){
  var Id = "-1";
  
  //Logger.log("--- searchFile --- " + ScriptApp.getProjectTriggers()) ;
//  Logger.log("--- searchFile --- ") ;
  // Get any starred spreadsheets from Google Drive, then open the spreadsheets and log the name
  // of the first sheet within each spreadsheet.
  var files = DriveApp.getFilesByName(name);
  while (files.hasNext()) {
    var spreadsheet = SpreadsheetApp.open(files.next());
    //var sheet = spreadsheet.getSheets()[0];
    //Logger.log(sheet.getName() + " -> " + spreadsheet.getName());
    if (spreadsheet.getName() == name) {
      Id = spreadsheet.getId();
      break;
    }
    
  }
  return Id;
}

// Esta función encuentra la carpeta solicitada
// y devuelve el Id si la carpeta es encontrada, de lo contrario
// devuelve el valor "-1";
function searchFolder(name){
  var Id = "-1";
  
  var folders = DriveApp.getFoldersByName(name);
  while (folders.hasNext()) {
    var folder = folders.next();
    if ( folder.getName() == name) {
      Id = folder.getId();
      break;
    }
  }

/*
  var childFolders = DriveApp.getFolderById(Id).getFolders();
  while (childFolders.hasNext()) {
    var folder = childFolders.next();
    Logger.log(folder.getName() + "->" + folder.getId());
  }
*/
  return Id;
}

// Esta función encuentra el nombre del archivo que corresponde
// con el Nombre del mes actual + el año en 4 dígitos actual
// y devuelve el Id correspondiente. En caso de no encontrar
// el archivo, crea uno nuevo dentro de la carpeta "Logging"
function createNewFile(){
  
  var d = new Date();
  var tagNameMonth = ["Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"];
  var IdFile = -1;
  var IdFolder = -1;
  var name = tagNameMonth[d.getMonth()] + d.getUTCFullYear().toString();
  var ssNew;
  var newFolder;

//  Logger.log("--- createNewFiles --- init: " + searchFolder("Respaldo"));

  // Se verifica la existencia del archivo en el Drive de Google
  IdFile = searchFile(name);
  if( IdFile == -1) {
    
    // Si el archivo no existe se bloquea el acceso a éste método para evitar
    // que este metodo se ejecute varias veces al mismo tiempo (Multitarea)
    var lock = LockService.getScriptLock();
    try {
      lock.waitLock(3000); // wait 3 seconds for others' use of the code section and lock to stop and then proceed
    } catch (e) {
      Logger.log('Could not obtain lock after 30 seconds.');
      return HtmlService.createHtmlOutput("<b> Server Busy please try after some time <p>")
      // In case this a server side code called asynchronously you return a error code and display the appropriate message on the client side
      return "Error: Server busy try again later... Sorry :("
    }
    
    // Se crea la carpeta llamada "Logging"
    IdFolder = searchFolder("Logging");
    if (IdFolder == -1) {
      newFolder = DriveApp.createFolder("Logging");
      // Se obtiene el Id del folder recién creado
      IdFolder = newFolder.getId();
    }

    // Se crea el archivo con el Nombre del mes actual + el año en 4 dígitos actual
    ssNew = SpreadsheetApp.create(name);

    // Se obtiene el Id del archivo recién creado
    IdFile = ssNew.getId();
    
    // Se obtiene la instancia del archivo para su manipulación
    docFile = DriveApp.getFileById( IdFile );

    // Se mueve el archivo a la carpeta recién creada
    DriveApp.getFolderById(IdFolder).addFile( docFile );
    
    // Se libera este método para que otros usuarios tengan acceso
    lock.releaseLock();

  }

//  Logger.log("--- createNewFiles --- end" + IdFile);
  return IdFile;
}
/* Esta función se encarga de crear el response que contiene obtiene el contenido 
   del archivo que guarda los históricos e identifica al archivo como un 
   csv y se empaqueta para su envío al cliente.
*/
function downloadFile(name){ // and Hyde
   try
      {
        var ssLogging = SpreadsheetApp.openById(searchFile(name));
        var dataLoggerSheet = ssLogging.getSheetByName("Hoja 1");
        var acontent = convertRangeToCsvFile_("csvFileName", dataLoggerSheet);
        
        var output = ContentService.createTextOutput();
        output.setMimeType(ContentService.MimeType.CSV);
        output.setContent(acontent);
        output.downloadAsFile(name + ".csv");
        //afile.setTrashed(true);
        return output;
      }   catch (e) {
           Logger.log("--- downloadFile --- " + e );
           return ContentService.createTextOutput('Nothing To Download')
      } 
}
/* Esta función se encarga de convertir la hoja de cálculo (spreadsheet)
   en un archivo csv (compatible con excel de Microsoft)
*/
function convertRangeToCsvFile_(csvFileName, sheet) {
  // get available data range in the spreadsheet
  var activeRange = sheet.getDataRange();
  try {
    var data = activeRange.getValues();
    var csvFile = undefined;

    // loop through the data in the range and build a string with the csv data
    if (data.length > 1) {
      var csv = "";
      for (var row = 0; row < data.length; row++) {
        for (var col = 0; col < data[row].length; col++) {
          if (data[row][col].toString().indexOf(",") != -1) {
            data[row][col] = "\"" + data[row][col] + "\"";
          }
        }

        // join each row's columns
        // add a carriage return to end of each row, except for the last one
        if (row < data.length-1) {
          csv += data[row].join(",") + "\r\n";
        }
        else {
          csv += data[row];
        }
      }
      csvFile = csv;
    }
    return csvFile;
  }
  catch(err) {
    Logger.log(err);
  }
}
/* Esta función se encarga de anexar las dependencias (archivos JS y SVG) de la página web
   para que dicha página web funcione correctamente. Google bloquea las dependencias que sean
   guardadas en el Drive u otro sitio (CORS). Los archivos que esta función envia son ControlGraphic,
   Logo, Graphic y JavaScript. Los archivos que Google no bloquea son los JSON, XML, HTML y texto plano.
*/
function include(filename) {
  return HtmlService.createHtmlOutputFromFile(filename).getContent();
}
