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

int main()
        {
          struct sockaddr_in servaddr;
          int client_sockfd;
          int rec_len;
          int fd, count_r,count_t,i;
          unsigned char buf[1],buff[27];  // the reading & writing buffer
          unsigned int Temperature[2],CO2[2],TVOC[2],Humidity[2],Illuminance[2],Wap[2];
          unsigned int co2,tvoc,wap;
          double tem,hum,illu;
          struct termios opt;       //uart  confige structure
          int sum,wdf,timeout;
            if ((fd = open("/dev/ttyO1", O_RDWR)) < 0)
            {
                perror("UART: Failed to open the UART device:ttyO1.\n");
                return -1;
            }
            tcgetattr(fd, &opt); // get the configuration of the UART
            opt.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
            opt.c_iflag = IGNPAR | ICRNL;
            opt.c_iflag &= ~(INLCR | ICRNL); //不要回车和换行转换
            opt.c_iflag &= ~(IXON | IXOFF | IXANY);
            opt.c_oflag &= ~OPOST;
            opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
            //关闭回显功能,关闭经典输入 改用原始输入
            tcflush(fd,TCIOFLUSH);        // 清理输入输出缓冲区
            tcsetattr(fd, TCSANOW, &opt); // changes occur immmediately
            int sum,wdf,timeout;
            while(1)
             {
               buff[0]=9;
               for(int i=0;i<27;i++)
               {
                    if ((count_r = read(fd,buf,1))<0)
                        break;
                    else if (count_r == 0)
                        break;
                    else
                    {
                        buff[i]=buf[0];
                        printf("%x ", buff[i]);
                    }
               }
               printf("\n");

               if(buff[0]==2)
               {
                   Wap[1]=(unsigned int)(buff[7]*256);
                   Wap[0]=(unsigned int)(buff[6]);
                   wap=Wap[0]+Wap[1];
                   printf("wap:%d\n",wap);
                   sum = sprintf(sendbuf,"%d+",wap);
                   sum += sprintf(sendbuf + sum, "192.168.10.11:wap");
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
                     sum += sprintf(sendbuf + sum, "192.168.10.11:indoor");
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
                     sum += sprintf(sendbuf + sum, "192.168.10.11:outdoor");
                      printf("send msg to server: %s\n",sendbuf);
              }
            return 0;
}
