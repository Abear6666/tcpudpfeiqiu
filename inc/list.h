#ifndef LIST_H
#define LIST_H

struct node
{
	char name[50];
	char ip[50];
   	int socket; //描述符
	char signature[1024];//个性签名
	char pathname[512];//共享文件路径
	char share_file[2048];//共享文件夹下的所有文件
	struct node *next;
};
void Insert_Node(struct node *head, int socket);
void Insert_Node_tail(struct node *head, int socket, char *name, char *ip, char *sign,char *share_path, char *share_file);
char * FindNodes(struct  node *pos, char *name);
void Traver_Nodes(struct  node *head);
int Change_Node(struct  node *head, int socket,int socket1);
int Remove_Node(struct  node *head, char *name);
void Traver_sharepath_Nodes(struct  node *head);
struct node* Find_IP_Nodes(struct  node *pos, char *IP);

#endif


