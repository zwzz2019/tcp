#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#include "mysql.h"

#define MAXLINE    4096
#define SERV_PORT  9997    /*接收数据端口号（客户端）*/

int main(int argc, char** argv)
{
while(1){
      time_t timep;
      int    len,res;
      int    socket_fd, connect_fd,client_fd;
      char   ipv4[20],temperator[10],humidity[10],illuminance[10],co2[10],tvoc[10],wap[10];/*定义所有传感器数组*/
      struct sockaddr_in servaddr;     /* 服务器端网络地址结构体 */
      char    buf[MAXLINE],cmd[1024]={0};
      MYSQL *conn_ptr;

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
      }

while(1)
{
            /*建立数据库连接*/
            conn_ptr = mysql_init(NULL);
            if(!conn_ptr)
            {
              fprintf(stderr,"mysql_init failed\n");
              sleep(1);
              continue;
            }

            /*数据库连接参数设置*/
            conn_ptr = mysql_real_connect(conn_ptr,"localhost","root","123456","sensor_data",0,NULL,0);

            if(conn_ptr)
              printf("Connection database success\n");
            else
            {
              printf("Connection database failed\n");
              sleep(1);
              continue;
            }

            printf("waiting for client's connection......\n");
            /*阻塞直到有客户端连接，不然多浪费CPU资源*/
            if((connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) <0)
             {
                  printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
                  sleep(1);
                  continue;
             }
             printf("Tcp connected\n");

             /*start receive message*/
             while((len = recv(connect_fd, buf, MAXLINE, 0))>0)
             {
                   buf[len] = '\0';
                   printf("receive message from client: %s\n", buf);
                   int i=0,j=0;

                   time (&timep);
                   printf("%s",ctime(&timep));
                   /*分割传输数据*/
                   if(len>=22&&len<=24)
                   {
                           while(1)
                           {
                              if(buf[i]=='+')
                              {
                               i++;
                               wap[j]='\0';
                               break;
                              }
                              wap[j]=buf[i];
                              i++;
                              j++;
                           }
                           printf("%d\n", i);
                           j=0;
                           while(1)
                           {
                             if(i>len)
                             {
                              i++;
                              ipv4[j]='\0';
                              break;
                             }
                             ipv4[j]=buf[i];
                             i++;
                             j++;
                           }
                           printf("%d\n", i);
                           j=0;

                           /*生成数据库语句*/
                           sprintf(cmd, "insert into wap(time,ip,wap) values('%s','%s','%s')",ctime(&timep),ipv4,wap);
                           res = mysql_query(conn_ptr, cmd);

                           if(!res)  /*ret != 0 表示出错 */
                           {
                             printf("Inserted by %lu rows\n",(unsigned long)mysql_affected_rows(conn_ptr));
                           }
                           else  /*打印出错及相关信息*/
                           {
                             fprintf(stderr, "Insert error %d:%s\n", mysql_errno(conn_ptr), mysql_error(conn_ptr));
                             sleep(1);
                             continue;
                           }
                   }/*end if len<30*/

                   else if(len>=44&&len<=54)
                   {
                         while(1)
                         {
                            if(buf[i]=='+')
                            {
                             i++;
                             co2[j]='\0';
                             break;
                            }
                            co2[j]=buf[i];
                            i++;
                            j++;
                         }
                         printf("%d\n", i);
                         j=0;
                         while(1)
                         {
                           if(buf[i]=='+')
                           {
                            i++;
                            tvoc[j]='\0';
                            break;
                           }
                           tvoc[j]=buf[i];
                           i++;
                           j++;
                         }
                         printf("%d\n", i);
                         j=0;
                         while(1)
                         {
                           if(buf[i]=='+')
                           {
                            i++;
                            temperator[j]='\0';
                            break;
                           }
                           temperator[j]=buf[i];
                           i++;
                           j++;
                         }
                         printf("%d\n", i);
                         j=0;
                         while(1)
                         {
                           if(buf[i]=='+')
                           {
                            i++;
                            humidity[j]='\0';
                            break;
                           }
                           humidity[j]=buf[i];
                           i++;
                           j++;
                         }
                         printf("%d\n", i);
                         j=0;
                         while(1)
                         {
                           if(buf[i]=='+')
                           {
                            i++;
                            illuminance[j]='\0';
                            break;
                           }
                           illuminance[j]=buf[i];
                           i++;
                           j++;
                         }
                         printf("%d\n", i);
                         j=0;
                         while(1)
                         {
                           if(i>len)
                           {
                            i++;
                            ipv4[j]='\0';
                            break;
                           }
                           ipv4[j]=buf[i];
                           i++;
                           j++;
                         }
                         printf("%d\n", i);
                         j=0;

                         /*生成数据库语句*/
                         sprintf(cmd, "insert into sensor_data(time,ip,temperator,humidity,illuminance,co2,tvoc,wap) values('%s','%s','%scd','%s%%','%slux','%sppm','%sppb','0')",ctime(&timep),ipv4,temperator,humidity,illuminance,co2,tvoc);
                         res = mysql_query(conn_ptr, cmd);
                         if(!res)  /*ret != 0 表示出错 */
                         {
                           printf("Inserted by %lu rows\n",(unsigned long)mysql_affected_rows(conn_ptr));
                         }
                         else  /*打印出错及相关信息*/
                         {
                           fprintf(stderr, "Insert error %d:%s\n", mysql_errno(conn_ptr), mysql_error(conn_ptr));
                           sleep(1);
                           continue;
                         }
                   }/*end else if len>30*/
                   printf("insert message into database: %s\n", cmd);
             }/*end while receive*/
             close(connect_fd);
             mysql_close(conn_ptr);
}/*end while*/
close(socket_fd);
sleep(200);
}
return 0;
}
