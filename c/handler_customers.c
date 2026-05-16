#include "../include/chat.h"


Customer *customers[MAX_CUSTOMERS]; //Arreglo global de clientes - Usuarios conectados
int totalCustomers = 0; //Total de ususarios conectados
sem_t mutexCustomers;   //Semaforo

//Agregar cliente
void addCustomer(Customer *customer) //Cliente a registrar
{
    sem_wait(&mutexCustomers);  //Bloquea el arreglo de customers

    if(totalCustomers < MAX_CUSTOMERS)
    {
        customers[totalCustomers] = customer;   //Añade al cliente a la lista
        totalCustomers++;   //Suma un cliente
    }

    sem_post(&mutexCustomers);  //Desbloque el arreglo
}

//Eliminar cliente
void deleteCustomer(int socket) //Nuemero de socket
{
    sem_wait(&mutexCustomers);  //Bloquea el arreglo de customers
    int i;

    for(i = 0; i < totalCustomers; i++) //Itera sobre el arreglo
    {
        if(customers[i]->socket == socket)  //Comparacion de socket de usuario a desconectar
        {
            int j;
            for(j = i; j < totalCustomers - 1; j++) //Reordenamiento de la lista de customers
            {
                customers[j] = customers[j + 1];
            }
            totalCustomers--;

            break;
        }
    }
    sem_post(&mutexCustomers);  //Desbloquea el arreglo de customers
}

//Puente para el envio del mensaje a los usuarioa
void broadcastMessage(char message[], int socketOrigin) //Mensaje - Socket de origen
{
    sem_wait(&mutexCustomers);  //Bloquea el arreglo de clientea
    int i;

    for(i = 0; i < totalCustomers; i++) //Recorre el arreglo de cutomers
    {
        if(customers[i]->socket != socketOrigin)    //Comparacion sockets destino vs socket remitente
        {
            send(customers[i]->socket, message, strlen(message), 0);    //Envia del mensaje atraves de los sockets
        }
    }
    sem_post(&mutexCustomers); //Desbloquea el arreglo de customers
}

//Manejador de clientes
void *handlerCustomer(void *arg)    //Apuntador a estructura de customers
{
    Customer *customer = (Customer*)arg;    //Cliente asociado al hilo
    char buffer[BUFFER + 1];    //Almaenar el mensaje

    if(recv(customer->socket, customer->user, sizeof(customer->user), 0) <= 0) //Recivir el nombre del usuario
    {
        close(customer->socket);    //Cierra el socket
        free(customer); //Libera el espacio en memoria 
        pthread_exit(NULL); //Sale del programa
    }

    customer->user[strcspn(customer->user, "\n")] = '\0';   //Asignacion del nombre del usuario
    addCustomer(customer);  //Añade al usuario
    snprintf(buffer, sizeof(buffer), "SERVER: %s entered the chat!\n", customer->user); //Vienvenida al chat

    broadcastMessage(buffer, customer->socket); //Mandar el mensaje

    while(1)    //Espera
    {
        int bytes;  
        bytes = recv(customer->socket, buffer, BUFFER, 0);  //Mensaje recibido en bytes

        if(bytes <= 0)
        {
            break;
        }

        buffer[bytes] = '\0';   //Final de la cadena
        broadcastMessage(buffer, customer->socket); //Enviar el mensaje
    }

    snprintf(buffer, sizeof(buffer), "SERVER: %s left the chat!\n", customer->user);    //Mensaje de confirmacion de recivido por el servidor
    broadcastMessage(buffer, customer->socket); //Notificar del envio del mensaje
    deleteCustomer(customer->socket);   //Eliminacion del usuario
    close(customer->socket);    //Cerrar la comunicacion
    free(customer); //Liberar la memoria 
    pthread_exit(NULL); //Salir del proceso
}