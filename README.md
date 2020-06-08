# practica1

Práctica 1

Para probar esta práctica, primero hay que realizar el compilado de los módulos y generar el 
archivo ejecutable (server) con el makefile proporcionado (> make).

El Makefile contiene una serie de órdenes, entre las cuales está help (> make help). Con esto
se muestra el resto de órdenes posibles con este fichero con una breve explicación de ayuda.

Tras obtener el ejecutable, simplemente se debe poner en marcha (> ./server) y activar el 
servidor. En este caso, todas las peticiones de archivos que se quieran obtener del servidor 
se deberán realizar en la url: http://0.0.0.0:2772/archivo_solicitado

Se debe de tener en cuenta que este archivo debe encontrarse dentro de la carpeta data/, que
es la carpeta raíz de archivos de nuestro servidor.

Se incluye el archivo index.html para la prueba del profesor. Sin embargo, este debe incorporar
la carpeta de media/ manualmente a la carpeta data/ si quiere poder ver las imágenes y gifs 
correspondientes (ya que debido al peso de esta carpeta, no la hemos subido al repositorio).
También se incluyen otros archivos para probar, como los scripts en la carpeta scripts/ y los
archivos de texto plano.

Toda esta información y más, como las especificaciones del servidor y cómo funciona por 
debajo, viene bien detallado en la wiki de esta práctica.