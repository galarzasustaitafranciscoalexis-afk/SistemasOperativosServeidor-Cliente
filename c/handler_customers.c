#include "../include/chat.h"


Customer *customers[MAX_CUSTOMERS]; //Arreglo global de clientes - Usuarios conectados
int totalCustomers = 0; //Total de ususarios conectados
sem_t mutexCustomers;   //Semaforo de clientes
sem_t mutexLog;     //Semaforo de documento
FILE *logFile = NULL;   //Archivo log
char logName[100];   //Nombre del archivo

//Creacion del archivo 
void createLog()
{
    time_t currentTime;     //Varible para almacenar el tiempo actual del sistema
    struct tm *timeInfo;    //Estructura de tiempo - Año, mes,dia, hora, minuto, segundo

    time(&currentTime); //Hora actual del sistema
    timeInfo = localtime(&currentTime);     //Conversion a una lectura mas facil

    strftime(logName, sizeof(logName), "%Y%m%d_%H%M%S.txt", timeInfo);  //Construccion de cadena - Destino, tamaño, formato recivido, fuente de donde biene

    logFile = fopen(logName, "a");  //Abri/Crear archivo - Modo de adicion
}

//Escribir en el archivo
void writeLog(char message[])
{
    sem_wait(&mutexLog);    //Bloquear acceso al archivo

    if(logFile != NULL) //Verificacion de archivo existente
    {
        fprintf(logFile, "%s", message);    //Escritura en el archivo - Destino, formato, mensaje
        fflush(logFile);    //uerza escritura inmediata
    }

    sem_post(&mutexLog);     //Desbloque el acceso al archivo
}

//Cerrar el archivo log
void closeLog()
{
    sem_wait(&mutexLog);    //Bloquea acceso al archivo 

    if(logFile != NULL) //Verificacion de archivo existente
    {  
        fclose(logFile);//Cerrar el archivo
        logFile = NULL; //Reiniciar el apuntador
    }

    sem_post(&mutexLog);  //Desbloquea el acceso al archivo
}

//Agregar cliente
void addCustomer(Customer *customer) //Cliente a registrar
{
    sem_wait(&mutexCustomers);  //Bloquea el arreglo de customers

    if(totalCustomers == 0)
    {
        createLog();    //Crecion del archivo log
    }

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

            if(totalCustomers == 0)
            {
                closeLog();
            }

            break;
        }
    }
    sem_post(&mutexCustomers);  //Desbloquea el arreglo de customers
}

//Comparar nombres y buscar repetidos
int compareRepeated(char *name) //Nombre de usuario
{
	sem_wait(&mutexCustomers);  // Bloquear el arreglo de customers
	int exists = 0;
	int i;
	
	for(i = 0; i < totalCustomers; i++)
	{
		if(strcmp(customers[i]->user, name) == 0) //Compara los nombres en busca de alguna coincidencia
		{
			exists = 1;  // El nombre ya existe
			break;
		}
	}
	
	sem_post(&mutexCustomers);  // Desbloquear el arreglo
	return exists;
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
    char buffer[BUFFER + 1];    //Almacenar el mensaje
	int nameAccepted = 0; //Comprobar si el nombre fue aceptado

	while(!nameAccepted) //Mientras el nombre no sea aceptado...
	{
		if(recv(customer->socket, customer->user, sizeof(customer->user), 0) <= 0) //Recibir el nombre del usuario
		{
			close(customer->socket);    //Cierra el socket
			free(customer); //Libera el espacio en memoria
			pthread_exit(NULL); //Finaliza el hilo actual
		}

		customer->user[strcspn(customer->user, "\n")] = '\0';   //Asignacion del nombre del usuario
	
		if(compareRepeated(customer->user)) //Si el nombre esta repetido...
		{
			snprintf(buffer, sizeof(buffer), 
					 "SERVER: Error! Someone is already using this username.\n Enter a different username or press Ctrl+C to exit", customer->user); //Mensaje de error
			send(customer->socket, buffer, strlen(buffer), 0); //Mandar el mensaje de error al cliente
			printf("Name duplicated: %s\n", customer->user); //Manda mensaje de error al server
		}
		else
		{
			nameAccepted = 1;  // Se acepto el nombre
			snprintf(buffer, sizeof(buffer), "SERVER: Welcome to the chat!\n"); //Mensaje de confirmacion
			send(customer->socket, buffer, strlen(buffer), 0); //Se envia el mensaje
		}
	}
	
    addCustomer(customer);  //Añade al usuario
    snprintf(buffer, sizeof(buffer), "SERVER: %s entered the chat!\n", customer->user); //Bienvenida al chat
    writeLog(buffer);   //Escritura en el archivo antes de mandar el mensaje
    broadcastMessage(buffer, customer->socket); //Mandar el mensaje

    while(1)    //Espera mensajes del cliente
    {
        int bytes;  //Cantidad de bytes recibidos
        char messageComplete[BUFFER + 100];   //Mensaje completo con nombre del usuario
        bytes = recv(customer->socket, buffer, BUFFER, 0);  //Mensaje recibido en bytes

        if(bytes <= 0) //Verifica desconexion del cliente
        {
            break;
        }

        buffer[bytes] = '\0';   //Final de la cadena
		if(strcmp(buffer,"bye\n") == 0) //Comparacion de mensaje
		{
			break;
		}
        snprintf(messageComplete, sizeof(messageComplete), "%s: %s", customer->user, buffer); //Construccion del mensaje completo
        writeLog(messageComplete); //Escritura en el archivo
        broadcastMessage(messageComplete, customer->socket); //Enviar el mensaje
    }

    snprintf(buffer, sizeof(buffer), "SERVER: %s left the chat!\n", customer->user);    //Mensaje de salida del cliente
    writeLog(buffer);   //Escritura en el archivo
    broadcastMessage(buffer, customer->socket); //Notificar desconexion
    deleteCustomer(customer->socket);   //Eliminacion del usuario
    close(customer->socket);    //Cerrar la comunicacion
    free(customer); //Liberar la memoria
    pthread_exit(NULL); //Finaliza el hilo actual
}
