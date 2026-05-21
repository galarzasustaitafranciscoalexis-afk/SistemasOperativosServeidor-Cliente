#include "../include/chat.h"

//Recive el mensaje
void *receiveMessage(void *arg) //Puntero generico de los argumentos
{
    int socketClient = *((int*)arg);    //Conversion del apuntador a int
    char buffer[BUFFER + 1];    //Almacena los mensajes recibidos

    while(1)    //Eserando
    {
        int bytes;
        bytes = recv(socketClient, buffer, BUFFER, 0);  //Bytes del mensaje recibido

        if(bytes <= 0)
        {
            break;
        }

        buffer[bytes] = '\0';   //Conversion a cadena
        printf("%s", buffer);   //Impresion del mensaje
    }
    pthread_exit(NULL); //Finalizacion del trabajo del hilo
}



int main()
{
    int socketClient;   //Descriptor del socket
    struct sockaddr_in serverAddr;  //Direccion del servidor
    char user[50];  //Almacenara el nombre del usuario
    char message[BUFFER];   //Tamaño maximo de los mensaje
	

    socketClient = socket(AF_INET, SOCK_STREAM, 0); //Creacion del socket - IPV4, Conexion estable, Protocolo

    if(socketClient < 0)
    {
        perror("Error socket"); //Mensaje de error
        exit(1);
    }

    serverAddr.sin_family = AF_INET;    //Familia de IPV4
    serverAddr.sin_port = htons(PORT);  //Conversion al formato de red
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);  //Conversion de ip a texto - IPV4, localhost, destino

    if(connect(socketClient, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)    //Solicitacion de la conexion con el servidor
    {
        perror("Error connect");    //Mensaje de error 
        exit(1);
    }

	while(1) //Ciclo de verificacion de nombre de usuario
	{
		printf("Username: ");   //Peticion de nombre de usuario
		fgets(user, sizeof(user), stdin);   //Lectura del nombre
		send(socketClient, user, strlen(user), 0);  //Envio de datos al servidor - Socket propio. nombre de usuaion longitud, bandera
		
		int bytes = recv(socketClient, message, BUFFER, 0); //Respuesta del servidor
		
		if(bytes <= 0) //Si se desconecto del servidor...
		{
			printf("Disconnected from server\n");
			close(socketClient);
			return 1;
		}
		
		message[bytes] = '\0'; //Terminador de cadena
		

		if(strstr(message, "Error!") != NULL) // Si el nombre esta repetido
		{
			printf("%s\n", message);
		}
		else
		{
			printf("%s\n", message); //Si no lo esta, se sale del ciclo
			break;
		}
		
	}
	
	pthread_t thread;   //Hilo
	pthread_create(&thread, NULL, receiveMessage, &socketClient);   //Creacion del hilo que recibira mensajes - Hilo,NULL, programa, destino 
	
	
    while(1)    //Espera
    {
        fgets(message, sizeof(message), stdin); //Captura mensaje a enviar
        send(socketClient, message, strlen(message), 0);    //Manda el mensaje al servidor
    }

    close(socketClient);    //Cierra la conexion
    return 0;
}
