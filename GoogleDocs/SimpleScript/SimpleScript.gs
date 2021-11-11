/* 

  Created by Gabriel Pool, 2021

*/

/*
Este script tiene el propósito de recibir la información que envía el cliente (sensor remoto)
y almacenarla en el actual archivo. Si la variable tag no está definida con el valor "Update",
los datos NO se guardarán. Este script siempre responderá con un eco de los datos enviados.

*/

function doGet(e) {

  var tag = "",
    temp = "",
    hum = "";

  try {

    // this helps during debuggin
    if (e == null){e={}; e.parameters = {tag:"Update",Temp:"25.00",Hum:"50.00"};}

    // Se extrae el valor de ítem "tag"
    tag = e.parameters.tag;
        
    // Se guardan los datos en el actual archivo.
    if (tag == "Update"){
      temp = e.parameters.Temp;
      hum = e.parameters.Hum;
    // save the data to spreadsheet
    save_data(temp, hum);
    }
    
    // Se envía como respuesta los parametros recibidos
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
