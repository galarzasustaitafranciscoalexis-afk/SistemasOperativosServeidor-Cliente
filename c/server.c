#include "../include/chat.h"

//Espera conexiones constantemente
void startServer(int socketServer)
{
    while(1)
    {
        Customer *customer = malloc(sizeof(Customer));  //Reserva espacio para el cliente

        if(customer == NULL)
        {
            continue;
        }

        struct sockaddr_in customerAddr;    //Direccion del cliente conectado
        socklen_t size = sizeof(customerAddr);  //Tamaño de la estructura
        customer->socket = accept(socketServer, (struct sockaddr*)&customerAddr, &size);    //Guarda la direccion del puerto usado por el usuario

        if(customer->socket < 0)
        {
            free(customer); //Liberacion de memoria en caso de error
            continue;
        }

        printf("New client connected!\n");

        pthread_t thread;   //Hilo para manejar a un cliente
        pthread_create(&thread, NULL, handlerCustomer, (void*)customer);    //Creacion del hilo - Ejecuta el manejador de cliente, estructura de cliente creada
        pthread_detach(thread); //Separacion del hilo del proceso principal
    }
}