#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<sys/time.h>
#include<unistd.h>
#include<stdlib.h>

#define BUFMAX 40
#define PORT_NO (u_short)20000
#define Err(x) {fprintf(stderr,"-"); perror(x); exit(0);}

static int n,number,sofd,nsofd,retval;
static struct hostent *shost;
static struct sockaddr_in own,svaddr;
static char buf[BUFMAX], shostn[BUFMAX];
static fd_set mask; 
static struct timeval tm; 


int seavpro();
int cliepro();


main()
{

while( 1 ) {
		printf( "server:1 client:2\n" );
		scanf( "%d", &number );
		printf( "\n" );
		if( number == 1 || number == 2 ) break;
	}

	if( number == 1 ) servpro();
	if( number == 2 ) cliepro();
}

int servpro()
{
Display *d;
Window w;
  XEvent event;
GC gc;
unsigned long white,black;
int a[30],b[30],data,i,k=1,j,l;
 char c[30];


	tm.tv_sec = 0; 
	tm.tv_usec = 1; 

	if( gethostname( shostn, sizeof( shostn ) ) < 0 )
		Err( "gethostname" );

	printf( "hostname is %s", shostn );
	printf( "\n" );

	shost = gethostbyname( shostn );
	if( shost == NULL ) Err( "gethostbyname" );

	bzero( (char *)&own, sizeof( own ) );
	own.sin_family = AF_INET;
	own.sin_port = htons(PORT_NO);
	bcopy( (char *)shost->h_addr, (char *)&own.sin_addr, shost->h_length );

	sofd = socket( AF_INET, SOCK_STREAM, 0 );
	if( sofd < 0 ) Err( "socket" );
	if( bind( sofd, (struct sockaddr*)&own, sizeof( own ) ) < 0 ) Err( "bind" );

	listen( sofd, 1 );

	nsofd = accept( sofd, NULL, NULL );
	if( nsofd < 0 ) Err( "accept" );
	close( sofd );

	write( 1, "GAME START\n", 11 );


	d=XOpenDisplay(NULL);
	white=WhitePixel(d,0);
	black=BlackPixel(d,0);
	w=XCreateSimpleWindow(d,RootWindow(d,0),300,300,700,600,1,black,white);

	gc=XCreateGC(d,w,0,0);

	XSelectInput(d,w,ButtonPressMask|ExposureMask);
	XMapWindow(d,w);

	while( 1 ) {

		FD_ZERO( &mask ); 
		FD_SET( nsofd, &mask );  
		FD_SET( 0, &mask ); 
		retval = select( nsofd+1, &mask, NULL, NULL, &tm ); 
		
		if( retval < 0 ) Err( "select" ); 

	

		if( FD_ISSET( nsofd, &mask )){
		  n=read( nsofd, buf, BUFMAX );
		  k++;
		  strncpy(c,buf+6,2);
		  data=atoi(c);
		  a[k]=data/10;
		  b[k]=data%10;
		  XDrawArc(d,w,gc,a[k]*100+1,b[k]*100+1,98,98,0*64,360*64);

			write( 1, buf, n );

			if( !strcmp( buf, "Q\n" ) ) break;
			bzero( buf, BUFMAX );
		}
		  
		while(XPending(d)){
			  XNextEvent(d,&event);
			  switch(event.type){

      //再描画の処理
			  case Expose:
      //盤面の格子の描画
			    for(j=1;j<8;j++){
			      XDrawLine(d,w,gc,j*100,0,j*100,600);
			    }
			    for(l=1;l<7;l++){
			      XDrawLine(d,w,gc,0,l*100,700,l*100);
			    }

      //黒丸と白丸の再描画
			    for(i=1;i<k;i++){
			      if(i%2==1){
				XFillArc(d,w,gc,a[i]*100+1,b[i]*100+1,98,98,0*64,360*64);
			      }else{
				XDrawArc(d,w,gc,a[i]*100+1,b[i]*100+1,98,98,0*64,360*64);
			      }
			    }break;

	//ボタンを押したときの処理
			  case ButtonPress:
			    a[k]=(event.xbutton.x/100);//x座標の設定
			    b[k]=(event.xbutton.y/100);//y座標の設定

			    XFillArc(d,w,gc,a[k]*100+1,b[k]*100+1,98,98,0*64,360*64);

			    sprintf(buf,"PLACE-%d%d\n",a[k],b[k]);
			    write(nsofd,buf,10);
			    k++;
			    break;
			  }
		}
	  

	}
	XFlush(d);
	getchar();
	close( nsofd );
}

int cliepro()
{
  Display *d;
  Window w;
  XEvent event;
  GC gc;
  unsigned long white,black;
  int a[30],b[30],data,i,k=1,j,l;
  char c[30];

	tm.tv_sec  = 0; 
	tm.tv_usec = 1; 

	printf( "hostname?\n" );
	scanf( "%s",shostn );

	shost = gethostbyname( shostn );
	if( shost == NULL ) Err( "gethostbyname" );

	bzero( (char *)&svaddr, sizeof(svaddr) );
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons( PORT_NO );

	bcopy( (char *)shost->h_addr, (char *)&svaddr.sin_addr, shost->h_length );

	sofd = socket( AF_INET, SOCK_STREAM, 0 );
	if( sofd < 0 ) Err( "socket" );

	connect( sofd, (struct sockaddr*)&svaddr, sizeof( svaddr ) );

	write( 1, "GAME START\n", 11 );
	

	d=XOpenDisplay(NULL);
	white=WhitePixel(d,0);
	black=BlackPixel(d,0);
	w=XCreateSimpleWindow(d,RootWindow(d,0),300,300,700,600,1,black,white);

	XSelectInput(d,w,ButtonPressMask|ExposureMask);
	XMapWindow(d,w);
	gc=XCreateGC(d,w,0,0);


	while( 1 ) {

		FD_ZERO( &mask ); 
		FD_SET( sofd, &mask ); 
		FD_SET( 0, &mask ); 
		retval=select( sofd+1, &mask, NULL, NULL, &tm ); 

		if( retval < 0 ) Err( "select" );

		if( FD_ISSET( sofd, &mask) ) { 
		  n=read(sofd,buf,BUFMAX);
		  k++;
		  strncpy(c,buf+6,2);
		  data=atoi(c);
		  a[k]=data/10;
		  b[k]=data%10;

		  XFillArc(d,w,gc,a[k]*100+1,b[k]*100+1,98,98,0*64,360*64);
		  write(1,buf,n);

			if( !strcmp( buf, "Q\n" ) ) break;
			bzero( buf, BUFMAX );
		}

		while(XPending(d)){
		  XNextEvent(d,&event);
		  switch(event.type){

      //再描画の処理
			  case Expose:
      //盤面の格子の描画
	printf("aaa");
			    for(j=1;j<8;j++){
			      XDrawLine(d,w,gc,j*100,0,j*100,600);
			    }
			    for(l=1;l<7;l++){
			      XDrawLine(d,w,gc,0,l*100,700,l*100);
			    }

      //黒丸と白丸の再描画
			    for(i=1;i<k;i++){
			      if(i%2==1){
				XFillArc(d,w,gc,a[i]*100+1,b[i]*100+1,98,98,0*64,360*64);
			      }else{ 
				XDrawArc(d,w,gc,a[i]*100+1,b[i]*100+1,98,98,0*64,360*64);
			      }
			    }break;

	//ボタンを押したときの処理
			  case ButtonPress:
			    a[k]=(event.xbutton.x/100);//x座標の設定
			    b[k]=(event.xbutton.y/100);//y座標の設定

			    XDrawArc(d,w,gc,a[k]*100+1,b[k]*100+1,98,98,0*64,360*64);

      //白丸と黒丸を交互に描画する
			  
			   
			    sprintf(buf,"PLACE-%d%d\n",a[k],b[k]);
			    write(sofd,buf,sizeof(buf));
			    k++;
			    break;
		  }
		}
	XFlush(d);
	getchar();

					}
	close( sofd );
}
