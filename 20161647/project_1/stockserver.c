/*
 stock server
 concurrent based by event
   */
#include "csapp.h"
#include <time.h>

// binary search tree
#define MAXSTOCK 10

typedef struct _Node{
	int ID;
	int left_stock;
	int price;
	int readcnt;
//	sem_t mutex; not for event based
	struct _Node* lchild;
	struct _Node* rchild;
}NODE;

int IDarr[MAXSTOCK]; // maximum number of stock type
int len = 0; // length of array

// Binary search tree
NODE* root = NULL;
// Insert node, 
// search tree, tree search for buy, tree search for sell
void init_tree(const char*filename);
NODE* tree_insert(NODE*root, int ID, int stock_left, int price);
NODE* tree_search(NODE*root, int key);
void tree_buy(NODE*tmp, int num);
void tree_sell(NODE*tmp, int num);

// about event based
typedef struct{
	int maxfd;
	fd_set read_set;
	fd_set ready_set;
	int nready;
	int maxi;
	int clientfd[FD_SETSIZE];
	rio_t clientrio[FD_SETSIZE];
}pool;

//clock_t start, end;

int byte_cnt=0;

void init_pool(int listenfd, pool*p);
void add_client(int connfd, pool*p);
void check_clients(pool*p);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */
    char client_hostname[MAXLINE], client_port[MAXLINE];
	static pool pool;

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }

	// make stock tree
	init_tree("stock.txt");

    listenfd = Open_listenfd(argv[1]); // listenfd to accept
	init_pool(listenfd, &pool);

    while (1) {
	//	start = clock();
		pool.ready_set = pool.read_set;
		pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

		if(FD_ISSET(listenfd, &pool.ready_set)){
			clientlen = sizeof(struct sockaddr_storage); 
			connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
	        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                    client_port, MAXLINE, 0);
	        printf("Connected to (%s, %s)\n", client_hostname, client_port);
			add_client(connfd, &pool);
		}
		check_clients(&pool);
	//	end = clock();
	//	printf("start = %d end = %d time = %f\n", start, end, (float)(end-start)/CLOCKS_PER_SEC);
    }
    exit(0);
}
void init_pool(int listenfd, pool*p)
{
	int i;
	p->maxi=-1;
	for(i=0;i<FD_SETSIZE;i++)
		p->clientfd[i]=-1;
	p->maxfd = listenfd;
	FD_ZERO(&p->read_set);
	FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool*p)
{
	int i;
	p->nready--;
	for(i=0;i<FD_SETSIZE;i++){
		if(p->clientfd[i]<0){
			p->clientfd[i]=connfd;
			Rio_readinitb(&p->clientrio[i], connfd);
			FD_SET(connfd,&p->read_set);
			if(connfd>p->maxfd)
				p->maxfd=connfd;
			if(i>p->maxi)
				p->maxi=i;
			break;
		}
	}
	if(i==FD_SETSIZE)
		app_error("add client error: Too many clients");
}
void check_clients(pool*p)
{
	int i, connfd, n;
	int key, value; // arguments to search in tree
	char buf[MAXLINE];
	char cmd[MAXLINE], fcmd1[MAXLINE], fcmd2[MAXLINE];
	char copy[MAXLINE], *tmp;
	NODE* Ntmp;
	rio_t rio;

	int fd;

	for(i=0;(i<=p->maxi)&&(p->nready>0);i++)
	{
		connfd = p->clientfd[i];
		rio = p->clientrio[i];

		if((connfd>0)&&(FD_ISSET(connfd, &p->ready_set))){
			p->nready--;
			if((n=Rio_readlineb(&rio, buf, MAXLINE))>0){
				byte_cnt+=n;
				printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);

				// action for request
				if(!strncmp(buf, "show", 4)){
					fd = Open("stock.txt", O_RDONLY, S_IRUSR);
					rio_readn(fd, cmd, sizeof(cmd));
					Close(fd);
				}
				else if(!strncmp(buf, "buy", 3)){
					strcpy(copy,buf);
					tmp = strtok(copy, " ");
					tmp = strtok(NULL, " ");
					key = atoi(tmp);
					tmp = strtok(NULL, " ");
					value = atoi(tmp);
					if((Ntmp = tree_search(root, key))!=NULL){
						if(Ntmp->left_stock<value){
							strcpy(cmd, "Not enough left stock\n");
						}
						else{
							fd = Open("stock.txt", O_RDWR|O_TRUNC, S_IWUSR);
							strcpy(cmd, "[buy] success\n");
							fcmd2[0] = '\0';
							tree_buy(Ntmp, value);
							for(int i=0;i<len;i++){
								Ntmp = tree_search(root, IDarr[i]);
								fcmd1[0] = '\0';
								sprintf(fcmd1,"%d %d %d\n"
										,Ntmp->ID, Ntmp->left_stock, Ntmp->price);
								strcat(fcmd2, fcmd1);
							}
							Write(fd, fcmd2, strlen(fcmd2));
							Close(fd);
						}
					}
					else{
						strcpy(cmd, "There is no such stock\n");
					}
				}
				else if(!strncmp(buf, "sell", 4)){
					strcpy(copy,buf);
					tmp = strtok(copy, " ");
					tmp = strtok(NULL, " ");
					key = atoi(tmp);
					tmp = strtok(NULL, " ");
					value = atoi(tmp);
					if((Ntmp = tree_search(root, key))!=NULL){
						fd = Open("stock.txt", O_RDWR|O_TRUNC, S_IWUSR);
						strcpy(cmd, "[sell] succes\n");
						fcmd2[0] = '\0';
						tree_sell(Ntmp, value);
						for(int i=0;i<len;i++){
							Ntmp = tree_search(root, IDarr[i]);
							fcmd1[0] = '\0';
							sprintf(fcmd1, "%d %d %d\n"
									, Ntmp->ID, Ntmp->left_stock, Ntmp->price);
							strcat(fcmd2, fcmd1);
						}
						Write(fd, fcmd2, strlen(fcmd2));
						Close(fd);
					}
					else{
						strcpy(cmd,"There is no such stock\n");
					}
				}
				else if(!strncmp(buf, "exit", 4)){
					printf("client %d is closed\n",connfd);
					Close(connfd);
					FD_CLR(connfd, &p->read_set);
					p->clientfd[i]=-1;
					continue;
				}
				Rio_writen(connfd, cmd, sizeof(cmd));
			}
			else{
				printf("client %d is closed\n", connfd);
				Close(connfd);
				FD_CLR(connfd, &p->read_set);
				p->clientfd[i]=-1;
			}
		}
	}
}

void init_tree(const char*filename)
{
	FILE*fp = fopen(filename, "r");
	int i,j,k;
	while(fscanf(fp,"%d %d %d", &i, &j, &k)!=EOF){
		root = tree_insert(root,i,j,k);
		IDarr[len++] = i;
	}
	fclose(fp);
}
NODE* tree_insert(NODE*root, int ID, int stock_left, int price)
{
	if(root==NULL){
		root = (NODE*)malloc(sizeof(NODE));
		root->ID = ID;
		root->left_stock = stock_left;
		root->price = price;
		root->readcnt = 0;
		root->lchild = NULL;
		root->rchild = NULL;
		return root;
	}
	else{
		if(ID>root->ID)
			root->rchild = tree_insert(root->rchild, ID, stock_left, price);
		else
			root->lchild = tree_insert(root->lchild, ID, stock_left, price);
	}
	return root;
}
NODE* tree_search(NODE* root, int key)
{
	NODE*tmp =root;
	while(1)
	{
		if(key>tmp->ID){
			tmp = tmp->rchild;
		}
		else if(key < tmp->ID){
			tmp = tmp->lchild;
		}
		else{
			return tmp;
		}
		if(tmp==NULL)
			break;
	}
	return NULL;
}
void tree_sell(NODE*tmp, int num)
{
	tmp->left_stock += num;
}
void tree_buy(NODE*tmp, int num)
{
	tmp->left_stock -= num;
}
