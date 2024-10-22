/*
Este código transforma un archivo csv a un formato estandarizado geoJson. El archivo csv debe de tener 3 columnas: latitud, longitud y una columna con la magnitud mapeada. 
Se puede revisar el esquema de geoJson en esta página: https://geojson.org/
*/

Table table; // crea una nuevo objeto tabla en el que se imporatará nuestro archivo csv

JSONObject geoJson; //Declara un objeto JSON vacio con el nombre geoJson
JSONArray features; //Declara una lista vacia de JSON con el nombre features

int maxRSSI = -100; //inicia una variable con el valor máximo esperado de RSSI (received signal strength indicator) de wi-fi
int minRSSI = 0; ////inicia una variable con el valor mínimo esperado de RSSI (received signal strength indicator) de wi-fi

int maxMappedRSSI = 0; //inicia una variable con el valor máximo esperado de RSSI (received signal strength indicator) de wi-fi normalizado de 0 a 100
int minMappedRSSI = 100; //inicia una variable con el valor minimo esperado de RSSI (received signal strength indicator) de wi-fi normalizado de 0 a 100

void setup() { //inicio de la función setup, que engloba todas las acciones que se ejecutan al inicio del programa. 

  table = loadTable("datalog_tarjeta_todos.csv", "header"); //carga el archivo "datalog_tarjeta_todos.csv dentro de nuestra tabla --> para convertir otro archivo sólo cambiar el nombre del archivo aquí
  features = new JSONArray();//inicializa el objeto features
  geoJson = new JSONObject();//inicializa el objeto geoJson
  
  int index = 0; //inicia variable index en 0. Esta variable la usaremos para asignar el lugar dentro de la lista de features de cada elemento
  
  for (TableRow row : table.rows()) { //recorre cada elemento de la tabla
    JSONObject Feature = new JSONObject(); //crea un objeto JSON vacio bajo el nombre Feature
    Feature.setString("type", "Feature");//asigna una primera etiqueta  al objeto feature con el nombre "type" y el valor "Feature"
    JSONObject properties = new JSONObject();//crea un objeto JSON vacio con el nombre de properties
    JSONObject geometry = new JSONObject();//crea un objeto JSON vacio con el nombre geometry
    
    float latitud = row.getFloat(0);//extrae el valor de latitud del elemento actual de la tabla (columna 0 de nuestro archivo)
    float longitud = row.getFloat(1);//extrae el valor de longitud del elemento actual de la tabla (columna 1 de nuestro archivo)
    int RSSI = row.getInt(4); //extrae el elemento RSSI del elemento actual de la tabla (columan 4 de nuestro archivo)
    int mappedRSSI = int( map(RSSI, -100, 0, 0, 100));//normaliza el valor del RSSI de su valor original esperado (entre -100 y 0) a un valor de entre 0 y 100
    String textRSSI = RSSI+" db"; //crea una representación textual del RSSI y añade las letras "db"(decibeles) al final.
    properties.setString("RSSI", textRSSI);//añade el texto de RSSI al objeto JSON properties
    properties.setInt("mappedRSSI", mappedRSSI);////añade el valor normalizado de RSSI al objeto JSON properties
    Feature.setJSONObject ("properties", properties);//añade el objeto properties al objeto Features
    geometry.setString("type", "Point");//añade la etiquete "type" con el valor "point" al objeto JSON geometry
    JSONArray coordinates = new JSONArray();//crea una nueva lista de JSON con el nombre coordinates
    
    coordinates.setFloat(0, longitud);//añade el valor de longitud a la lista coordinates
    coordinates.setFloat(1, latitud); //añade el valor de latitud a la lista coordinates
    geometry.setJSONArray ("coordinates", coordinates);//añade la lista coordinates al objeto geometry
    Feature.setJSONObject ("geometry", geometry);//añade el objeto geometry al objeto Feature
    features.setJSONObject (index, Feature);//añade el objeto Fearure a la lista de features, en la posición asiganada por el valor de index
    index++; //aumenta el valor de index más 1
    
    println(RSSI + ", "+latitud+", "+longitud+", mapped: "+mappedRSSI); //imprime en consola el valor de RSSI, latitud, longitud y el valor nomralizado de RSSI
    
    //compara el valor de RSSI con el valor de la variable maxRSSI. Si es mayor, subtituye ese valor por el valor actual. Al final de la iteración por todos los elmentos, nos dará el valor máximo dentro de nuestra lista
    if (int(RSSI) > maxRSSI){
        maxRSSI = int(RSSI); 
    }
    //compara el valor de RSSI con el valor de la variable minRSSI. Si es menor, subtituye ese valor por el valor actual. Al final de la iteración por todos los elmentos, nos dará el valor mínimo dentro de nuestra lista
    if(int(RSSI) < minRSSI){
       minRSSI = int(RSSI); 
    }
    if (mappedRSSI > maxMappedRSSI){
        maxMappedRSSI = mappedRSSI; 
    }
    if(mappedRSSI < minMappedRSSI){
       minMappedRSSI = mappedRSSI; 
    }
  }
  geoJson.setJSONArray ("features", features);//añade la lista JSON features al objeto geoJSON
  geoJson.setString("type", "FeatureCollection");//añade la etiqueta "type" con el valor "FeatureCollection" al objeto geoJSON
  saveJSONObject(geoJson, "data/geoJson.json");//guarda el objeto geoJSON en un nuevo archivo con el nombre "geoJSON.json"
   println(table.getRowCount() + " total rows in table");//imprime en consola el total de filas dentro del archivo
   println("maxRSSI = "+maxRSSI+", minRSSI = "+minRSSI);//imprime en consola el valor minimo y máximo de RSSI
   println("maxMappedRSSI = "+maxMappedRSSI+", minMappedRSSI = "+minMappedRSSI); //imprime el valor minimo y máximo de RSSI normalizado
}

/* el código crea una extructura como la siguiente a partir de cada elemento de la tabla: 
{
      "geometry": {
        "coordinates": [
          -99.12824249267578,
          19.424972534179688
        ],
        "type": "Point"
      },
      "type": "Feature",
      "properties": {
        "RSSI": "-65 db",
        "mappedRSSI": 35
      }
    }
    
    */
