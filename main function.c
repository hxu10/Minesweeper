#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#define REPEAT 1  //用于时间测量，重复次数
#define PMIN 0.05//P表示该处有地雷的概率，是从PMIN向PMAX逐步变换的
#define PMAX 0.25
#define PSTEP 0.001 //每次概率变化的幅度
#define M 9//扫雷区域的长度
#define N 9//扫雷区域的宽度
#define T 10000 //每次概率变化时运行的次数
#define DISPLAY 0 //是否显示过程
#define JUSTIFY 0  //判定胜利条件
#define INTERVAL 1   //统计时间隔多少地雷
#define TEST 0  //如果是1，那么处于测试模式
static short area[M+1][N+1];//有雷与否的情况，有雷为0，没有雷为1
static short explore[M+1][N+1];//玩家能看到的画面，9代表未开发，0-8分别代表地雷的个数，10代表已经标记为地雷
static short unknown[M+1][N+1];//未知板块，用于做逻辑判断
static short suspect[M+1][N+1];//怀疑是否有雷板块，用于做逻辑判断
static short left_area[8];//左区域
static short right_area[8];//右区域
static short common[M][N][5][5][4];

static double density[M+1][N+1]; //密度操作

static int totalunknown; //总共的未知板块
static int totalmine; //总共的地雷
static int touchmine; //触雷判定
static int unknownmine; //未知地雷
static float click;  //模拟时间
static int logictimes; //逻辑次数
static int opps; //统计最后出现二选一的情况





//随机数产生器
int ran(int seed)
{
int  num;
num=16807*(seed%127773)-2836*(seed/127773);
if (num<0) num=num+2147483647;
return num;
}


//鉴定这个是不是落在边界内部，出界返回0
int boundary(int i,int j)
{
  if(i>-1 && i<M && j>-1 && j<N) return 1;
  else return 0;
}

//判断两个格点是否相邻，对角返回2，相邻返回1，否则为0
int neigh(int i,int j,int i1,int j1)
{
  int distance;
  distance=(i1-i)*(i1-i)+(j1-j)*(j1-j);
  if(distance==2) return 2;
  if(distance==1) return 1;
  else return 0;
}


//计算两个板块的共同部分，注意：这里编号代表相邻程度。从0到23，分别从左到右，从上到下。
void getcommon()
{
   int i,j,i1,j1,k;
   int p,q;
   

   for(i=0;i<M;i++)
   {
      for(j=0;j<N;j++)
	  {
	     for(i1=0;i1<5;i1++)
		 {
		    for(j1=0;j1<5;j1++)
			{
              if(boundary(i+i1-2,j+j1-2)==0) continue; //如果要比较的格点出界，直接跳过
			  k=0;

			  for(p=i-1;p<=i+1;p++)
			  {
			     for(q=j-1;q<=j+1;q++)
				 {
				   if(boundary(p,q)==0) continue;  //如果周边的格点有出界的，直接跳过
                   if(p==i && q==j) continue;      //中心的格点自然跳过
				   if(neigh(p,q,i+i1-2,j+j1-2)!=0) 
				   {
					   common[i][j][i1][j1][k]=p*N+q+1;
					   k++;
				   }
				 
				 }
			  
			  }
		
			}		 
		 
		 }
	  	  	  
	  }
   
   }

}




//打印用户界面函数
void printexplore()
{
int i,j;
 printf("----------------------------------------------------------\n");
 for(i=0;i<M;i++)
 {
	 for(j=0;j<N;j++) 
	 { 
		 if(explore[i][j]==9) {printf("█");continue;}
		 if(explore[i][j]==10) {printf("●");continue;}
         if(explore[i][j]==0) {printf("  ");continue;}
	 printf("%2d",explore[i][j]);
	 }
 printf("\n");
 }
 printf("----------------------------------------------------------\n");
}
//打印失败界面函数
void printfail(int k,int l)
{
 int i,j;
 system("cls");
 printf("第%d行第%d列是雷，你触♂雷了！\n",k+1,l+1);
 printf("板块\t  剩余雷数\t开采进度\n");
 printf("%d×%d\t     %d\t\t%d/%d\n",M,N,unknownmine,M*N-totalunknown,M*N);
 printf("----------------------------------------------------------\n");
 for(i=0;i<M;i++)
 {
	 for(j=0;j<N;j++) 
	 { 
		 if(i==k && j==l) {printf("♂");continue;}
		 if(area[i][j]==1) {printf("%2c",11);continue;}
		 if(explore[i][j]==9) {printf("█");continue;}
         printf("  ");
	 }
 printf("\n");
 }
 printf("----------------------------------------------------------\n");
}

//打印unknown函数
void printunknown()
{
int i,j;
 for(i=0;i<M;i++)
 {
	 for(j=0;j<N;j++) 
	 { 
	 printf("%2d",unknown[i][j]);
	 }
 printf("\n");
 }
 printf("-----------------------unknown----------------------------\n");
}

//打印密度
void printdensity()
{
int i,j;
 for(i=0;i<M;i++)
 {
	 for(j=0;j<N;j++) 
	 { 
	 printf("%4.1f",density[i][j]);
	 }
 printf("\n");
 }
 printf("-----------------------density----------------------------\n");
}



//打印unknown函数
void printsuspect()
{
int i,j;
 for(i=0;i<M;i++)
 {
	 for(j=0;j<N;j++) 
	 { 
	 printf("%2d",suspect[i][j]);
	 }
 printf("\n");
 }
 printf("----------------------suspect-----------------------------\n");
}

//打印区域函数，目标是打印出地雷的分布图
void printarea()
{
 int i,j;
 for(i=0;i<M;i++)
 {   
	 for(j=0;j<N;j++)
	 {
		 if(area[i][j]==0) {printf("  ");continue;}
		 printf("%2d",area[i][j]);
	 }
 printf("\n");
 }
  printf("---------------------------------------------------------\n");
}

//显示函数，显示屏幕的扫雷进度
void display()
{ 
	int i;

     if (DISPLAY==1) 
	{
	   printf("板块\t  剩余雷数\t开采进度\n");
	   printf("%d×%d\t    %3d \t %3d/%d\n",M,N,unknownmine,M*N-totalunknown,M*N);
	   printexplore();				   	  
      for(i=0;i<234567891;i++);
	
	}
} 





//当这一块被开采或被标记后，其周围的8块图块自减1
void modunknown(int i,int j)
{
 if(i>0 && j>0)   unknown[i-1][j-1]--;
 if(i>0 )  unknown[i-1][j]--;
 if(i>0 && j<N-1)  unknown[i-1][j+1]--;
 if(j>0)  unknown[i][j-1]--;
 if(j<N-1)  unknown[i][j+1]--;
 if(i<N-1 && j>0)  unknown[i+1][j-1]--;
 if(i<N-1)  unknown[i+1][j]--;
 if(i<N-1 && j<N-1)  unknown[i+1][j+1]--;
}



//功能函数，给定一个特定的块，返回周边方块地雷的个数
int count(int i,int j)
{
 int number=0;
 if(i==M-1 && j==N-1) return area[i-1][j]+area[i-1][j-1]+area[i][j-1];
 if(i==M-1 && j==0) return area[i-1][j]+area[i-1][j+1]+area[i][j+1];
 if(i==0 && j==N-1) return area[i+1][j]+area[i+1][j-1]+area[i][j-1];
 if(i==0 && j==0) return area[i][j+1]+area[i+1][j+1]+area[i+1][j];
 if(i==0 && j%(N-1)!=0) return area[i][j-1]+area[i][j+1]+area[i+1][j-1]+area[i+1][j]+area[i+1][j+1];
 if(i==M-1 && j%(N-1)!=0) return area[i][j-1]+area[i][j+1]+area[i-1][j-1]+area[i-1][j]+area[i-1][j+1];
 if(j==0 && i%(M-1)!=0) return area[i-1][j]+area[i+1][j]+area[i-1][j+1]+area[i][j+1]+area[i+1][j+1];
 if(j==N-1 && i%(M-1)!=0) return area[i+1][j]+area[i-1][j]+area[i+1][j-1]+area[i-1][j-1]+area[i][j-1];
 return area[i-1][j-1]+area[i][j-1]+area[i+1][j-1]+area[i-1][j]+area[i+1][j]+area[i-1][j+1]+area[i][j+1]+area[i+1][j+1];
}

//开采函数
void exploit(int i,int j,int flag)
{

	if (boundary(i,j)==0 ) return;
	if (explore[i][j]!=9) return; //越出边境或者已经被开采过的，不予开采。
    if (flag==1 && DISPLAY==1) {system("cls");printf("扫雷进行中...\n");display();}
  //  if (flag==1) {click+=1;}
	if(area[i][j]==1) 
	{
      if(TEST==1)
	  {
		printf("你触雷了!\n"); 
		printf("%d行%d列\n",i+1,j+1);
		printf("未知块数%d，剩余地雷%d\n",totalunknown,unknownmine);
		printexplore(); 
		printarea();
		printunknown();
		printdensity();
		exit(0);
	  }

		if(DISPLAY==1) printfail(i,j);
		touchmine = 1;
		return ;
	}
	totalunknown--;
    explore[i][j]=count(i,j);
    modunknown(i,j);
	if(explore[i][j]==0)
	{
	 exploit(i-1,j-1,2);
	 exploit(i-1,j,2);
	 exploit(i-1,j+1,2);
	 exploit(i,j-1,2);
	 exploit(i,j+1,2);
	 exploit(i+1,j-1,2);
	 exploit(i+1,j,2);
	 exploit(i+1,j+1,2);
	}
}

void makeclear(int i,int j)
{
//   printf("make clear %d %d\n",i,j);
  if(boundary(i,j)==0) return;
  if(explore[i-1][j-1]==9) exploit(i-1,j-1,1);
  if(explore[i-1][j]==9) exploit(i-1,j,1);
  if(explore[i-1][j+1]==9) exploit(i-1,j+1,1);
  if(explore[i][j-1]==9) exploit(i,j-1,1);
  if(explore[i][j+1]==9) exploit(i,j+1,1);
  if(explore[i+1][j-1]==9) exploit(i+1,j-1,1);
  if(explore[i+1][j]==9) exploit(i+1,j,1);
  if(explore[i+1][j+1]==9) exploit(i+1,j+1,1);

}

//计算周围地雷中
void calcsuspect(int i,int j)
{
 int a=0;
 if(explore[i-1][j-1]==10) a++;
 if(explore[i-1][j]==10) a++;
 if(explore[i-1][j+1]==10) a++;
 if(explore[i][j-1]==10) a++;
 if(explore[i][j+1]==10) a++;
 if(explore[i+1][j-1]==10) a++;
 if(explore[i+1][j]==10) a++;
 if(explore[i+1][j+1]==10) a++;
 suspect[i][j]=a;
}



void modsuspect(int i,int j)
{
 // printf("modsuspect%d %d\n",i,j);

  calcsuspect(i-1,j-1);
    if(suspect[i-1][j-1]==explore[i-1][j-1]) makeclear(i-1,j-1);
  calcsuspect(i-1,j);
    if(suspect[i-1][j]==explore[i-1][j]) makeclear(i-1,j);
  calcsuspect(i-1,j+1);
    if(suspect[i-1][j+1]==explore[i-1][j+1]) makeclear(i-1,j+1);
  calcsuspect(i,j-1);
    if(suspect[i][j-1]==explore[i][j-1]) makeclear(i,j-1);
  calcsuspect(i,j+1);
    if(suspect[i][j+1]==explore[i][j+1]) makeclear(i,j+1);
  calcsuspect(i+1,j-1);
    if(suspect[i+1][j-1]==explore[i+1][j-1]) makeclear(i+1,j-1);
  calcsuspect(i+1,j);
    if(suspect[i+1][j]==explore[i+1][j]) makeclear(i+1,j);
  calcsuspect(i+1,j+1);
    if(suspect[i+1][j+1]==explore[i+1][j+1]) makeclear(i+1,j+1);
}


//把第i行第j列标记成地雷
void mark(int i, int j)
{
	explore[i][j]=10;
	modsuspect(i,j);
	modunknown(i,j);
	unknownmine--;
	totalunknown--;
}


// 当地雷的未知数目和地雷数目相同时，其周围全部标记上地雷
void markall(int i,int j)
{
	if(explore[i-1][j-1]==9) {mark(i-1,j-1);}
	if(explore[i-1][j]==9)   {mark(i-1,j);}
	if(explore[i-1][j+1]==9) {mark(i-1,j+1);}
	if(explore[i][j-1]==9)   {mark(i,j-1);}
	if(explore[i][j+1]==9)   {mark(i,j+1);}
	if(explore[i+1][j-1]==9) {mark(i+1,j-1);}
	if(explore[i+1][j]==9)   {mark(i+1,j);}
	if(explore[i+1][j+1]==9) {mark(i+1,j+1);}
}









//对特定区域进行处理，前两个数字为要处理的单区域，1代表全部打开，2代表全部标记
void handle(int i,int j,int i1,int j1,int flag)
{
 //  printf("下面执行%d处理\n",flag);
   int p,q,k=0;
   for(p=i-1;p<i+2;p++)
   {
     for(q=j-1;q<j+2;q++)
	 {
	    if(p==i && q==j) continue;
		if(neigh(p,q,i1,j1)!=0 || boundary(p,q)==0) continue;
		if(flag==1) exploit(p,q,1);
		if(flag==2 && explore[p][q]==9) mark(p,q);
        
	 }
   }

}





//逻辑操作，本作品中的精华部分
void logic(int i, int j, int i1, int j1)
{

    int common_unknown,left_unknown,right_unknown;
	int common_mine,left_mine,right_mine;
	int common_guess; //共同区域猜测的地雷数目
	int k,l,p,q,num;
	click ++;
	logictimes++;

	common_unknown = 0;
	common_mine = 0;
    k=0;
	l=0;
	num= (i-i1)*5+(j-j1);


 //   printf("逻辑开始%d,%d,%d,%d\n",i+1,j+1,i1+1,j1+1);

    for(k=0;k<4;k++)
	{
        l=common[i][j][i1-i+2][j1-j+2][k];
		if(l==0) continue;
		p=(l-1)/N;
		q=(l-1)%N;

	    if(explore[p][q]==10) common_mine++;
		if(explore[p][q]==9)  common_unknown++;
	}

	
    //我们定义，i,j为左边，i1,j1为右边
	left_unknown = unknown[i][j]-common_unknown;
	right_unknown = unknown[i1][j1]-common_unknown;
	left_mine = suspect[i][j]-common_mine;
	right_mine = suspect[i1][j1]-common_mine;
	if(left_unknown+right_unknown==0) return;
	


    //左边或右边所有的地方都不是雷
	   if(explore[i1][j1]-right_mine==explore[i][j]-left_mine)
	{
		if(left_unknown == 0)  handle(i1,j1,i,j,1);
		if(right_unknown == 0)   handle(i,j,i1,j1,1);
	}
	//左边或右边所有的地方都是雷

	   	if(explore[i1][j1]-right_mine-right_unknown==explore[i][j]-left_mine-left_unknown)
	{
		if(left_unknown == 0)  handle(i1,j1,i,j,2);
		if(right_unknown == 0)   handle(i,j,i1,j1,2);
	}
	// 判断左边全是雷，右边都不是
    if(explore[i][j]-explore[i1][j1]==left_mine+left_unknown)
	{
		   handle(i1,j1,i,j,1);
		   handle(i,j,i1,j1,2);
	}
	//右边全是雷，左边都不是     
	if(explore[i1][j1]-explore[i][j]==right_mine+right_unknown)
	{
		   handle(i1,j1,i,j,2);
		   handle(i,j,i1,j1,1);
	}
}

//给一个特定的格点均分地雷，返回1表示标记上了，否则表示没有标记上
int dist_density(int i,int j)
{
  int p,q,temp;
  temp=explore[i][j]-suspect[i][j];
  //计算要被扣除的地雷

  if(temp==0 || temp == unknownmine) return 0;

  for(p=i-1;p<i+2;p++)
  {
     for(q=j-1;q<j+2;q++)
	 {
        if((p==i && q==j) || boundary(p,q)==0) continue;	 
		if(density[p][q]!=0)  {return 0;}
	 }

  }


  //剩下的均摊分到未知区域
  for(p=i-1;p<i+2;p++)
  {
     for(q=j-1;q<j+2;q++)
	 {
        if((p==i && q==j) || boundary(p,q)==0) continue;	 
		if(explore[p][q]==9)  {density[p][q]=temp*1.0/unknown[i][j];}
	 }

  }
return 1;
}




int makeclear()
{
  int i,j,temp1,temp2,flag=0;
  temp1=totalunknown; //剩下的未知板块数
  temp2=unknownmine; //剩下的未知地雷数
  //初始化，地雷密度全是0
  for(i=0;i<M;i++)
  {
    for(j=0;j<N;j++)
	{
       density[i][j]=0;
	}
  }

  //标记密度
  for(i=0;i<M;i++)
  {
     for(j=0;j<N;j++)
	 {
	    if(unknown[i][j]>0 && explore[i][j]<9)
		{
			if(dist_density(i,j)==1) 
			{  
		//	  	if(i==0 && j==1) printf("kaka %d %d %d\n",unknown[i][j],explore[i][j],suspect[i][j]);
		//		 if(i==0 && j==1) printexplore();
				 temp1-=unknown[i][j];
				 temp2-=(explore[i][j]-suspect[i][j]);
			}
		} 

	 }
  }
  //如果剩余地雷全部用完，直接把没有的全开采
  if(temp2==0)
  {
    for(i=0;i<M;i++)
	{
      for(j=0;j<N;j++)
	  {
		  if(explore[i][j]==9 && density[i][j]==0) {exploit(i,j,1);flag=1;}
	  }
	}

  }
    //如果剩余地雷与未知区域相等，直接把没有的全标记
  if(temp2==temp1)
  {
    for(i=0;i<M;i++)
	{
      for(j=0;j<N;j++)
	  {
		  if(explore[i][j]==9 && density[i][j]==0) {mark(i,j);flag=1;}
	  }
	}

  }



return flag;

}




// 当地雷数目仅剩几个的时候，展开clearall操作
int clearall()
{
   int i,j,flag=0;
   if (unknownmine==0) //所有雷都知道了，就直接胜利
   {
      for(i=0;i<M;i++)
	  {
	     for(j=0;j<N;j++)
		 {
		    if(explore[i][j]==9) exploit(i,j,1);
		 }
	  }
	  return 1;
   }
   else
   {
      
      if(makeclear()==1)  return 1;
   }

   return 0;
}


//对地图进行一轮扫雷操作
void cycle(int flag)
{
	int i,j,i1,j1,diff;
	if(flag==1)
	{
    for(i=0;i<M;i++)
	 {
        for(j=0;j<N;j++)
		{
		  if(unknown[i][j]+suspect[i][j]==explore[i][j] && explore[i][j]<9 && unknown[i][j]!=0) {markall(i,j);click++;}	
		  else if(suspect[i][j]==explore[i][j] && unknown[i][j]>0) {makeclear(i,j);click++;}
          else if(unknown[i][j]*explore[i][j]!=0 && explore[i][j]<9) {click+=0.25;}

		}

	 }
	  
	}

	else if(flag==2)
	{
	  for(i=0;i<M;i++)
	  {
         for(j=0;j<N;j++)
		 {
            if(explore[i][j]==0 || explore[i][j]>7) continue;
	        if(unknown[i][j]+unknown[i+1][j]>4 && explore[i+1][j]>0 && explore[i+1][j]<8 && boundary(i+1,j)!=0) logic(i,j,i+1,j);
	        if(unknown[i][j]+unknown[i][j+1]>4 && explore[i][j+1]>0 && explore[i][j+1]<8 && boundary(i,j+1)!=0) logic(i,j,i,j+1);
		 }
	  }
	}

    else if(flag==3)
     for(i=0;i<M;i++)
	 {
        for(j=0;j<N;j++)
		{
		   if(explore[i][j]>7 || explore[i][j]==0) continue; //如果它是未知区域或者周围没地雷，直接跳过
		   for(i1=i;i1<i+3;i1++)
		   {
              for(j1=j-2;j1<j+3;j1++)
			  {
				if(unknown[i][j]+unknown[i1][j1]<5) continue;
				if(explore[i1][j1]>7 || explore[i1][j1]==0) continue; 
				diff=(i1-i)*N+j1-j;
                if(diff<2 || diff==N) continue;
			    if(boundary(i1,j1)!=0)  logic(i,j,i1,j1);
			  }
		   }
	//	  printf("hor---%d行%d列\n",i+1,j+1);
	//	  printf("ver---%d行%d列\n",i+1,j+1);
    //如果触雷，探索失败
		}
	 }

}


//从未开采的方块里随机选择一个
int randomchoice(int seed)
{
	int i,j,choice;
    seed= ran(seed);
    choice = int(seed*1.0/2147483647*totalunknown)+1;
    if(totalunknown==2 && unknownmine==1) {opps++;}

	click+=5;
//	if(totalunknown<(M*N/2) ) click+=20;
//	if(totalunknown<(M*N/4) ) click+=30;

//	printf("一共未知%d,随机数是%d\n",totalunknown,choice);
//	printexplore();
	for(i=0;i<M;i++)
	{
	   for(j=0;j<N;j++)
	   {
	      if(explore[i][j]==9) 
		  {
			  choice--;
			  if(choice==0) exploit(i,j,1);

		  }
	   }
	}
return seed;
}








//更新地图的操作
int initial(double p,int seed)
{
    int i,j,mine,firstx,firsty;
	double x;//x代表0-1之间的均匀抽样数
	totalunknown=M*N;
	firstx=M/2;firsty=N/2;

	//雷区重置为0
	for(i=0;i<=M;i++)
	{ 
		 for(j=0;j<=N;j++)
		{
	       area[i][j]=0;
		}
	}
   //其他区域重置
	 for(i=0;i<M;i++)
   {
      for(j=0;j<N;j++)
	  {
        suspect[i][j]=0;
        explore[i][j]=9;
        unknown[i][j]=8;   //周围的地雷不知道的个数
        if( i==0 || i==M-1 || j==0 || j==N-1 ) unknown[i][j]=5;
        if( i==0 && j==0) unknown[i][j]= 3;
        if( i==0 && j==N-1) unknown[i][j]= 3;
        if( i==M-1 && j==0) unknown[i][j]= 3;
        if( i==M-1 && j==N-1) unknown[i][j]= 3;
	  }
   }
	 //放置地雷
   mine=0;
   for(i=0;i<M;i++)
   {
     for(j=0;j<N;j++)
	{

	  if(abs(i-firstx)<2 &&  abs(j-firsty)<2) continue;
//	  if(i==0 && j==N-1) continue;
//	  if(i==0 && j==0) continue;
//	  if(i==M-1 && j==0) continue;
//	  if(i==M-1 && j==N-1) continue;
    	seed=ran(seed);
    	x=seed*1.0/2147483647;
	  if(x<p)
	  {
		  area[i][j]=1; //1代表有雷
		  mine++;//地雷数加1
	  }
	}
   }




return mine;
}


 





int main()
{
int i,j,k,seed,success_seed,round,unknownlast;
int success,trial;
int firstx,firsty;//第一次点击的行列坐标
int mine,plot,groupsize,minlast;
int start,end;
double lastrate,p,lastp;
int group[int((PMAX-PMIN)*M*N)+1]={0};
int groupsucceed[int((PMAX-PMIN)*M*N)+1]={0};
int grouptime[int((PMAX-PMIN)*M*N)+1]={0};
int groupclick[int((PMAX-PMIN)*M*N)+1]={0};
groupsize=int((PMAX-PMIN)*M*N)+1;
opps=0;

FILE *fp;
FILE *fq;
fp=fopen("data.txt","w");
fq=fopen("click.txt","w");

logictimes=0;
printf("输入一个种子值\n");
scanf("%d",&seed);
getcommon();

for(p=PMIN;p<=PMAX;p+=PSTEP)
{
 success=0;
for(trial=1;trial<=T;trial++)
{
  touchmine=0;//触雷设置为0
  seed=ran(seed);


  //seed=trial;



  mine=initial(p,seed); //设置地雷
 // if(mine!=99) continue;

  firstx=M/2;firsty=N/2;
  success_seed=seed;

  
 //边界放置为0
 
 // printf("一共有%d块雷，地雷率为%4.3f\n",mine,mine*1.0/M/N);
 // printarea();
  unknownmine = mine;
  j=1;//j是重复次数
  click=0; //点击次数
  start=clock();//开始计时

add1:
  seed = success_seed;
  exploit(firstx,firsty,1);
//  exploit(0,0,1);
 // exploit(0,N-1,1);
 // exploit(M-1,0,1);
//  exploit(M-1,N-1,1);
  //printexplore();
 // printsuspect();
 // printunknown();


  for(round=1; ;round++)
  {
     unknownlast=totalunknown;
//	 if(trial==1) {printf("round %d, mine %d ,the unknown is %d\n",round, unknownmine, totalunknown);printexplore();}  //试验板块

     cycle(1);//正式操作

  // printexplore();
  // printsuspect();
  // printunknown();
     
//	 if(lastrate < 0.2) {break;}
	 if(totalunknown==unknownmine)         { break;} //如果剩下的全是地雷，说明开采完毕
     if(totalunknown==unknownlast)         
	 {
		 cycle(2); 
	 }
	 if(totalunknown==unknownlast)         
	 {
		 cycle(3); 
	 }
     if(totalunknown==unknownlast)      
	 {
	   if(unknownmine<6 && clearall()==1)
	   {
		 
  	      cycle(1);
		  cycle(2);
		  cycle(3); 
	   }
	   else seed=randomchoice(seed);
   //   break;

	 }//如果本轮没有任何的开采，则表示探索失败
 
   
   

     if(touchmine==1) break;  //触雷，此游戏结束
  }
  



 lastrate=(totalunknown-unknownmine)*1.0/(M*N-mine);
 if(lastrate == 0 && j<REPEAT) {j++;goto add1;} //用于重复计算时间用
 end = clock();
//  printf("%d,%d,%d,%lf,%d\n",end,start,mine,lastrate,trial);
 plot=(mine-int(PMIN*M*N))/INTERVAL;
 if(plot>=0 && plot<=groupsize)  group[plot]++;
 //printf("剩余开采率=%lf\n",lastrate);
 if( lastrate <= JUSTIFY)
 {  
//	 printf("%lf,%d次,成功,地雷数%d,最高纪录%d,对应次数%d,%lf\n",p,trial,mine,minlast,k,lastp);
//	 if(mine>minlast) {minlast=mine;k=trial,lastp=p;}
//	 printf("地雷数目%d,用时%d到%d\n",mine,start,end);
	 success++;
	 if(lastrate>0 && unknownmine>1)
	 {
	//     printf("trial %d, 剩余地雷数%d,剩余模块%d\n",trial,unknownmine,totalunknown);
//		 printexplore();
	 }
	 if(totalunknown>2)
	 {
//	 printexplore();
	 }
     if(mine>minlast) {minlast=mine;k=trial;lastp=p;}
     if(plot>=0 && plot<groupsize)	 {groupsucceed[plot]++;grouptime[plot]+=(end-start);groupclick[plot]+=int(click);}

 }
 else 
 {
	 if(TEST==1)
	 {
 //       printf("%d,地雷数%d,剩余%lf,剩余雷数%d,失败,最高纪录%lf,对应%d\n",trial,mine,lastrate,unknownmine,minlast,j);
//	if(lastrate<minlast)  {minlast=lastrate;j=trial;}

  // 		printexplore();
//		printunknown();
//		printsuspect();
	 }
 }
   if(DISPLAY==1)
   {
    
	   if(lastrate==0) { system("cls");printf("Oh,yeah!扫雷成功!\n");display();}
	 else printf("扫雷失败\n");
   
   }



}
// printf("p=%lf时，成功率%lf\n",p,success*1.0/T);
   printf("完成进度%lf\n",(p-PMIN+PSTEP)/(PMAX-PMIN));
}
 for(i=0;i<groupsize;i++)
 {
	  // printf("%d  %d  %d\n",i,groupsucceed[i],group[i]);
	   if(group[i]!=0) {fprintf(fp,"%d\t%lf\t%lf\n", i*INTERVAL+int(PMIN*M*N), groupsucceed[i]*1.0/group[i],grouptime[i]*1.0/groupsucceed[i]/REPEAT);}
	   if(group[i]!=0) {fprintf(fq,"%d\t%d\t%lf\n", i*INTERVAL+int(PMIN*M*N),groupclick[i],groupclick[i]*1.0/groupsucceed[i]);}
	   if(group[i]!=0) {printf("%d\t%d\t%d\t%lf\t%lf\n", i*INTERVAL+int(PMIN*M*N), groupsucceed[i],group[i],groupsucceed[i]*1.0/group[i],grouptime[i]*1.0/groupsucceed[i]/REPEAT);}
  // 	   if(group[i]!=0) {printf("%d\t%d\t%lf\n", i+int(PMIN*M*N),groupclick[i],groupclick[i]*1.0/groupsucceed[i]);}
//	    if(group[i]!=0) {printf("%d\n",grouptime[i]);}
 }
 end = clock();
 printf("一共进行%d次逻辑，总用时%d毫秒\n",logictimes,end);
 printf("最后出现二选一次数为%d\n",opps);
return 0;
}
