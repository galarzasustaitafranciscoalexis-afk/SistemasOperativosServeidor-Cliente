#include "../include/chat.h"


int main()
{
    int socketServer;   //Varibale para almacenar el numero del socket
    int option = 1; //Para reutilizacion del puerto
    struct sockaddr_in serverAddr;  //Almacena la direccion del servidor
    socketServer = socket(AF_INET, SOCK_STREAM, 0); //Creacion del socket - IPV4, Conexion estable, Protocolo

    if(socketServer < 0)    //Verificacion de la creacion del server
    {
        perror("Error socket");
        exit(1);
    }

    setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));    //Configuracion del socket - Socket a configurar, Nivel del socket, reutilizacion del puerto, bandera, tamaño de la bandera 

    serverAddr.sin_family = AF_INET;    //Familia de IPV4
    serverAddr.sin_addr.s_addr = INADDR_ANY;    //Escuchar wifi, ethernet, loopback
    serverAddr.sin_port = htons(PORT);  //Puerto de escucha - Conversion  delittle endia a big endian

    if(bind(socketServer, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)   //Busqueda de los clientes - Socket, direccion, tamaño
    {
        perror("Error bind");   //Mensaje de error de busqueda
        exit(1);
    }

    listen(socketServer, 10);   //Escuchar - Maximas conexiones escuchadas por ejecucion 10
    sem_init(&mutexLog, 0, 1);  //Inicializacion del semaforo del archivo log
    sem_init(&mutexCustomers, 0, 1);    //Iicializacion del semaforo de clientes

    printf("Server running...\n");  //Mensaje de confirmacion encendido
    startServer(socketServer);  //Iniciacion del servidor - Socket del serveidor
    sem_destroy(&mutexLog);     //Liberar los recursos
    sem_destroy(&mutexCustomers);   //Liberar los recursos
    close(socketServer);    //Cerrar el puerto del server

    return 0;
}