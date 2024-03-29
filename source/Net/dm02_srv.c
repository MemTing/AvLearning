//服务器端实现思路：

//标准C库
#include <stdio.h>
#include <string.h>

//系统调用
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
	int	listenfd;
	
	listenfd = socket(PF_INET, SOCK_STREAM, 0);
	//listenfd = socket(AF_INET,  IPPROTO_TCP, 0);
	if (listenfd < 0)
	{
		printf("func socket() err\n");
		exit(0);
	}
	
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8001);
	//servaddr.sin_addr.s_addr = inet_addr(INADDR_ANY); //绑定服务器任意一个ip
	servaddr.sin_addr.s_addr = inet_addr("192.168.1.114"); //绑定服务器任意一个ip。 为什么127.0.0.1拒绝连接？
	//inet_aton("127.0.0.1", &servaddr.sin_addr);*/
	
	int  optval = 1;
	//每个级别SOL_SOCKET下，也有很多选项 不同的选择会有不同的结构
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{ 
		perror("setsockopt");
		exit(0);
	} 
	
 	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
 	{
 		perror("bind");
		exit(0);
 	}
 	
 	if (listen(listenfd, SOMAXCONN) < 0) 
 	{
 		perror("listen");
		exit(0);
 	}	
 	
 	int conn;
 	
 	struct sockaddr_in peeraddr;
 	socklen_t addrlen;
	memset(&peeraddr, 0, sizeof(struct sockaddr_in ));
	addrlen =  sizeof(struct sockaddr_in );
	
    
    while (1)
    {
		conn = accept(listenfd, (struct sockaddr *)&peeraddr, &addrlen );
		if (conn < 0)
		{
			perror("accept");
			exit(0);
		}
		printf("perradd:%s peerport:%d \n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
		
		//每来一个连接 启动一个子进程
		int pid = fork();
		if (pid == 0)
		{
			close(listenfd); //子进程不需要监听
			char recbuf[1024];
			while (1)
			{
				memset(recbuf, 0, sizeof(recbuf));
				//如果对方退出，捕捉
				int ret = read(conn, recbuf, sizeof(recbuf));
				if (ret == 0)
				{
					//如果在读的过程中，对方已经关闭，tcpip协议会返回一个0数据包
					printf("ret == 0 peer close退出\n");
					close(conn);
					break;  //去掉做实验
				}
				else if (ret < 0)
				{
					perror("ret<0");
					break;
				}
				fputs(recbuf, stdout); //服务端收到数据，打印屏幕
				write(conn, recbuf, ret); //服务端回发信息
			}
		}
		else if (pid > 0)
		{
			close(conn); //父进程不需要conn
		}
		else 
		{
			printf("创建进程失败\n");
			close(conn);
    		//close(listenfd);
			//exit(0);
		}

    }

    close(conn);
    close(listenfd);

	return 0;
}