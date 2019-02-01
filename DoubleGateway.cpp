#include "stdafx.h"
#include "RecvAndSend.h"
#define MAX_LINE 64000
#define MTU 1460
#define UDP_LOCAL_PORT 1200
#define TCP_LOCAL_PORT 1201
#define LISTENQ  10

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Please input the ip address port!\n");
		exit(1);
	}
	/*int pid;
	pid = fork();*/
	struct sockaddr_in Tcp_server_addr,Udp_server_addr, Udp_pair_addr, Tcp_pair_addr;
	int Tcplistenfd, Udplistenfd, Tcpconnfd, maxfd;
	int  Udpclientfd;
	socklen_t Udp_pair_addr_len = sizeof(struct sockaddr), Tcp_pair_addr_len = sizeof(struct sockaddr);
	char Tcprecvbuf[MAX_LINE], Udprecvbuf[MAX_LINE];
	int ErrorCode;
	fd_set allfds, rfds;
	bool Is_connected;

#ifdef _WIN32
	//start up Winsocket
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	Tcpconnfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	Tcplistenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	Udplistenfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if ((Tcplistenfd == -1)||(Udplistenfd == -1))
	{
		printf("socket create error!");
	}
	memset(&Tcp_server_addr,0, sizeof(struct sockaddr_in));
	memset(&Udp_server_addr, 0, sizeof(struct sockaddr_in));
	memset(&Udp_pair_addr, 0, sizeof(struct sockaddr_in));
	//Set local tcp listening address
	Tcp_server_addr.sin_family = AF_INET;
	Tcp_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Tcp_server_addr.sin_port = htons(TCP_LOCAL_PORT);
	//Set local udp listening address
	Udp_server_addr.sin_family = AF_INET;
	Udp_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Udp_server_addr.sin_port = htons(UDP_LOCAL_PORT);
	//Set the address of Destination
	Udp_pair_addr.sin_family = AF_INET;
	Tcp_pair_addr.sin_family = AF_INET;
	

	if (bind(Tcplistenfd, (LPSOCKADDR)&Tcp_server_addr, sizeof(Tcp_server_addr)) == -1)
	{
		printf("bind TCP error!\n");
		perror("call to bind");
	}

	if (bind(Udplistenfd, (LPSOCKADDR)&Udp_server_addr, sizeof(Udp_server_addr)) == -1)
	{
		printf("bind UDP error!\n");
		perror("call to bind");
	}
	listen(Tcplistenfd, LISTENQ);

	FD_ZERO(&allfds);
	FD_SET(Tcplistenfd, &allfds);
	FD_SET(Udplistenfd, &allfds);

	printf("The gateway server is running....\n");
	maxfd = Tcplistenfd>Udplistenfd?Tcplistenfd:Udplistenfd;
	//Start listening
	while (1) 
	{
		printf("Into select loop\n");
		rfds = allfds;
		select(maxfd + 1, &rfds, NULL, NULL, NULL);
		//Tcp SYN comes, means the first situation
		if (FD_ISSET(Tcplistenfd, &rfds))	
		{
			Tcpconnfd = accept(Tcplistenfd, (LPSOCKADDR)&Tcp_pair_addr, &Tcp_pair_addr_len);
			printf("connected with sender!\n");
			if (Tcpconnfd>maxfd)
			{ maxfd = Tcpconnfd; }
			else
			{ maxfd = Tcplistenfd; }                   	
			FD_SET(Tcpconnfd, &allfds);
			Is_connected = true;
			Udp_pair_addr.sin_addr.s_addr = inet_addr(argv[1]);
			Udp_pair_addr.sin_port = htons(atoi(argv[2]));
		}
		//If the Tcp connection is established
		if (FD_ISSET(Tcpconnfd, &rfds))
		{
			memset(Tcprecvbuf, 0, MAX_LINE);
			
			ErrorCode = recv(Tcpconnfd, Tcprecvbuf, MAX_LINE, 0);
			printf("response data from Apache!Data is:\n%s\n",Tcprecvbuf);

			if (ErrorCode == -1)
			{
				perror("fail to receive");
				send_reliable(Udplistenfd, "exit", 4, (struct sockaddr *)&Udp_pair_addr, Udp_pair_addr_len);
				CLOSE(Tcpconnfd);
				FD_CLR(Tcpconnfd, &allfds);
				Is_connected = false;
				continue;
			}
			if(strcmp(Tcprecvbuf, "") == 0)
			{
				printf("empty response, close tcp connection!\n");
				send_reliable(Udplistenfd, "exit", 4, (struct sockaddr *)&Udp_pair_addr, Udp_pair_addr_len);
				CLOSE(Tcpconnfd);
				FD_CLR(Tcpconnfd, &allfds);
				Is_connected = false;
				continue;
			}
			printf("tcp recv, ready to send as udp!\n");
			//revise the pack to send to host
			// bool start_override = false;
			// int j = 0, d_value = 0;
		 //    for(int i=0; i <= strlen(Tcprecvbuf); i++)
		 //    {
		 //    	if(memcmp(Tcprecvbuf + i, "Host", 4) == 0)
		 //    	{
		 //    		for(j = 0; j<strlen(Tcprecvbuf + i + 6); j++)
		 //    		{
		 //    			if(memcmp(Tcprecvbuf + i + 6 + j, "User-Agent", 10) == 0)
		 //    			{
		 //    				printf("j: %d", j);
		 //    				break;
		 //    			}
		 //    		}
		 //    		d_value = j-18;
		 //    		printf("differece: %d\n", d_value);
		 //    		memcpy(Tcprecvbuf + i + 6, "192.168.1.110:80", 16);
		 //            start_override = true;
		 //            i = i + 6 + 16 + d_value;
		 //    	}
		 //    	if(start_override == true)
		 //    	{
		 //    		Tcprecvbuf[i-2] = Tcprecvbuf[i];
		 //    	}
		  //  }
		    //divide package into MTUs
			int index = 0;
			// while (strlen(Tcprecvbuf + index) > MTU)
			// {
			// 	printf("Over length, need to devide into smaller pack!\n");
			// 	ErrorCode = send_reliable(Udplistenfd, Tcprecvbuf + index, MTU, (struct sockaddr *)&Udp_pair_addr, Udp_pair_addr_len);
			// 	if (ErrorCode == -1) {
			// 		perror("fail to reply\n");
			// 		exit(1);
			// 	}
			// 	index += MTU;
			// }
			
			ErrorCode = send_reliable(Udplistenfd, Tcprecvbuf + index, strlen(Tcprecvbuf + index), (struct sockaddr *)&Udp_pair_addr, Udp_pair_addr_len);
			if (ErrorCode == -1) {
				perror("fail to reply\n");
				exit(1);
			}

		}
		//Udp packet comes
		if (FD_ISSET(Udplistenfd, &rfds))
		{
			printf("enter udp process\n");
			Tcp_pair_addr.sin_addr.s_addr = inet_addr(argv[1]);
			Tcp_pair_addr.sin_port = htons(atoi(argv[2]));

			if(recv_reliable(Udplistenfd, Udprecvbuf, MTU, (struct sockaddr *)&Udp_pair_addr, &Udp_pair_addr_len) == -1)
			{
				printf("Pack repeated or recv error\n");
				continue;
			}
			if(strcmp(Udprecvbuf, "exit") == 0)
			{
				CLOSE(Tcpconnfd);
				FD_CLR(Tcpconnfd, &allfds);
				Is_connected = false;
				continue;
			}
			if (Is_connected == false)
			{
				if (connect(Tcpconnfd, (LPSOCKADDR)&Tcp_pair_addr, sizeof(struct sockaddr_in)) < 0)
				{
					perror("call to connect");
					exit(1);
				}
				if (Tcpconnfd>maxfd)
				{ maxfd = Tcpconnfd; }
				else
				{ maxfd = Tcplistenfd; }
				FD_SET(Tcpconnfd, &allfds);
				Is_connected = true;
				printf("connented to receiver!\n");
				if (send(Tcpconnfd, Udprecvbuf, strlen(Udprecvbuf) + 1, 0) < 0) 
				{
					perror("call to send");
					exit(1);
				}
				printf("sent first pack to receiver!\n");
			}
			else 
			{
				if (send(Tcpconnfd, Udprecvbuf, strlen(Udprecvbuf) + 1, 0) < 0)
				{
					perror("call to send");
					exit(1);
				}
				printf("send pack to receiver!\n");
			}
		}
	}
#ifdef _WIN32
	WSACleanup();
#endif
    return 0;

}

