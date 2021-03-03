// Esta sección se encarga de la cominicación asíncrona entre cliente-servidor (Modulo ESP32 y PC ó el móvil)
// Envía, espera y recibe las peticiones hechas al servidor, por último se encarga de mostrar los valores en el lugar asignado.


// Determines when a request is considered "timed out"
var timeOutMS = 10000; //ms
var refreshTime = 50; //ms
var timerID = null;
var lastTemp;	//Ultimo valor de temperatura

// Stores a queue of AJAX events to process
var ajaxList = new Array();

/*
  Crea el objeto para realizar las comunicaciones con el servidor, 
  son utilizadas por las funciones newAJAXCommand y EnviaDatos
*/

function ajax(){
	var req = false;
	try{
		req = new XMLHttpRequest(); 
	}
	catch(err1){
		try{
			req = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch(err2){
			try{
				req = new ActiveXObject("Microsoft.XMLHTTP");
			}
			catch(err3){
				req = false;
			}
		}
	}
	return req;
}

/*
  Esta función crea el contenedor para almacenar una función,
  la cual, mediante la función pollAJAX, se mantiene funcionando
  periódicamente.
*/
// Initiates a new AJAX command
// url: the url to access
// container: the document ID to fill, or a function to call with response XML (optional)
// repeat: true to repeat this call indefinitely (optional)
// data: an URL encoded string to be submitted as POST data (optional)
function newAJAXCommand(url, container, repeat, data)
{
	// Set up our object
	var newAjax = new Object();
	var theTimer = new Date();
	newAjax.url = url;
	newAjax.container = container;
	newAjax.repeat = repeat;
	newAjax.ajaxReq = null;
	newAjax.runOnce = false;
	
	// Create and send the request
	newAjax.ajaxReq = ajax();
	newAjax.ajaxReq.open((data==null)?"GET":"POST", newAjax.url, true);
	newAjax.ajaxReq.send(data);
	
	newAjax.lastCalled = theTimer.getTime();
	
	// Store in our array
	ajaxList.push(newAjax);
}

/*
  Esta función se encarga de mantener activa las peticiones periódicas 
  que se programan para la comunicación con el servidor.
*/
function pollAJAX() { 
	var curAjax = new Object();
	var theTimer = new Date();
	var elapsed;
	
	// Read off the ajaxList objects one by one
	for(i = ajaxList.length; i > 0; i--)
	{
		curAjax = ajaxList.shift();
		if(!curAjax) continue;
		
		elapsed = theTimer.getTime() - curAjax.lastCalled;  //elepsed contiene el tiempo transcurrido
										//desde el envio del request.
		
		if (!curAjax.runOnce) {   //se ejecuta desde el envio del request hasta recibir el response.
			
			// If we suceeded
			if(curAjax.ajaxReq.readyState == 4 && curAjax.ajaxReq.status == 200) {
				
				//Si el response llego con exito entonces establecemos la bandera runOnce a cierto.
				curAjax.runOnce = true;
				// If it has a container, write the result
				if(typeof(curAjax.container) == 'function'){
					curAjax.container(curAjax.ajaxReq.responseXML.documentElement);
				} else if(typeof(curAjax.container) == 'string') {
					document.getElementById(curAjax.container).innerHTML = curAjax.ajaxReq.responseXML.documentElement.getElementsByTagName(curAjax.container)[0].firstChild.nodeValue;
				}	// (otherwise do nothing)
				
				curAjax.ajaxReq.abort();
				curAjax.ajaxReq = null;
				
				// If it's a repeatable request, porque repeat es mayor que 0 ms.
				if(curAjax.repeat>0)
					// Otherwise, just keep waiting
					ajaxList.push(curAjax);   //si utilizaramos newAjax, el tiempo se reinicializara.
				
				continue;
			}
			
			// If we've waited over 5 second, then we timed out
			if(elapsed > timeOutMS) {
				// Invoke the user function with null input
				if(typeof(curAjax.container) == 'function'){
				//curAjax.container(null);
				} else {
				// Alert the user\n					alert("Command failed.\nConnection to development board was lost.");
			}
			
			curAjax.ajaxReq.abort();
			curAjax.ajaxReq = null;
			
			// If it's a repeatable request, then do so
			if(curAjax.repeat>0)
				newAJAXCommand(curAjax.url, curAjax.container, curAjax.repeat);
				continue;
			}
			//fin de runOnce = false 
			
		}
		else if (elapsed >= curAjax.repeat)  {	//este else, se ejecuta despuÃ©s de procesado el response en adelante.
			//si y es hora de ejecutar nuevamente
			if(curAjax.repeat>0)
				newAJAXCommand(curAjax.url, curAjax.container, curAjax.repeat);
			continue;     
		}
		
		// Otherwise, just keep waiting
		ajaxList.push(curAjax);   //si utilizaramos newAjax, el tiempo se reinicializara.
	}
	
	// Call ourselves again in 10ms
	timerID = setTimeout("pollAJAX()", refreshTime);    //VELOCIDAD DE RECARGA DE LOS PARAMETROS DINAMICOS.   
}

/*
  Esta función se encarga de recibir los parámetros enviado por el servidor,
  se encarga de ubicar el tag en la página HTML e insertar el valor que el servidor 
  acaba de enviar. Es decir, si el servidor envió: <SALUDO>Hola Mundo</SALUDO>, esta función
  ubica el tag con el id="SALUDO" en la página web e inserta el contenido "Hola Mundo" entre
  los tags "text", así como se muestra: <text id="SALUDO"></text>
  Se ha probado con:
  ... <input type="text" id="SALUDO"></input> 
  ... checkbox con valores true o false
  ... también es capaz de desabilitar opciones en un combo box enviando la posicion que se desea deshabilidar (0 ~ etc).
*/
function RecibeDatos(xmldata) {
	
	var newData;
	var i = 0;
	var nodes = xmldata.childNodes;
  var tag, tagHTML, typeHTML;
	
	for(i = 0; i < nodes.length; i++) {
	  if (nodes[i].nodeType == 1) { //sólo nodos tipo "element" (Type=1)
	    newData = nodes[i].childNodes[0].nodeValue; //Contenido del tag del xml
        tag = nodes[i].nodeName; //Nombre de la etiqueta del xml
		  tagHTML = document.getElementById(tag.toString());  // Búsqueda del tag en el documento html
		  if (tagHTML != null){ // Se verifica la existencia del tag en el documento html
		    typeHTML = tagHTML.type; // Se determina el tipo del tag en el documento HTML
		    if(typeHTML == 'checkbox' || typeHTML == 'radio')  // only values: true or false
		      if (newData == "true") 
			      tagHTML.checked = true;
          else
			      tagHTML.checked = false;
        else if (typeHTML == null)
		      tagHTML.innerHTML = newData;
		    else if (typeHTML == 'select-one')  // combo box only disable items
          tagHTML.options[newData].disabled = false;
        else
		      tagHTML.value = newData;
	    }
	  }
	}
	
}
/*
  Esta función se encarga de enviar las parámetros al recurso solicitado por el cliente
*/
function EnviaDatos(recurso, args) {
	var xmlhttp;
	xmlhttp = ajax();
	

	// Formato de envio en ambos métodos: enviar.html?dato1=valor1&dato2=valor2

	// Metodo GET
	xmlhttp.open("GET", recurso + args, true);
	xmlhttp.send();
	
	// Metodo POST
	//xmlhttp.open("POST",recurso,true);
	//xmlhttp.setRequestHeader("Content-type","application/x-www-form-urlencoded");
	//xmlhttp.send(args);
}

/*
  Esta función se encarga de programar las llamadas periódicas al servidor (newAJAXCommand),
  crea los items para preparar la gráfica para la recepción de los datos (createItemsGraphic),
  y mantiene agendadas las peticiones al servidor (pollAJAX).
*/
function inicializarEventos()
{
	createItemsGraphic();
	newAJAXCommand('./dataGraphics.xml',loadGraphicValues, 1000);	// Se carga los valores de la grÃ¡fica
	newAJAXCommand('./temp.xml',RecibeDatos,1000);	//Refresh temp and alarm parameters after 500 ms.
	pollAJAX();
}
/*
  Esto programa el funcionamiento apenas se tengan TODOS los archivos descargados
  que solicita la página index.htm
*/
window.onload = function(){
	inicializarEventos();
}
