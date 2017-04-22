#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "bmp.h"
#include <new>
using namespace std;

//定義平滑運算的次數
#define NSmooth 1000

/*********************************************************/
/*變數宣告：                                             */
/*  bmpHeader    ： BMP檔的標頭                          */
/*  bmpInfo      ： BMP檔的資訊                          */
/*  **BMPSaveData： 儲存要被寫入的像素資料               */
/*  **BMPData    ： 暫時儲存要被寫入的像素資料           */
/*********************************************************/
BMPHEADER bmpHeader;                        
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;                                               
RGBTRIPLE **BMPData = NULL;                                                   
RGBTRIPLE **LBMPSaveData = NULL;                                               
RGBTRIPLE **LBMPData = NULL;                               
RGBTRIPLE *PBMPData = NULL;// for scatter
RGBTRIPLE *PLBMPData = NULL;  //for local receiver
/*********************************************************/
/*函數宣告：                                             */
/*  readBMP    ： 讀取圖檔，並把像素資料儲存在BMPSaveData*/
/*  saveBMP    ： 寫入圖檔，並把像素資料BMPSaveData寫入  */
/*  swap       ： 交換二個指標                           */
/*  **alloc_memory： 動態分配一個Y * X矩陣               */
/*********************************************************/
int readBMP( char *fileName);        //read file
int saveBMP( char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory( int Y, int X );        //allocate memory

int main(int argc,char *argv[])
{
/*********************************************************/
/*變數宣告：                                             */
/*  *infileName  ： 讀取檔名                             */
/*  *outfileName ： 寫入檔名                             */
/*  startwtime   ： 記錄開始時間                         */
/*  endwtime     ： 記錄結束時間                         */
/*********************************************************/
	char *infileName = "input.bmp";
    char *outfileName = "outputMPI.bmp";
	double startwtime = 0.0, endwtime=0.0;
    int numprocs,myid;
// chen start parallel processing
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
// the  first one reads in the data and distributes data to all the nodes
    if(myid==0)
    {//讀取檔案 image data is in BMPData
        if ( readBMP( infileName) )
                cout << "Read file successfully!!" << endl;
        else 
                cout << "Read file fails!!" << endl;
// chen 2.Copy the first/last line of the image to the last/first line of the BMPData
      for(int i = 0; bmpInfo.biWidth ; i++)
         BMPData[0][i] = BMPData[bmpInfo.biHeight][i];
         
         int BHeight = bmpInfo.biHeight +1;
      for( int i = 0; bmpInfo.biWidth ; i++)
         BMPData[BHeight][i] = BMPData[1][i];
    }
// chen 3.  Compute the amount of data each node needs
//    3.1       basic operation lines  Nol = H/n  n is number of nodes
//    3.2       Aol= H%n  nodes need to do one additional operation lines; 
//             the first Aol nodes need to do Nol+1 lines of operations  
//basic number of operation lines in each node        
        int    BNol = bmpInfo.biHeight /numprocs; 
//the first Aol nodes have additional operation line        
        int    Aol = bmpInfo.biHeight %numprocs;
    
// chen compute # of lines to be processed of each node 
        int *Nol = new int[numprocs];
        for(int pc = 0; pc <numprocs; pc++)
        {    if (pc < Aol) Nol[pc] = BNol + 1; 
            else  Nol[pc] = BNol;
        }
// chen compute # of pixels of each node needed by MPI
	   int *Nop = new int[numprocs];
 	  for(int pc = 0; pc <numprocs; pc++)
		Nop[pc] = bmpInfo.biWidth* (Nol[pc]+2);
// chen compute starting address of each node needed by MPI   
        int *Sa = new int [numprocs];
        for(int pc = 0; pc <numprocs; pc++)
        {
            if (pc==0) Sa[pc] = 0;
            else Sa[pc] = Sa[pc-1]+bmpInfo.biWidth* (Nol[pc-1]);
        }


///////////////////////recv conts and recv start point.
        int *recvCounts=new int[numprocs];
        for(int pc = 0; pc <numprocs; pc++)
        recvCounts[pc] = bmpInfo.biWidth* Nol[pc];

        int *recvdispls=new int[numprocs];
        for(int pc = 0; pc <numprocs; pc++)
        {
            if (pc==0) recvdispls[pc] = 1*bmpInfo.biWidth;
            else recvdispls[pc] =recvdispls[pc-1]+ (Nol[pc-1])*bmpInfo.biWidth;
        }





    
// chen every node allocates memory here to receive the data
    PLBMPData = new RGBTRIPLE [bmpInfo.biWidth* (Nol[myid]+2)];
// get the receiver local pointer    
    for(int i = 0; i < (Nol[myid]+2); i++)
                LBMPData[i] = &PLBMPData[i*bmpInfo.biWidth];
   
// chen convert the starting memory pointer to a one-dimensional array pointer
        PBMPData = &BMPData[0][0];
//記錄開始時間
        startwtime = MPI_Wtime();
// chen scatter data
        MPI_Scatterv(PBMPData,Sa,Nop,MPI_CHAR, PLBMPData,bmpInfo.biWidth*(Nol[myid]+2),MPI_CHAR,0,MPI_COMM_WORLD);

    
// chen set barrier here
    MPI_Barrier(MPI_COMM_WORLD);
// chen begin to distribute data to each node

        //進行多次的平滑運算 chen start from 1 to the # of line to process
	for(int count = 1; count <= NSmooth ; count ++){

		//進行平滑運算
		// chen starting from 1 to Nol[myid] since we already have the boundary data
        for(int i = 1; i<=Nol[myid] ; i++)
			for(int j =0; j<bmpInfo.biWidth ; j++){
				/*********************************************************/
				/*設定上下左右像素的位置                                 */
				/*********************************************************/
				// chen do not need to check boundary condition
                int Top = i-1;
				int Down = i+1;
				int Left = j>0 ? j-1 : bmpInfo.biWidth-1;
				int Right = j<bmpInfo.biWidth-1 ? j+1 : 0;
				/*********************************************************/
				/*與上下左右像素做平均，並四捨五入                       */
				/*********************************************************/
				LBMPSaveData[i][j].rgbBlue =  (double) (LBMPData[i][j].rgbBlue+LBMPData[Top][j].rgbBlue+LBMPData[Down][j].rgbBlue+LBMPData[i][Left].rgbBlue+LBMPData[i][Right].rgbBlue)/5+0.5;
				LBMPSaveData[i][j].rgbGreen =  (double) (LBMPData[i][j].rgbGreen+LBMPData[Top][j].rgbGreen+LBMPData[Down][j].rgbGreen+LBMPData[i][Left].rgbGreen+LBMPData[i][Right].rgbGreen)/5+0.5;
				LBMPSaveData[i][j].rgbRed =  (double) (LBMPData[i][j].rgbRed+LBMPData[Top][j].rgbRed+LBMPData[Down][j].rgbRed+LBMPData[i][Left].rgbRed+LBMPData[i][Right].rgbRed)/5+0.5;
			}
        //把像素資料與暫存指標做交換
        swap(LBMPSaveData,LBMPData);
	}
// chen set barrier here
    MPI_Barrier(MPI_COMM_WORLD); 
// chen End of parallel processing and gether all the data bacl to the host
//MPI_Gatherv method
 /*   MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm);
 */   
     MPI_Gatherv(&LBMPSaveData[0][0], Nol[myid]*bmpInfo.biWidth, MPI_CHAR,
                &BMPData[0][0], recvCounts, recvdispls,
                MPI_CHAR, 0, MPI_COMM_WORLD);
//BMPData[0][0] is the output of MPI_Gatherv that acts as a receive buffer .
//and      

// Since you have allocated the BMPSaveData starting from the first line
        //BMPSaveData = &BMPData[1];
        //so, I think it's done. 

     


 	//寫入檔案
        if ( saveBMP( outfileName ) )
                cout << "Save file successfully!!" << endl;
        else
                cout << "Save file fails!!" << endl;
	
	//得到結束時間，並印出執行時間
        endwtime = MPI_Wtime();
    	cout << "The execution time = "<< endwtime-startwtime <<endl ;

 	free(BMPData);
 	free(BMPSaveData);
 	MPI_Finalize();

        return 0;
}

/*********************************************************/
/* 讀取圖檔                                              */
/*********************************************************/
int readBMP(char *fileName)
{
	//建立輸入檔案物件	
        ifstream bmpFile( fileName, ios::in | ios::binary );
 
        //檔案無法開啟
        if ( !bmpFile ){
                cout << "It can't open file!!" << endl;
                return 0;
        }
 
        //讀取BMP圖檔的標頭資料
    	bmpFile.read( ( char* ) &bmpHeader, sizeof( BMPHEADER ) );
 
        //判決是否為BMP圖檔
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
 
        //讀取BMP的資訊
        bmpFile.read( ( char* ) &bmpInfo, sizeof( BMPINFO ) );
        
        //判斷位元深度是否為24 bits
        if ( bmpInfo.biBitCount != 24 ){
                cout << "The file is not 24 bits!!" << endl;
                return 0;
        }

        //修正圖片的寬度為4的倍數
        while( bmpInfo.biWidth % 4 != 0 )
        	bmpInfo.biWidth++;
// chen read data into the BMPSaveData memory
        //動態分配記憶體給暫存空間
// chen 1.	New an (H+2)xW new buffer
        BMPData = alloc_memory( bmpInfo.biHeight+2, bmpInfo.biWidth);
// chen allocate the BMPSaveData starting from the first line
        BMPSaveData = &BMPData[1];
//        BMPSaveData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
        
        //讀取像素資料
    	//for(int i = 0; i < bmpInfo.biHeight; i++)
        //	bmpFile.read( (char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE));
	bmpFile.read( (char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);
	
        //關閉檔案
        bmpFile.close();
 
        return 1;
 
}
/*********************************************************/
/* 儲存圖檔                                              */
/*********************************************************/
int saveBMP( char *fileName)
{
 	//判決是否為BMP圖檔
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
        
 	//建立輸出檔案物件
        ofstream newFile( fileName,  ios:: out | ios::binary );
 
        //檔案無法建立
        if ( !newFile ){
                cout << "The File can't create!!" << endl;
                return 0;
        }
 	
        //寫入BMP圖檔的標頭資料
        newFile.write( ( char* )&bmpHeader, sizeof( BMPHEADER ) );

	//寫入BMP的資訊
        newFile.write( ( char* )&bmpInfo, sizeof( BMPINFO ) );

        //寫入像素資料
        //for( int i = 0; i < bmpInfo.biHeight; i++ )
        //        newFile.write( ( char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE) );
        newFile.write( ( char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );

        //寫入檔案
        newFile.close();
 
        return 1;
 
}


/*********************************************************/
/* 分配記憶體：回傳為Y*X的矩陣                           */
/*********************************************************/
RGBTRIPLE **alloc_memory(int Y, int X )
{        
	//建立長度為Y的指標陣列
        RGBTRIPLE **temp = new RGBTRIPLE *[ Y ];
	RGBTRIPLE *temp2 = new RGBTRIPLE [ Y * X ];
        memset( temp, 0, sizeof( RGBTRIPLE ) * Y);
        memset( temp2, 0, sizeof( RGBTRIPLE ) * Y * X );

	//對每個指標陣列裡的指標宣告一個長度為X的陣列 
        for( int i = 0; i < Y; i++){
                temp[ i ] = &temp2[i*X];
        }
 
        return temp;
 
}
/*********************************************************/
/* 交換二個指標                                          */
/*********************************************************/
void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;
}


