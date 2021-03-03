
/*  
    Se crean en la gráfica dos series de 24 líneas que unen 
    los puntos de los valores enviados por los sensores, 
	el color verde representa los valores del sensor de temperatura
	y el color azul representa los valores de humedad relativa.
	También se crean una serie de 24 rectángulos que delimitan
	el area de acción del tooltip (cuadro amarillo que aparece cuando 
	el puntero del mouse se ubica sobre la gráfica)
*/
function createItemsGraphic(){
    //Se ubica la gráfica en la página web y se almacena en una variable (svgDoc)
	// Get the Object by ID
	var a = document.getElementById("svgObject");
	// Get the SVG document inside the Object tag
	var svgDoc = a.contentDocument;
	// Other way to get the SVG document inside the Object tag
	//var svgDoc = a.contentDocument.getElementById('myGraphic');
	
	/* 
	El formato de la linea en un documento SVG es:
	<line id="lineTemp0" /> 
	Se crean las 24 líneas que representan los valores
	de la temperatura y se insertan en la gráfica
	*/
	for(j = 0; j < 24; j++) {
		// Se crea el nuevo elemento <line />	
		var newSvgItem=document.createElementNS("http://www.w3.org/2000/svg","line");
		// Se le asigna un nombre para poder encontarlo posteriormente en la gráfica
		newSvgItem.setAttribute("id","lineTemp" + j);
		// Se Agrega a la gráfica
		svgDoc.getElementById('lineTempGroup').appendChild(newSvgItem);
	}

	/* 
	El formato de la linea en un documento SVG es:
	<line id="lineHum0" /> 
	Se crean las 24 líneas que representan los valores
	de la humedad y se insertan en la gráfica
	*/
	for(j = 0; j < 24; j++) {
		// Se crea el nuevo elemento <line />	
		var newSvgItem=document.createElementNS("http://www.w3.org/2000/svg","line");
		// Se le asigna un nombre para poder encontarlo posteriormente en la gráfica
		newSvgItem.setAttribute("id","lineHum" + j);
		// Se Agrega a la gráfica
		svgDoc.getElementById('lineHumGroup').appendChild(newSvgItem);
	}

	/* 
	El formato del rectángulo en un documento SVG es:
	<rect id="section23" x="15" y="0" width="30" height="250" /> 
	Se crean las 24 rectángulos que son el area de acción
	del tooltip y se insertan en la gráfica, cada rectángulo 
	se desplaza 30 px (el valor de la anchura) con respecto al siguiente
	*/
	for(j = 24; j > 0; j--) {
		// Se crea el nuevo elemento <rect />
		var newSvgItem=document.createElementNS("http://www.w3.org/2000/svg","rect");
		// Se le asigna un nombre para poder encontarlo posteriormente en la gráfica
		newSvgItem.setAttribute("id","section" + (j - 1));
		// Se Agrega a la gráfica
		svgDoc.getElementById('sectionGroup').appendChild(newSvgItem);
		// Ya estando en la gráfica, se ubica el elemento creado ... 
		var svgItem = svgDoc.getElementById("section" + (j - 1));
		// ... y se modifican sus parámetros x, y, width y height
		svgItem.setAttribute("x", (15 + ((24 - j) * 30)));  // recuerde que para cada rectángulo, ...
		svgItem.setAttribute("y", 0);  // ... se requiere que se desplace 30 px de manera horizontal
		svgItem.setAttribute("width", 30);  // el punto 'y', el ancho, ...
		svgItem.setAttribute("height", 250);  // ... y la altura es fija.
	}
}


function loadGraphicValues(xmldata)
{
	var i = 0;
	var j = 0;
	var k = 0;
	
	var newData = [0, 0, 0, 0];  // Se utilizan para almacenar temporalmente el valor recibido por el servidor.
	var oldData = ["-N/A-", "-N/A-", "--N/A---", "-N/A-"];  // Se utiliza para almacenar temporalmente los valores extraidos del contenedor.
	var maxData = [0, 0, 0];  // Se utiliza para almacenar el máximo valor recibido de la serie de datos enviados por el servidor (Se requiere un valor minimo al inicio)
	var minData = [100, 100, 100];  // Se utiliza para almacenar el mínimo valor recibido de la serie de datos enviados por el servidor (Se requiere un valor maximo al inicio)
	var mData = [0, 0, 0];			// pendiente de la recta que se traza en la grafica ( se calcula en el programa )
	var bData = [90, 90, 90];		// Offset de la grafica valor inicial cuando (la pendiente) m = 0
	var data = [0, 0, 0];  // Almacena temporalmente los datos extraídos del contenedor que envia el servidor
	var tag = ["TEMP", "HUM", "TIME"];  // Los tags que se esperan recibir por el servidor
	var units = [" °C", " %", " Sec"];   // Unidades de las variables utilizadas.
	var dataMaxMin = ["TempMaxMin", "HumMaxMin"];  // Los tags que se utilizan para ubicar los campos en la gráfica (valores minimos y maximos)
	var lineTag = ["lineTemp", "lineHum"];  // Los tags que se utilizan para ubicar las líneas en la gráfica
	var svgItem;            // Almacena la ubicación de los items de la gráfica.
	var minRango = 140;		// Valor del eje Y que representa el "minimo" en la grafica
	var maxRango = 40;		// Valor del eje Y que representa el "maximo" en la grafica
	var b = 90;				// Offset de la grafica valor inicial cuando m = 0
	
    //Se ubica la gráfica en la página web y se almacena en una variable (svgDoc)
	// Get the Object by ID
	var a = document.getElementById("svgObject");
	// Get the SVG document inside the Object tag
	var svgDoc = a.contentDocument;
	// Other way to get the SVG document inside the Object tag
	//var svgDoc = a.contentDocument.getElementById('myGraphic');
	
	// Se encuentra los valores maximos y minimos de cada serie de datos
	for(k = 0; k < 2; k++) {
		for(i = 0; i < 24; i++) {
			// Se extrae cada valor del contenedor enviado por el servidor (archivo.xml)
			if ( i < 10 ) 
				data[k] = xmldata.getElementsByTagName(tag[k] + '0' + i)[0].firstChild.nodeValue;
			
			if ( i > 9 ) 
				data[k] = xmldata.getElementsByTagName(tag[k] + i)[0].firstChild.nodeValue;
			// Se descartan los que traigan estas etiquetas:
			if ( !(data[k] == "-N/A-") && !(data[k] == "--N/A---") ) {
				if (parseFloat(data[k]) >= parseFloat(maxData[k])) maxData[k] = data[k];  // valor maximo de la serie de datos
				if (parseFloat(data[k]) <= parseFloat(minData[k])) minData[k] = data[k];  // valor mínimo de la serie de datos
			} 
		}
	}
	// Se calcula la pendiente y el offset de cada serie de datos, 
	// basados en el valor máximo y mínimo que se han recibido
	// (Ecuación de la recta mx + y = b)
	for(k = 0; k < 2; k++) {
		if ( (maxData[k] - minData[k]) != 0) {
			mData[k] = ( maxRango - minRango ) / ( maxData[k] - minData[k] );
			bData[k] = minRango - ( mData[k] * minData[k] );
		}
	}
	// Se muestra en la gráfica los valores mínimos y máximos de cada serie de datos en el lado derecho de la gráfica
	for(k = 0; k < 2; k++) {
		svgItem = svgDoc.getElementById(dataMaxMin[k]);
		svgItem.childNodes.item(1).childNodes.item(0).firstChild.data = maxData[k] + units[k];
		svgItem.childNodes.item(1).childNodes.item(1).firstChild.data = minData[k] + units[k];
		
	}
    // Cuando los datos no han llegado, en la gráfica se muestra una línea en la parte inferior
	// dicha línea se oculta cuando los datos ya se han recibido.
	svgItem = svgDoc.getElementById("lineTemp");
	svgItem.setAttribute("visibility", "hidden");

	j = 0;
	// Cada dato recibido, se extrae del contenedor (archivo.xml) y se pega en la gráfica.
	for(k = 0; k < 3; k++) {
		for(i = 750; i > 30; i-=30) {
			// Se extraen los datos del contenedor (archivo xml) que envia el servidor y son los que se grafican
			if ( j < 10 ) {
				newData[k] = xmldata.getElementsByTagName(tag[k] + '0' + j)[0].firstChild.nodeValue;
			}
			if ( j > 9 ) {
				newData[k] = xmldata.getElementsByTagName(tag[k] + j)[0].firstChild.nodeValue;
			}
			// Get one of the SVG items by ID;
			svgItem = svgDoc.getElementById("section" + j);
			// Cada valor extraído se asigna a la línea correspondiente
			// para ir ensamblado la grafica línea por línea
			if ( k < 2 ) {	// Solo los dos primeros valores se grafican (temperatura y humedad)
				// Get one of the SVG items by ID;
				svgItem = svgDoc.getElementById(lineTag[k] + j);
				// Set the colour to something else
				if ( newData[k] == "-N/A-" || oldData[k] == "-N/A-" || newData[k] == "--N/A---" || oldData[k] == "--N/A---") {
					svgItem.setAttribute("visibility", "hidden");
				} else {
					svgItem.setAttribute("visibility", "visible");
					svgItem.setAttribute("x1", i-30);
					svgItem.setAttribute("y1", ((mData[k] * newData[k]) + bData[k]));
					svgItem.setAttribute("x2", i);
					svgItem.setAttribute("y2", ((mData[k] * oldData[k]) + bData[k]));
				}
			}
			// Se extraen los datos del contenedor y se insertan en el tooltip
			// se mostrará el valor de acuerdo al rectángulo asignado en la gráfica
			// es el equivalente a tener 24 toolstips listos para ser mostrados cuando se requiera.
			if ( k == 2 ) {	// Hasta ya se hayan graficado TODOS los valores se inserta el tooltip en la gráfica
				if ( j < 10 ) {
					newData[k-2] = xmldata.getElementsByTagName(tag[k-2] + '0' + j)[0].firstChild.nodeValue;
					newData[k-1] = xmldata.getElementsByTagName(tag[k-1] + '0' + j)[0].firstChild.nodeValue;
					newData[k] = xmldata.getElementsByTagName(tag[k] + '0' + j)[0].firstChild.nodeValue;
				}
				if ( j > 9 ) {
					newData[k-2] = xmldata.getElementsByTagName(tag[k-2] + j)[0].firstChild.nodeValue;
					newData[k-1] = xmldata.getElementsByTagName(tag[k-1] + j)[0].firstChild.nodeValue;
					newData[k] = xmldata.getElementsByTagName(tag[k] + j)[0].firstChild.nodeValue;
				}
				// Se insertan los valores en el tooltip para que esté disponible para mostrarse cuando se le requiera.
				svgItem.setAttribute("onmousemove", "ShowTooltip(evt, '" + newData[k-2] + " °C \\\\\\n " + newData[k-1] + " % \\\\\\n " + newData[k] + "');");
			}
			j++;
			// Se almacena el valor anterior, ya que servirá para "recordar" cual fué el último dato
			// y servirá como punto final de la línea siguiente. La recta inicia desde el dato nuevo hacia el dato anterior.
			oldData[k] = newData[k];
			
		}
		j = 0;
	}
}
