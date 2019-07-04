#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE    4096
#define SERV_PORT  9998    /*接收数据端口号（服务器）*/
#define CLIE_PORT  9997    /*发送数据端口号（客户端）*/

int main(int argc, char *argv[])
{
  while(1){
        int    len,socket_fd, connect_fd,client_fd;
        struct sockaddr_in servaddr;     /* 服务器端网络地址结构体 */
        struct sockaddr_in clieaddr;     /* 客户端端网络地址结构体 */
        char    buf[MAXLINE],sendbuf[MAXLINE];

      /* 判断命令端输入的参数是否正确 */
       if( argc != 2)
       {
           printf("usage: ./client <ipaddress>\n");
           sleep(1);
           continue;
        }

        /*创建服务器端套接字--IPv4协议，面向连接通信，TCP协议*/
        if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
        {
            printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
            sleep(1);
            continue;
        }
        /*初始化*/
        memset(&servaddr, 0, sizeof(servaddr));/*数据初始化-清零 */
        servaddr.sin_family = AF_INET;  /*设置IPv4通信*/
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
        servaddr.sin_port = htons(SERV_PORT);//设置服务器端口为SERV_PORT

        /*将本地地址绑定到所创建的套接字上*/
        if( bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) <0)
        {
             printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
             sleep(1);
             continue;
        }

        /*开始监听是否有客户端连接*/
        if( listen(socket_fd, 10) <0)
        {
             printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
             sleep(1);
             continue;
        }/*end receive socket create*/


while(1)
{
       /* 创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
       if((client_fd=socket(AF_INET,SOCK_STREAM,0))<0)
        {
            perror("socket");
            sleep(1);
            continue;
        }
       /* 初始化 */
      memset(&clieaddr,0,sizeof(clieaddr)); /* 数据初始化-清零 */
      clieaddr.sin_family = AF_INET; /* 设置IPv4通信 */
      clieaddr.sin_port = htons(CLIE_PORT);/* 设置服务器端口号 */
      /* IP地址转换函数，将点分十进制转换为二进制 */
       if( inet_pton(AF_INET, argv[1], &clieaddr.sin_addr) <= 0)
        {
             printf("inet_pton error for %s\n",argv[1]);
             sleep(1);
             continue;
        }
       /* 将套接字绑定到服务器的网络地址上*/
      if( connect(client_fd, (struct sockaddr *)&clieaddr, sizeof(clieaddr))<0)
        {
            perror("connected failed");
            sleep(1);
            continue;
        }
        /*end send data socket creat*/

      printf("waiting for client's connection......\n");
       /*阻塞直到有客户端连接，不然多浪费CPU资源*/
      if((connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) <0)
       {
            printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
            sleep(1);
            continue;
       }
       printf("Tcp connected\n");
          /*接受客户端传过来的数据*/
            while((len = recv(connect_fd, buf, MAXLINE, 0))>0)
             {
               buf[len] = '\0';
               printf("receive message from client: %s\n", buf);
               if( send(client_fd, buf, strlen(buf), 0) < 0)
                 {
                    printf("send messaeg error: %s(errno: %d)\n", strerror(errno), errno);
                    sleep(1);
                    break;
                 }
             }/*end while receive*/
      close(client_fd);
}/*end while*/
close(socket_fd);
close(connect_fd);
sleep(230);
}
return 0;
}
