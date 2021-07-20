/*
 stock server
 concurrent based by pthread
   */
#include "sbuf.h"
#include "csapp.h"

#define NTHREADS 10
#define SBUFSIZE 16

// binary search tree NODE
#define MAXSTOCK 10

typedef struct _Node{
	int ID;
	int left_stock;
	int price;
	int readcnt;
	sem_t mutex, w; // mutex for node reference
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

static int byte_cnt=0;

// thread-based concurrent server
void* thread(void* vargp);
void thread_stock(int connfd);
sbuf_t sbuf;
sem_t mutex, w; // mutex for file reference
int readcnt=0;


int main(int argc, char **argv) 
{
    int listenfd, connfd, i;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */
    char client_hostname[MAXLINE], client_port[MAXLINE];
	pthread_t tid;

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }

	// make stock tree
	init_tree("stock.txt");

    listenfd = Open_listenfd(argv[1]); // listenfd to accept

	sbuf_init(&sbuf, SBUFSIZE); // set shared buffer for descriptors
	Sem_init(&mutex, 0, NTHREADS);
	Sem_init(&w, 0, 1);

	for (i = 0; i < NTHREADS; i++) // make thread-pool to minimize overhead for thread control
		Pthread_create(&tid, NULL, thread, NULL);
	while (1) {
		clientlen = sizeof(struct sockaddr_storage);
		connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
		Getnameinfo((SA*)&clientaddr, clientlen, client_hostname, MAXLINE,
				client_port, MAXLINE, 0);
		printf("Connected to (%s, %s)\n", client_hostname, client_port);
		sbuf_insert(&sbuf, connfd);
	}
}

void* thread(void* vargp)
{
	Pthread_detach(pthread_self());
	while (1)
	{
		int connfd = sbuf_remove(&sbuf);
		thread_stock(connfd);
		Close(connfd);
		printf("client %d is close\n",connfd);
	}
}

void thread_stock(int connfd)
{
	int n,fd;
	int key, value;
	char buf[MAXLINE], cmd[MAXLINE];
	char fcmd1[MAXLINE], fcmd2[MAXLINE];
	char copy[MAXLINE], *tmp;
	rio_t rio;
	NODE* Ntmp;

	Rio_readinitb(&rio, connfd);
	while((n=Rio_readlineb(&rio,buf,MAXLINE))>0){
		P(&w);
		byte_cnt+=n;
		printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
		V(&w);
		if(!strncmp(buf,"show",4)){
			fd = Open("stock.txt", O_RDONLY, S_IRUSR);
			P(&mutex);
			readcnt++;
			if(readcnt == 1)
				P(&w); // block writing, allow multiple reading
			V(&mutex);

			rio_readn(fd, cmd, sizeof(cmd)); // critical(read file)
			Close(fd);
			
			P(&mutex);
			readcnt--; // reading finished
			if(readcnt == 0)
				V(&w); // all readers finished
			V(&mutex);
		}
		else if(!strncmp(buf,"buy",3)){
			strcpy(copy, buf);
			tmp = strtok(copy," ");
			tmp = strtok(NULL, " ");
			key = atoi(tmp);
			tmp = strtok(NULL," ");
			value = atoi(tmp);
			if((Ntmp=tree_search(root,key))!=NULL){
				P(&Ntmp->mutex); // Reading node
				Ntmp->readcnt++;
				if(Ntmp->readcnt==1)
					P(&Ntmp->w);
				V(&Ntmp->mutex);
				if(Ntmp->left_stock<value){
					strcpy(cmd,"Not enough left stock\n");
					P(&Ntmp->mutex);
					Ntmp->readcnt--;
					if(Ntmp->readcnt==0)
						V(&Ntmp->w);
					V(&Ntmp->mutex);
				}
				else{
					P(&Ntmp->mutex); // too write, need (V(&Ntmp->w))
					Ntmp->readcnt--;
					if(Ntmp->readcnt==0)
						V(&Ntmp->w);
					V(&Ntmp->mutex);
					P(&w); // file writing
					fd = Open("stock.txt", O_RDWR|O_TRUNC, S_IWUSR);
					strcpy(cmd, "[buy] succes\n");
					fcmd2[0] = '\0';
					P(&Ntmp->w);// node writing
					tree_buy(Ntmp,value); // critical
					V(&Ntmp->w);
					for(int i=0;i<len;i++){
						Ntmp= tree_search(root,IDarr[i]);
						fcmd1[0]='\0';
						sprintf(fcmd1,"%d %d %d\n"
								,Ntmp->ID, Ntmp->left_stock, Ntmp->price);
						strcat(fcmd2,fcmd1);
					}
					Write(fd, fcmd2, strlen(fcmd2));
					Close(fd);
					V(&w);
				}
			}
			else{
				strcpy(cmd,"There is no such stock\n");
			}
		}
		else if(!strncmp(buf,"sell",4)){
			strcpy(copy,buf);
			tmp = strtok(copy," ");
			tmp = strtok(NULL," ");
			key = atoi(tmp);
			tmp = strtok(NULL," ");
			value = atoi(tmp);
			if((Ntmp=tree_search(root, key))!=NULL){
				// file writing
				P(&w);
				fd = Open("stock.txt", O_RDWR|O_TRUNC, S_IWUSR);
				strcpy(cmd, "[sell] succes\n");
				fcmd2[0] = '\0';
				// node writing
				P(&Ntmp->w);
				tree_sell(Ntmp, value); //critical
				V(&Ntmp->w);
				for(int i=0;i<len;i++){
					Ntmp = tree_search(root, IDarr[i]);
					fcmd1[0] = '\0';
					sprintf(fcmd1,"%d %d %d\n",
							Ntmp->ID, Ntmp->left_stock, Ntmp->price);
					strcat(fcmd2, fcmd1);
				}
				Write(fd, fcmd2, strlen(fcmd2));
				Close(fd);
				V(&w);
			}
		}
		else if(!strncmp(buf,"exit",4)){
//			printf("client %d is closed\n",connfd);
			break;
		}
		Rio_writen(connfd, cmd, sizeof(cmd));
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
		Sem_init(&root->mutex, 0, NTHREADS);
		Sem_init(&root->w, 0, 1);
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
