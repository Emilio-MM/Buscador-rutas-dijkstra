#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#define INF_P UINT64_MAX
#define INF_N 0

////////////////////// TABLA HASH ////////////////////// 
size_t total_size = 110161;  
size_t total_size_heap = 63680;  

enum nodo_status{
    VISITED,
    NOT_VISITED
};

typedef struct {
    uint64_t destino; //llu
    double longitud; //.2f
    char nombre[70];
} vecino;

typedef struct{//son los heads, cada head contiene su informacion
    vecino *vecinos;//cada nodo tendra su vecino
    size_t cant_vecinos;
    size_t cap_vecinos;
    uint64_t id_nodo;
    uint64_t pos_heap;
    uint64_t previo;//nodo anterior en el camino
    char previo_calle[70];
    double distancia_acum;//distancia acumulada
    enum nodo_status status;//marcar visitado o no visitado
}   nodo;

typedef struct{//tabla que contiene heads
    nodo *head;
    size_t hash_size;
    size_t n_nodos;//cantidad total de nodos
}   hashTable;

////////////////////////// HEAP ////////////////////////
typedef struct {
    double distancia;//clave principal para ordenar 
    uint64_t node_id;//nodo asociado
} heap_item;

typedef struct {
    heap_item *items;// arreglo de heap_items
    size_t cap;//total number of available positions 
    size_t cont;//index of the last element
} heap;

/////////////////////////// LOSH DOSH ///////////////////////
typedef struct{
    heap *H;
    hashTable *T;
}   heapash_struct; 


size_t find_pos(hashTable *T, uint64_t origen) {
    size_t pos = origen % T->hash_size;
    size_t i = 0;
    size_t initial_pos = pos;
    
    while (T->head[pos].id_nodo != UINT64_MAX && T->head[pos].id_nodo != origen) {
        i++;
        pos = (initial_pos + (i * i)) % T->hash_size;  
        
        if (i >= T->hash_size) {
            fprintf(stderr, "Error: Tabla hash llena\n");
            exit(1);
        }
    }
    return pos;
}

void reserve_memory(hashTable *T, uint64_t pos) {//caso inicial reservar memoria para 2 vecinos
    if (T -> head[pos].vecinos == NULL){ 
        T -> head[pos].cap_vecinos = 3;
        T -> head[pos].vecinos = (vecino*)malloc(sizeof(vecino)*T -> head[pos].cap_vecinos); //reserva 2 espacios al principio

        if (T -> head[pos].vecinos == NULL){
        fprintf(stderr,"Error en memoria 3\n");
        exit(1);
        }

    }
}

void increase_memory(hashTable *T, uint64_t pos){ //caso donde nos quedamos sin memoria en una chain para mas vecinos

    if (T -> head[pos].cant_vecinos >= T->head[pos].cap_vecinos){ //si nos quedamos sin capacidad entonces aumentamos
        size_t newCap = T -> head[pos].cap_vecinos*2;
        vecino *aux = realloc(T->head[pos].vecinos, (newCap) * sizeof(vecino));//incrementamos la memoria 
        if (aux == NULL){
        fprintf(stderr,"Error en memoria 4\n");
        exit(1);
        }

        T -> head[pos].vecinos = aux;
        T -> head[pos].cap_vecinos = newCap;
    }
} 
/********************************************************************** CREACION *****************************************************************************/
hashTable *new_HT(){
    hashTable *T = (hashTable*)malloc(sizeof(hashTable));//primero debemos reservarle algo de memoria a la estructura para ahora si trabajar con ella
    if (T == NULL){
        fprintf(stderr,"Error en memoria 1\n");
        exit(1);
    }

    T -> hash_size = total_size;
    T -> n_nodos = 0;
    T -> head = (nodo*)malloc(sizeof(nodo) * T -> hash_size); //reservamos el espacio de esa cantidad de heads
    if (T -> head == NULL){
        fprintf(stderr,"Error en memoria 2\n");
        exit(1);
    }
    
    for(size_t i = 0 ; i < T->hash_size ; i++){ //esto ira por cada head asignandole sus valores iniciales
        T -> head[i].cant_vecinos = 0;
        T -> head[i].cap_vecinos = 0;
        T -> head[i].id_nodo = UINT64_MAX;
        T -> head[i].vecinos = NULL;
        T -> head[i].distancia_acum = UINT64_MAX;
        T -> head[i].status = NOT_VISITED;
        T -> head[i].previo = UINT64_MAX;
        T -> head[i].previo_calle[0] = '\0';
        T -> head[i].pos_heap = 0;
    }
    return T;
}

heap* new_HEAP(){
    heap *H = (heap*)malloc(sizeof(heap));
    if (H == NULL){
        fprintf(stderr,"Error en memoria \n");
        exit(1);
    }
    
    H -> cap = total_size_heap;//se inicia en los valores que ya se conocen
    H -> cont = 0;
    H -> items = (heap_item*)malloc(sizeof(heap_item)*(H->cap+1)); //se guarda el espacio para h->cap + 1, la posicion 0 no la utilizamos
    if (H -> items == NULL){
        fprintf(stderr,"Error en memoria \n");
        exit(1);
    }

    H->items[0].distancia = INF_N;
    return H;
}

heapash_struct *new_HEAPASH(){
    heapash_struct *newHH = (heapash_struct*)malloc(sizeof(heapash_struct));
    if (newHH == NULL){
        fprintf(stderr,"Error en memoria \n");
        exit(1);
    }
    
    newHH->T = new_HT();
    newHH->H = new_HEAP(newHH->T);
    return newHH;
}


/******************************************************************************** LLENAR LA TABLA *****************************************************************************/
void fill_HEAP(heapash_struct *HH, double distancia, uint64_t node_id){

    heap *H = HH->H;
    hashTable *T = HH->T;

    //insertamos en el heap la distancia y el nodo en un slot juntos
    H->items[H->cont+1].distancia = distancia; 
    H->items[H->cont+1].node_id = node_id; 
    H->cont++;
    //avisamos a hashtable la posicion que se les dió
    T->head[find_pos(T,node_id)].pos_heap = H->cont;
    
}

void fill_HT(heapash_struct *HH) {

    hashTable *T = HH->T;

    //////// ABRIR ARCHIVO /////////
    FILE *archivo = fopen("aristas_completo.csv", "r");
    if (!archivo) {
        printf("Error abriendo archivo");
        exit(1);
    }
    
    char linea[256]; //sirve pa guardar una linea entera del archivo
    fgets(linea, sizeof(linea), archivo); // leer la primer linea pero no se usa

    while (fgets(linea, sizeof(linea), archivo)) {

        uint64_t origen, destino;
        double longitud;
        char nombre[50];
        sscanf(linea, "%llu,%llu,%lf,%49[^\n]", &origen, &destino, &longitud, nombre);

        ////GUARDAMOS VALORES PARA NODO 1////
        uint64_t pos = find_pos(T,origen);
        reserve_memory(T,pos);
        increase_memory(T,pos);
        
        T->head[pos].vecinos[T->head[pos].cant_vecinos].destino = destino;
        T->head[pos].vecinos[T->head[pos].cant_vecinos].longitud = longitud;
        strcpy(T->head[pos].vecinos[T->head[pos].cant_vecinos].nombre, nombre); //no se puede hacer asignacion directa a un arreglo de caracteres

        if (T->head[pos].id_nodo == UINT64_MAX) {
            T->head[pos].id_nodo = origen;
            T->n_nodos++; //actualizamos cantidad de nodos
            fill_HEAP(HH,INF_P,origen);
        }
        T->head[pos].cant_vecinos++;
        
    }
    
    fclose(archivo);
}

/**************************************************************************** DELETE ***************************************************************************/
void heapify_down(heapash_struct *, size_t );
void delete_heap_first(heapash_struct *HH){

    heap *H = HH->H;

    //manda el numero a borrar al final 
    H->items[1].distancia = H->items[H->cont].distancia;
    H->items[1].node_id = H->items[H->cont].node_id;
    H->items[H->cont].distancia = INF_P;
    H->cont--;
    
    heapify_down(HH,1);    
    
}
/************************************************************************ DIJKSTRA *****************************************************************************/
size_t parent(size_t pos)  {return pos >> 1;} //divide entre 2 truncado para encontrar al apa
size_t left_child(size_t pos)  { return pos << 1;} //el hijo izquierdo esta en 2*i
size_t right_child(size_t pos)  {return (pos << 1) + 1;} //el hijo izquierdo esta en 2*i + 1

void heapify_up(heapash_struct *HH, size_t pos_act){
    heap *H = HH->H;
    hashTable *T = HH->T;

    while(H->items[pos_act].distancia < H->items[parent(pos_act)].distancia){//aqui vemos que los padres tengan que ser menores 
        //swap de distancias 
        double aux_dist = H->items[pos_act].distancia; 
        H->items[pos_act].distancia = H->items[parent(pos_act)].distancia;//1er numero
        H->items[parent(pos_act)].distancia = aux_dist; //2do numero
        
        //swap de nodo_Id 
        uint64_t aux_id = H->items[pos_act].node_id; 
        H->items[pos_act].node_id = H->items[parent(pos_act)].node_id;//1er numero
        H->items[parent(pos_act)].node_id = aux_id; //2do numero

        //marcamos en la HT las nuevas posiciones que tienen las distancias en el heap
        uint64_t new_pos = find_pos(T,H->items[pos_act].node_id);
        uint64_t new_pos2 = find_pos(T,H->items[parent(pos_act)].node_id);
        T->head[new_pos].pos_heap = pos_act;
        T->head[new_pos2].pos_heap = parent(pos_act);

        pos_act = parent(pos_act);
    }
}

void heapify_down(heapash_struct *HH, size_t pos_act) {
    heap *H = HH->H;
    hashTable *T = HH->T;

    while (1) {
        size_t pos_left = left_child(pos_act);
        size_t pos_right = right_child(pos_act);
        size_t smallest = pos_act;

        if (pos_left <= H->cont && H->items[pos_left].distancia < H->items[smallest].distancia)
            smallest = pos_left;

        if (pos_right <= H->cont && H->items[pos_right].distancia < H->items[smallest].distancia)
            smallest = pos_right;

        if (smallest == pos_act)//aqui ya llego al final del heap
            break; 

        //swap de nodos en el heap 
        heap_item temp = H->items[pos_act];
        H->items[pos_act] = H->items[smallest];
        H->items[smallest] = temp;

        //actualizar posiciones en la ht
        uint64_t new_pos1 = find_pos(T, H->items[pos_act].node_id);
        uint64_t new_pos2 = find_pos(T, H->items[smallest].node_id);
        T->head[new_pos1].pos_heap = pos_act;
        T->head[new_pos2].pos_heap = smallest;

        pos_act = smallest; //actualiza posicione actual
    }
}

size_t algoritmo(heapash_struct *HH , size_t nodo_act, size_t nodo_dest_1, size_t nodo_dest_2) {
    heap *H = HH->H;
    hashTable *T = HH->T;

    //iniclaizar nodo origen
    size_t pos_origen = find_pos(T, nodo_act);
    size_t pos_heap_origen = T->head[pos_origen].pos_heap;
    H->items[pos_heap_origen].distancia = 0;
    heapify_up(HH, pos_heap_origen);
    T->head[pos_origen].distancia_acum = 0;
    T->head[pos_origen].status = VISITED;
    

    while (nodo_act != nodo_dest_1 && nodo_act != nodo_dest_2) {
        uint64_t pos = find_pos(T, nodo_act);//posicion nodo en la HT

        for (size_t i = 0; i < T->head[pos].cant_vecinos; i++) {//actualizar vecinos
            
            uint64_t vecino = T->head[pos].vecinos[i].destino;
            double nueva_dist = T->head[pos].distancia_acum + T->head[pos].vecinos[i].longitud;

            uint64_t pos_vecino = find_pos(T, vecino);//posicion de vecino en HT
            uint64_t pos_vecino_heap = T->head[pos_vecino].pos_heap;//posicion de vecino en heap

            if (T->head[pos_vecino].status == NOT_VISITED && nueva_dist < H->items[pos_vecino_heap].distancia) {//si el recorrido mejora actualizamos
                H->items[pos_vecino_heap].distancia = nueva_dist;
                T->head[pos_vecino].distancia_acum = nueva_dist;
                T->head[pos_vecino].previo = nodo_act;
                strcpy(T->head[pos_vecino].previo_calle, T->head[pos].vecinos[i].nombre);
                heapify_up(HH, pos_vecino_heap);
            }
        } 
        
        delete_heap_first(HH); //eliminamos el nodo actual del heap
        nodo_act = H->items[1].node_id;
        T->head[find_pos(T, nodo_act)].status = VISITED;
    }

    double distancia_total = T->head[find_pos(T, nodo_act)].distancia_acum;//mostrar la distancia final 
    printf("- Distancia total(km): %.2f\n\n", distancia_total/1000);
    return nodo_act;
}

/************************************************************************* EXTRA *****************************************************************************/
void printTable(hashTable *T){
    for (size_t j = 0 ; j < T -> hash_size ; j++){

        printf("%zu: ", j);
        if(T->head[j].id_nodo != UINT64_MAX){
            printf("(nodo:%llu,pos_heap:%llu) - ", T->head[j].id_nodo, T->head[j].pos_heap);
        }

        if (T->head[j].id_nodo != UINT64_MAX) {
            for (size_t i = 0; i < T->head[j].cant_vecinos; i++) {
                printf("(%llu, ", T->head[j].vecinos[i].destino);
                printf("%.2f, ", T->head[j].vecinos[i].longitud);
                printf("%s) ", T->head[j].vecinos[i].nombre); 
            }
        } else {
            printf("*");
        }
        printf("\n");
    }
    printf("\n");
}
void freeTable(hashTable *T) {
    if (T == NULL) return;

    for (size_t i = 0; i < T->hash_size; i++) {
        free(T->head[i].vecinos);
    }

    free(T->head);
    free(T);
}
void printHeap(heap *H){

    for (size_t i=1; i<H->cont ; i++){
        printf("(%f,%llu) ", H->items[i].distancia , H->items[i].node_id);
    }
    printf("\n");
    
}
void freeHeap(heap *H) {
    if (H == NULL) return;
    free(H->items);
    free(H);
}
void freeHeapash(heapash_struct *HH) {
    if (HH == NULL) return;
    freeTable(HH->T);
    freeHeap(HH->H);
    free(HH);
}
void guardar_camino_csv(uint64_t *camino, size_t longitud, const char *calle_origen, const char *calle_destino) {
    FILE *f = fopen("camino.csv", "w");
    if (!f) {
        printf("No se pudo crear camino.csv\n");
        return;
    }
    //se guarda origen y desitno del camino
    fprintf(f, "origen,destino,calle_origen,calle_destino\n");
    fprintf(f, "%llu,%llu,%s,%s\n", camino[longitud], camino[0], calle_origen, calle_destino);
    
    //guardar camino completo
    fprintf(f, "\n# Camino completo\n");
    fprintf(f, "origen,destino\n");
    for (size_t i = longitud; i > 0; i--) {
        fprintf(f, "%llu,%llu\n", camino[i], camino[i-1]);
    }
    fclose(f);
}

size_t find_calle(hashTable *T, const char *nombre_buscado, bool aux_nodo) {
    FILE *archivo = fopen("aristas_completo.csv", "r");
    if (!archivo) {
        printf("Error abriendo archivo");
        exit(1);
    }
    
    char linea[256];
    fgets(linea, sizeof(linea), archivo); 
    
    while (fgets(linea, sizeof(linea), archivo)) {
        uint64_t origen, destino;
        double longitud;
        char nombre[70] = ""; //iniciar el string vaciote
        
        if (sscanf(linea, "%llu,%llu,%lf,%69[^\n]", &origen, &destino, &longitud, nombre) == 4) {//verificar formato correcto
            
            while (nombre[strlen(nombre) - 1] == '\n' || nombre[strlen(nombre) - 1] == '\r') {
                nombre[strlen(nombre) - 1] = '\0';
            }

            if (nombre[0] != '\0' && strcmp(nombre_buscado, nombre) == 0) {//solo comparar si tiene nombre de calle
                fclose(archivo);
                if (aux_nodo) {
                    return find_pos(T, destino);
                } else {
                    return find_pos(T, origen);
                }
            }
        }

    }
    
    fclose(archivo);
    if(aux_nodo == false) printf("    No se encontro la calle '%s' en el archivo. Intente nuevamente.\n\n", nombre_buscado);
    return UINT64_MAX;
}

void print_camino(heapash_struct *HH , size_t node_origen, size_t node_act, const char *calle_origen, const char *calle_destino){
    hashTable *T = HH->T;
    uint64_t camino[1000];
    char nombres[1000][70]; 
    size_t indice = 0;

    //inicios
    camino[0] = node_act;
    strcpy(nombres[0], T->head[find_pos(T, node_act)].previo_calle); 

    while(node_act != node_origen){
        indice++;
        node_act = T->head[find_pos(T, node_act)].previo;
        camino[indice] = node_act;
        strcpy(nombres[indice], T->head[find_pos(T, node_act)].previo_calle);
    }
    
    for (size_t i = indice + 1; i > 0 ; i--) {
        printf("%s -> ", nombres[i - 1]);
    }

    printf("\n");
    guardar_camino_csv(camino, indice, calle_origen, calle_destino);
}


int main(){
    clock_t inicio, fin;
    double tiempo_total_fill;
    double tiempo_total_algoritmo;


    heapash_struct *HH = new_HEAPASH();
    hashTable *T = HH->T;
    
    inicio = clock();
    fill_HT(HH);//llenar tabla y heap
    fin = clock();
    tiempo_total_fill = ((double)(fin - inicio)) / CLOCKS_PER_SEC;


    system("cls");
    printf("\n");
    printf("  +--------------------------------------------------+\n");
    printf("  |            SISTEMA DE NAVEGACION URBANA          |\n");
    printf("  +--------------------------------------------------+\n");
    printf("\n");

    printf("  +--------------------------------------------------+\n");
    printf("  | Ingrese los datos de navegacion:                 |\n");
    printf("  +--------------------------------------------------+\n\n");

    char calle_origen[50]; 
    char calle_destino[50];
    size_t pos_origen = UINT64_MAX;
    size_t pos_destino_1 = UINT64_MAX;
    size_t pos_destino_2 = UINT64_MAX;

    while(1){
        while(pos_origen == UINT64_MAX){
            printf("- Localizacion de origen: ");
            scanf(" %[^\n]", calle_origen); 
            pos_origen = find_calle(HH->T,calle_origen,false);//posicion del origen en la ht
        }
        while(pos_destino_1 == UINT64_MAX){
            printf("- Localizacion de destino: ");
            scanf(" %[^\n]", calle_destino);  
            pos_destino_1 = find_calle(HH->T,calle_destino,false);//posiciones del destino en la ht
            pos_destino_2 = find_calle(HH->T,calle_destino,true);
        }
        printf("\n");
        break;
    }

    //ids de las calles
    size_t nodo_origen = T->head[pos_origen].id_nodo;
    size_t nodo_destino_1 = T->head[pos_destino_1].id_nodo;
    size_t nodo_destino_2 = T->head[pos_destino_2].id_nodo;


    printf("  +--------------------------------------------------+\n");
    printf("  | Resultados de navegacion:                        |\n");
    printf("  +--------------------------------------------------+\n\n");

    printf("- Camino Encontrado\n");
    printf("- Numero total de nodos: %zu\n", T->n_nodos);
    
    inicio = clock();
    size_t nodo_final = algoritmo(HH,nodo_origen, nodo_destino_1, nodo_destino_2);//algoritmo completo y regresa el nodo final al que se llegó
    fin = clock();
    tiempo_total_algoritmo = ((double)(fin - inicio)) / CLOCKS_PER_SEC;
    printf("- Tiempo de ejecucion del algoritmo: %.4f segundos\n", tiempo_total_algoritmo);
    printf("- Tiempo de carga de la tabla hash y heap: %.4f segundos\n\n", tiempo_total_fill);


    printf("  +--------------------------------------------------+\n");
    printf("  | Camino de navegacion:                            |\n");
    printf("  +--------------------------------------------------+\n\n");
    print_camino(HH,nodo_origen,nodo_final,calle_origen,calle_destino);//muestra camino regresivamente

    freeHeapash(HH);
    return (0);
}