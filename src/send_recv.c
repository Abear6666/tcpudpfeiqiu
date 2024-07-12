#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int send_file(char *pathname, char *ip ,int port);
int recv_file();
int recv_sharefile();
int bind_sharetcp_serv(unsigned short port);
extern int tcp_server_port ;
int   tcp_socket;
int tcp_share_socket;

//文件发送函数
int send_file(char *pathname, char *ip ,int port)
{
    //发送文件名与大小给服务器  file 文件名 文件大小
    int fd = open(pathname,O_RDWR);
        if(fd < 0)
        {
            perror("");
            return -1;
        }


    //1.创建客户端通信socket 
    int   tcp_socket1 = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_socket1 < 0 )
    {
        perror("");
    }

    //2.设置服务器信息
     struct sockaddr_in   addr; 
    addr.sin_family = AF_INET; //ipv4 
    addr.sin_port   = htons(port);//端口为 8686
    addr.sin_addr.s_addr = inet_addr(ip); //本地网卡地址

     //链接服务器
   int ret=connect(tcp_socket1,(struct sockaddr *)&addr,sizeof(addr));
       if(ret < 0)
       {
            perror("链接失败\n");
            return -1; 
       }else{
        printf("连接好友成功 准备发送文件\n");
       }

    //获取文件大小
    struct stat  file_size; 
    stat(pathname,&file_size); 

    char file_msg[1024]={0};
    //拼接协议  
    sprintf(file_msg,"file %s %ld",pathname,file_size.st_size); 

    //发送给服务器 
    int a = write(tcp_socket1,file_msg,strlen(file_msg));

    //等待服务器应答 
    char  req[1024]={0}; 
    read(tcp_socket1,req,1024); 
    if(strcmp(req,"get_file_msg") == 0)  //应答成功 
    {
        //发送文件内容 
        while (1)
        {
            //读取源文件数据 
            char  data[4096]={0}; 
            int size = read(fd,data,4096);
                if(size <= 0)
                {
                    printf("发送完毕\n");
                    break;
                }
        
            //发送到网络中
            write(tcp_socket1,data,size);
        }

    }
    //等待服务器接收完毕 
    bzero(req,1024); 
    read(tcp_socket1,req,1024); 

    // write(new_socet,"down_ok",strlen("down_ok"));
    if(strcmp(req,"down_ok") == 0)
    {
         printf("关闭所有链接\n");
         close(tcp_socket1);
         close(fd); 
    }
}

//绑定tcp服务器
int bind_tcp_serv(unsigned short port)
{
    //1.创建服务器socket 
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 使套接字sockfd关联的地址在套接字关闭后立即释放
    int on = 1;
    setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
    //2.绑定服务器信息  

     struct sockaddr_in   addr; 
      addr.sin_family = AF_INET; //ipv4 
      addr.sin_port   = htons(port);//端口为 6666
      addr.sin_addr.s_addr = INADDR_ANY; //本地所有网卡地址


   int ret=bind(tcp_socket,(struct sockaddr *)&addr,sizeof(addr));
       if(ret < 0)
       {
            perror("绑定失败\n");
            return -1; 
       }else{
        printf("服务器绑定成功\n");
       }

    //3.设置为监听模式 
        listen(tcp_socket,5);    
}
 
//接收文件
int recv_file()
{

         //4.接收客户端的连接请求 

        printf("等待客户端发送文件\n");
        int  new_socet = accept(tcp_socket,NULL,NULL);
        if(new_socet > 0)  //连接成功 
        {
            printf("开始接收………\n");
            //接收文件名+文件大小 
            char file_msg[1024]; //file 文件名 文件大小 
            read(new_socet,file_msg,1024);


            //获取文件名和文件大小  
            char file_name[1024];  
            int file_size; 
            if(strstr(file_msg,"file"))
            {
              sscanf(file_msg,"file %s %d",file_name,&file_size); 
            }else{
                printf("解析文件失败\n");
                close(new_socet); 
                //continue;
            }


            printf("对方发送的文件名 %s,文件大小 %d\n",file_name,file_size);
            
            //告诉发送端，已经得到了文件的信息  
            write(new_socet,"get_file_msg",strlen("get_file_msg"));

            //创建文件 
            int fd = open(file_name,O_RDWR|O_CREAT|O_TRUNC,0777);

            //下载大小  
            int down_size=0; 
            //不断接收数据 
            while (1)
            {
                //读取网络数据
                char data[4096]={0};
                int size = read(new_socet,data,4096); 

                down_size+=size; 

                //写入本地文件 
                write(fd,data,size);

                //判断是否下载完毕  
                if(down_size >= file_size)  
                {
                    printf("下载完毕\n");

                    //告诉发送端已经下载完毕，可以断开链接 
                    write(new_socet,"down_ok",strlen("down_ok"));

                    close(fd);

                    close(new_socet);
                   
                    
                    break;
                }else{
                    printf("下载进度 %d %%\n",down_size*100/file_size);
                }

            }
            
        }


       
}

//绑定接收共享文件的tcp服务器端口
int bind_sharetcp_serv(unsigned short port)
{
    //1.创建服务器socket 
    tcp_share_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 使套接字sockfd关联的地址在套接字关闭后立即释放
    int on = 1;
    setsockopt(tcp_share_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
    //2.绑定服务器信息  

     struct sockaddr_in   addr; 
      addr.sin_family = AF_INET; //ipv4 
      addr.sin_port   = htons(port);//端口为 6666
      addr.sin_addr.s_addr = INADDR_ANY; //本地所有网卡地址


   int ret=bind(tcp_share_socket,(struct sockaddr *)&addr,sizeof(addr));
       if(ret < 0)
       {
            perror("绑定失败\n");
            return -1; 
       }else{
        printf("服务器绑定成功\n");
       }

    //3.设置为监听模式 
        listen(tcp_share_socket,5);        

}

//接收共享文件
int recv_sharefile()
{
         //4.接收客户端的连接请求 


        printf("等待客户端发送文件\n");
        int  new_socet = accept(tcp_share_socket,NULL,NULL);
        if(new_socet > 0)  //连接成功 
        {
            printf("开始接收………\n");
            //接收文件名+文件大小 
            char file_msg[1024]; //file 文件名 文件大小 
            read(new_socet,file_msg,1024);


            //获取文件名和文件大小  
            char file_name[1024];  
            int file_size; 
            if(strstr(file_msg,"file"))
            {
              sscanf(file_msg,"file %s %d",file_name,&file_size); 
            }else{
                printf("解析文件失败\n");
                close(new_socet); 
                //continue;
            }


            printf("对方发送的文件名 %s,文件大小 %d\n",file_name,file_size);
            
            //告诉发送端，已经得到了文件的信息  
            write(new_socet,"get_file_msg",strlen("get_file_msg"));

            //创建文件 
            int fd = open(file_name,O_RDWR|O_CREAT|O_TRUNC,0777);

            //下载大小  
            int down_size=0; 
            //不断接收数据 
            while (1)
            {
                //读取网络数据
                char data[4096]={0};
                int size = read(new_socet,data,4096); 

                down_size+=size; 

                //写入本地文件 
                write(fd,data,size);

                //判断是否下载完毕  
                if(down_size >= file_size)  
                {
                    printf("下载完毕\n");

                    //告诉发送端已经下载完毕，可以断开链接 
                    write(new_socet,"down_ok",strlen("down_ok"));

                    close(fd);

                    close(new_socet);
                   // close(tcp_share_socket);
                   
                    
                    break;
                }else{
                    printf("下载进度 %d %%\n",down_size*100/file_size);
                }

            }
            
        }


       
}