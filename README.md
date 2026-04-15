# Sistema de Navegación Urbana: Algoritmo de Dijkstra en C

Implementación de alto rendimiento del algoritmo de Dijkstra para la optimización de trayectorias y búsqueda del camino más corto en redes urbanas complejas. 

El sistema está desarrollado completamente en C estándar, priorizando la eficiencia computacional y el manejo manual de memoria. Para maximizar la velocidad de búsqueda y reducir los tiempos de ejecución, el motor prescinde de librerías externas de estructuras de datos e implementa desde cero:
* **Tabla Hash:** Sistema de almacenamiento de nodos con resolución de colisiones mediante sondeo cuadrático y redimensionamiento dinámico de memoria (`realloc`).
* **Cola de Prioridad (Min-Heap):** Estructura de árbol binario para la extracción en tiempo constante (O(1)) del nodo con la menor distancia acumulada, optimizando las iteraciones del algoritmo.

## Requisitos y Tecnologías
* **Lenguaje:** C (C99 o superior).
* **Compilador:** GCC, Clang o MSVC.
* **Librerías:** Estándar de C (`stdio.h`, `stdlib.h`, `string.h`, `stdint.h`, `time.h`, `stdbool.h`). No se requieren dependencias externas.

## Estructura de Datos (Archivos I/O)

Para que el programa se ejecute correctamente, requiere leer un archivo de base de datos de la red vial y generará un archivo de salida con los resultados.

### Archivo de Entrada Requerido
El ejecutable debe estar en el mismo directorio que el archivo `aristas_completo.csv`. Este archivo contiene la información del grafo urbano y debe seguir estrictamente este formato (separado por comas):
`origen,destino,longitud,nombre_calle`

### Archivo de Salida
Una vez calculado el camino más corto, el sistema genera automáticamente un archivo llamado `camino.csv` que contiene:
1. El nodo de origen y destino junto con el nombre de las calles.
2. La secuencia completa de IDs de los nodos que conforman la ruta óptima.

## Instrucciones de Compilación y Uso

1. Abre una terminal en el directorio del proyecto.
2. Compila el código fuente utilizando GCC (o tu compilador de preferencia):
   `gcc main.c -o dijkstra_nav`
3. Ejecuta el programa compilado:
   * En Windows: `dijkstra_nav.exe`
   * En Linux/Mac: `./dijkstra_nav`

## Interacción con el Sistema
Al iniciar, el sistema cargará el archivo CSV, poblará la Tabla Hash y el Min-Heap en memoria, y desplegará una interfaz en consola.
1. Ingresa el nombre de la calle de **origen**.
2. Ingresa el nombre de la calle de **destino**.
3. El sistema calculará la ruta e imprimirá en pantalla:
   * La distancia total de la ruta en kilómetros.
   * El tiempo exacto de ejecución del algoritmo.
   * El tiempo de carga de las estructuras de datos.
   * La secuencia paso a paso de las calles a tomar.
