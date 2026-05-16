#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <semaphore.h>

#define PORT 3333           //Numero de puerto
#define BUFFER 1024         //Tamaño maximo de mensajes
#define MAX_CUSTOMERS 100   //Cantidad maxima de clientes


//Estructura cliente
typedef struct
{
    int socket;         //Socket del cliente
    char user[50];      //Nombre del usuario
}Customer;


//Variables globales
extern Customer *customers[MAX_CUSTOMERS];  //Arreglo de clientes
extern int totalCustomers;                  //Total de usuarios conectados
extern sem_t mutexCustomers;                //Semaforo



//Funciones
void startServer(int socketServer);             //Iniciar el servidor - Recibe el numero del socket del servidor
void *handlerCustomer(void *arg);               //Mnejador de clientes - Apuntador a estructura de Customer

void addCustomer(Customer *customer);           //Añadir cliente - Cliente a registrar
void deleteCustomer(int socket);                //Eliminar cliente - Socket del cliente

void broadcastMessage(char message[], int socketOrigin);        //Enviar mensaje a todos, menos a uno mismo - Mensaje y socket del emisor

#endif