#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/types.h>
#include <linux/watchdog.h>

#define MAXLINE    1024
#define SERV_PORT  9998

int main(int argc, char *argv[])
        {
while(1){
            char sendbuf[MAXLINE];
            struct sockaddr_in servaddr;
            int client_sockfd;
            int rec_len;
            int fd, count_r,count_t,i;
            unsigned char buf[1],buff[27];  // the reading & writing buffer
            unsigned int Temperature[2],CO2[2],TVOC[2],Humidity[2],Illuminance[2],Wap[3];
            unsigned int co2,tvoc,wap;
            double tem,hum,illu;
            struct termios opt;       //uart  confige structure
            int sum,wdf,timeout;

            /* 判断命令端输入的参数是否正确 */
             if( argc < 2)
             {
                 printf("usage: ./client <ipaddress>\n");
                 exit(0);
              }

            if ((fd = open("/dev/ttyO1", O_RDWR)) < 0)
            {
                perror("UART: Failed to open the UART device:ttyO1.\n");
                sleep(1);
                continue;
            }
            tcgetattr(fd, &opt); // get the configuration of the UART
            // config UART
            opt.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
            // 9600 baud, 8-bit, enable receiver, no modem control lines
            opt.c_iflag = IGNPAR | ICRNL;
            // ignore partity errors, CR -> newline
            opt.c_iflag &= ~(INLCR | ICRNL); //不要回车和换行转换
            opt.c_iflag &= ~(IXON | IXOFF | IXANY);
            opt.c_oflag &= ~OPOST;
            //turn off software stream control
            opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
            //关闭回显功能,关闭经典输入 改用原始输入
            tcflush(fd,TCIOFLUSH);        // 清理输入输出缓冲区
            tcsetattr(fd, TCSANOW, &opt); // changes occur immmediately

            /*初始化看门狗*/

            if ((wdf = open("/dev/watchdog", O_RDWR)) < 0)
            {
                  printf("open dog false\n");
                  sleep(1);
                  continue;
            }
            ioctl(wdf, WDIOC_GETTIMEOUT, &timeout);
            printf("Default timeout: %d\n", timeout);

            timeout = 130;
            printf("Set timeout to %d\n", timeout);
            ioctl(wdf, WDIOC_SETTIMEOUT, &timeout);

            ioctl(wdf, WDIOC_GETTIMEOUT, &timeout);
            printf("New timeout: %d\n", timeout);


while (1) {
             /* 创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
             if((client_sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
              {
                  perror("socket");
                  sleep(1);
                  continue;
              }
             /* 初始化 */
            memset(&servaddr,0,sizeof(servaddr)); /* 数据初始化-清零 */
            servaddr.sin_family = AF_INET; /* 设置IPv4通信 */
            servaddr.sin_port = htons(SERV_PORT);/* 设置服务器端口号 */
            /* IP地址转换函数，将点分十进制转换为二进制 */
             if( inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
              {
                   printf("inet_pton error for %s\n",argv[1]);
                   sleep(1);
                   continue;
              }
             /* 将套接字绑定到服务器的网络地址上*/
            if( connect(client_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))<0)
              {
                  perror("connected failed");
                  sleep(1);
                  continue;
              }
              /*end send data socket create*/



            /* 循环发送接收数据，send发送数据，recv接收数据 */

          while(1)
           {
               buff[0]=9;
               for(int i=0;i<27;i++)
               {
                    if ((count_r = read(fd,buf,1))<0)
                        continue;
                    else if (count_r == 0)
                        continue;
                    else
                    {
                        buff[i]=buf[0];
                        if(buf[0]==0x63)
                        {
                          for(int a=0;a<27;a++)
                          {
                               if ((count_r = read(fd,buf,1))<0)
                                   continue;
                               else if (count_r == 0)
                                   continue;
                               else
                               {
                                   if(buf[0]==0xa)
                                   break;
                               }
                          }
                          i=0;
                        }/*end zigbee  signal process*/
                        if(i==26)
                        {
                            if(buff[26]!=0xa)
                            {
                              for(int a=0;a<27;a++)
                              {
                                   if ((count_r = read(fd,buf,1))<0)
                                       continue;
                                   else if (count_r == 0)
                                       continue;
                                   else
                                   {
                                       if(a>20)
                                       {
                                         if(buf[0]==0xa)
                                         break;
                                       }
                                   }
                              }
                              i=0;
                            }
                        }/*end error process*/
                    }
               }
               for(int i=0;i<27;i++)
               {
                 printf("%x ", buff[i]);
               }
               printf("\n");
               if(buff[0]==2&&buff[10]==0)
               {
                   Wap[2]=(unsigned int)(buff[8]*65536);
                   Wap[1]=(unsigned int)(buff[7]*256);
                   Wap[0]=(unsigned int)(buff[6]);
                   wap=Wap[0]+Wap[1]+Wap[2];
                   printf("wap:%d\n",wap);
                   sum = sprintf(sendbuf,"%d+",wap);
                   sum += sprintf(sendbuf + sum, "192.168.10.11:wap");
                   if( send(client_sockfd, sendbuf, strlen(sendbuf), 0) < 0)
                    {
                        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                        sleep(1);
                        break;
                    }
                    printf("send msg to server: %s\n",sendbuf);
               }

               if(buff[0]==0)
               {
                     CO2[0]=(unsigned int)(buff[6]*256);
                     CO2[1]=(unsigned int)(buff[7]);
                     co2=CO2[0]+CO2[1];
                     printf("co2:%d\n",co2);
                     sum = sprintf(sendbuf,"%d+",co2);
                     TVOC[0]=(unsigned int)(buff[8]*256);
                     TVOC[1]=(unsigned int)(buff[9]);
                     tvoc=TVOC[0]+TVOC[1];
                     printf("tvoc:%d\n",tvoc);
                     sum += sprintf(sendbuf + sum, "%d+",tvoc);
                     Temperature[0]=(unsigned int)(buff[14]*256);
                     Temperature[1]=(unsigned int)(buff[15]);
                     tem=(double)(Temperature[0]+Temperature[1]);
                     tem=tem/65536*175.72-46.85;
                     printf("tem:%.3lf\n",tem);
                     sum += sprintf(sendbuf + sum, "%.3lf+",tem);
                     Humidity[0]=(unsigned int)(buff[16]*256);
                     Humidity[1]=(unsigned int)(buff[17]);
                     hum=(double)(Humidity[0]+Humidity[1]);
                     hum=hum/65536*125-6;
                     printf("hum:%.3lf%%\n",hum);
                     sum += sprintf(sendbuf + sum, "%.3lf+",hum);
                     Illuminance[0]=(unsigned int)(buff[18]*256);
                     Illuminance[1]=(unsigned int)(buff[19]);
                     Illuminance[0]=Illuminance[0]+Illuminance[1];
                     Illuminance[1]=Illuminance[0]/4096;
                     Illuminance[0]=Illuminance[0]%4096;
                     illu=pow(2,Illuminance[1])*((double)(Illuminance[0]*0.01));
                     printf("illu:%.3lf\n",illu);
                     sum += sprintf(sendbuf + sum, "%.3lf+",illu);
                     sum += sprintf(sendbuf + sum, "192.168.10.11:437in");
                     if( send(client_sockfd, sendbuf, strlen(sendbuf), 0) < 0)
                      {
                          printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                          sleep(1);
                          break;
                      }
                      else
                      {
                        ioctl(wdf, WDIOC_KEEPALIVE, 0);
                      }
                      printf("send msg to server: %s\n",sendbuf);
                }

                else if(buff[0]==1)
                {
                     CO2[0]=(unsigned int)(buff[6]*256);
                     CO2[1]=(unsigned int)(buff[7]);
                     co2=CO2[0]+CO2[1];
                     printf("co2:%d\n",co2);
                     sum = sprintf(sendbuf,"%d+",co2);
                     TVOC[0]=(unsigned int)(buff[8]*256);
                     TVOC[1]=(unsigned int)(buff[9]);
                     tvoc=TVOC[0]+TVOC[1];
                     printf("tvoc:%d\n",tvoc);
                     sum += sprintf(sendbuf + sum, "%d+",tvoc);
                     Temperature[0]=(unsigned int)(buff[14]*256);
                     Temperature[1]=(unsigned int)(buff[15]);
                     tem=(double)(Temperature[0]+Temperature[1]);
                     tem=tem/65536*175.72-46.85;
                     printf("tem:%.3lf\n",tem);
                     sum += sprintf(sendbuf + sum, "%.3lf+",tem);
                     Humidity[0]=(unsigned int)(buff[16]*256);
                     Humidity[1]=(unsigned int)(buff[17]);
                     hum=(double)(Humidity[0]+Humidity[1]);
                     hum=hum/65536*125-6;
                     printf("hum:%.3lf%%\n",hum);
                     sum += sprintf(sendbuf + sum, "%.3lf+",hum);
                     Illuminance[0]=(unsigned int)(buff[18]*256);
                     Illuminance[1]=(unsigned int)(buff[19]);
                     Illuminance[0]=Illuminance[0]+Illuminance[1];
                     Illuminance[1]=Illuminance[0]/4096;
                     Illuminance[0]=Illuminance[0]%4096;
                     illu=pow(2,Illuminance[1])*((double)(Illuminance[0]*0.01));
                     printf("illu:%.3lf\n",illu);
                     sum += sprintf(sendbuf + sum, "%.3lf+",illu);
                     sum += sprintf(sendbuf + sum, "192.168.10.11:437out");
                     if( send(client_sockfd, sendbuf, strlen(sendbuf), 0) < 0)
                      {
                          printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                          sleep(1);
                          break;
                      }
                      else
                      {
                        ioctl(wdf, WDIOC_KEEPALIVE, 0);
                      }
                      printf("send msg to server: %s\n",sendbuf);
                }/*end else if*/

                else if(buff[0]==2&&buff[10]!=0)
                {
                     CO2[0]=(unsigned int)(buff[6]*256);
                     CO2[1]=(unsigned int)(buff[7]);
                     co2=CO2[0]+CO2[1];
                     printf("co2:%d\n",co2);
                     sum = sprintf(sendbuf,"%d+",co2);
                     TVOC[0]=(unsigned int)(buff[8]*256);
                     TVOC[1]=(unsigned int)(buff[9]);
                     tvoc=TVOC[0]+TVOC[1];
                     printf("tvoc:%d\n",tvoc);
                     sum += sprintf(sendbuf + sum, "%d+",tvoc);
                     Temperature[0]=(unsigned int)(buff[14]*256);
                     Temperature[1]=(unsigned int)(buff[15]);
                     tem=(double)(Temperature[0]+Temperature[1]);
                     tem=tem/65536*175.72-46.85;
                     printf("tem:%.3lf\n",tem);
                     sum += sprintf(sendbuf + sum, "%.3lf+",tem);
                     Humidity[0]=(unsigned int)(buff[16]*256);
                     Humidity[1]=(unsigned int)(buff[17]);
                     hum=(double)(Humidity[0]+Humidity[1]);
                     hum=hum/65536*125-6;
                     printf("hum:%.3lf%%\n",hum);
                     sum += sprintf(sendbuf + sum, "%.3lf+",hum);
                     Illuminance[0]=(unsigned int)(buff[18]*256);
                     Illuminance[1]=(unsigned int)(buff[19]);
                     Illuminance[0]=Illuminance[0]+Illuminance[1];
                     Illuminance[1]=Illuminance[0]/4096;
                     Illuminance[0]=Illuminance[0]%4096;
                     illu=pow(2,Illuminance[1])*((double)(Illuminance[0]*0.01));
                     printf("illu:%.3lf\n",illu);
                     sum += sprintf(sendbuf + sum, "%.3lf+",illu);
                     sum += sprintf(sendbuf + sum, "192.168.10.11:435in");
                     if( send(client_sockfd, sendbuf, strlen(sendbuf), 0) < 0)
                      {
                          printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                          sleep(1);
                          break;
                      }
                      else
                      {
                        ioctl(wdf, WDIOC_KEEPALIVE, 0);
                      }
                      printf("send msg to server: %s\n",sendbuf);
                }/*end else if*/
            }/*end while receive*/
    close(client_sockfd);
    write(fd, "V", 1);
    close(fd);
    printf("Close watchdog!");
}/*end tcp while*/
sleep(250);
}/*end while*/
    return 0;
}/*end main*/
