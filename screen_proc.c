#include "jpeg.h"
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h> //子プロセスを終わらせる用
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/select.h>
#include <sys/types.h>
#include <tslib.h>
#include <unistd.h>

#include "font.h"
#include "clock_B.h"

//***********
//マクロ定義
//***********
#define MMAP "/dev/fb0" //メモリマップ
#define FONTFILE "/home/pi/umefont_470/ume-pgc4.ttf"
#define SCREENWIDTH 800
#define SCREENHEIGHT 480
#define SCREENSIZE ( SCREENWIDTH * SCREENHEIGHT * 4 )
//色表現    RGB（red,green,yellow)。最大値（255,255,255）
#define RGB( r, g, b ) ( ( ( r ) << 16 ) | ( ( g ) << 8 ) | ( b ) )
#define rgb( r, g, b ) ( ( ( r ) << 11 ) | ( ( g ) << 5 ) | ( b ) )


#include "draw.h" //ターゲットボードに描画用


void screen_proc( void ) {

    //スクリーン用のフレームバッファ。(difは変化するものを記録、backは背景用.bufは最終的な形）
    unsigned long *pfb,*pfb_Normal,*pfb_Alarm,*pfb_NoAlarm;
	
    //画面表示用
	int fd,ret,i,w,x=10,z=0;
	
	char  AlarmSetTime[9]="00:00:00\0";

	unsigned char timer[]="ON";
	unsigned char timer1[]="/";
	unsigned char timer2[]="OFF";
	unsigned char timerset[]="OK";
	unsigned char timerset1[]=" ";

	//メッセージキューのKey
	int   msgid;
	key_t msgkey;
	buf_t buf;
	
	//共有メモリのKey生成用
 	key_t shmkey;
	int shmid;
	//共有メモリのアドレス
	ShareMem * psh;	


	//**********//
	//共有メモリ//
	//**********//

	//共有メモリのkeyはclock,sとしています（mainに合わせてください）
	if((shmkey=ftok("clock_B",'s')) == -1){
		perror("gTouch:ftok");
		_exit(1);
	}
	if((shmid=shmget(shmkey,sizeof(char),0666|IPC_CREAT)) == -1){
		perror("gTouch:msgget");
		_exit(1);
	}
	psh = shmat(shmid,0,0);


	// mmapデバイスのファイルを開く
	fd = open( MMAP, O_RDWR );

	// mmapによりバッファの先頭アドレスを取得
	pfb = mmap( 0, SCREENSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );

	//仮想メモリバッファのメモリ確保
	pfb_Normal = malloc(SCREENSIZE);
	pfb_Alarm = malloc(SCREENSIZE);
	pfb_NoAlarm = malloc(SCREENSIZE);


    // msgsendはこのプロセスのメインファイル名
    // Key取得・生成
    if( ( msgkey = ftok( "clock_B", 'b' ) ) == -1 ) { //教科書通り
        //エラー表示
        perror( "ftok" );
        return;
    }
    //メッセージキューのIDを生成・取得
    if( ( msgid = msgget( msgkey, 0666 | IPC_CREAT ) ) == -1 ) {
        //エラー表示
        perror( "msgget" );
        return;
    }

//------------------
//eNormal　初期画面での設定
//------------------
	psh->CS =  eNormal;//時計画面に

	printf("sc%s	1\n",AlarmSetTime);	//アラーム時刻保管用テスト

	//アラーム記録を初期化
	sprintf(AlarmSetTime,"%02d:%02d:%02d\n",psh->AlarmTime_h,psh->AlarmTime_m,psh->AlarmTime_s);



//************//
//メインループ//
//************//
	while( 1 ) {

       // メッセージを受けた時
        	// msgtypeは0にセット（はじめに送られてきたものを拾う）
        	if( msgrcv( msgid, &buf,sizeof(buf), eScreen, 0 ) == -1 ) {
           	perror( "msgrcv" );
            	return;
        	}

		//アタッチ
		psh = shmat(shmid,0,0);
		
//共有メモリを確認。アラーム画面のときの描画
		if(psh->CS == eAlarm ){
			load_jpeg( "alarm.jpg", pfb_Alarm,0,0, SCREENWIDTH, SCREENHEIGHT ); //アラーム画像（アラーム画面仮)

			//アラーム時刻描画処理
			x=112;//文字幅を設定
			for(i=0;i<8;i++){	
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_Alarm,FONTFILE,AlarmSetTime[i],230,x,90,RGB(0UL,0UL,0UL),RGB(0UL,0UL,0UL));
				x += w +5;
			}
			//時計に戻るボタンの描画
			load_jpeg( "toNormal.jpg", pfb_Alarm, 695,0,SCREENWIDTH, SCREENHEIGHT); //

			memcpy(pfb,pfb_Alarm,SCREENSIZE);//背景画像をpfbに渡す
		}
		//アラーム設定ボタンを押された場合
		if( buf.flag == eAlarmSet_h || buf.flag == eAlarmSet_m || buf.flag == eAlarmSet_s){
			load_jpeg( "alarm.jpg", pfb_Alarm, 0,0,SCREENWIDTH, SCREENHEIGHT ); //アラーム画像（アラーム画面仮)

			//アラーム時刻描画処理
			x=112;//文字幅を設定
			for(i=0;i<8;i++){	
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_Alarm,FONTFILE,buf.UN_RAW.mtext[i],230,x,90,RGB(0UL,0UL,0UL),RGB(0UL,0UL,0UL));
				AlarmSetTime[i] = buf.UN_RAW.mtext[i];
				x += w +5;
			}
			memcpy(pfb,pfb_Alarm,SCREENSIZE);//背景画像をpfbに渡す
		}

//共有メモリを確認。時計画面のときの描画
		if(psh->CS == eNormal){
			load_jpeg( "sea.jpg", pfb_Normal, 0,0,SCREENWIDTH, SCREENHEIGHT ); //海中画像（時計画面仮）

			//時刻描画処理
			x=112;//文字幅を設定
			for(i=0;i<8;i++){
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_Normal,FONTFILE,buf.UN_RAW.mtext[i],230,x,90,RGB(255UL,255UL,255UL),RGB(255UL,255UL,255UL));
				x += w +5;
			}

			//アラーム描画処理
			x=10;//文字幅を設定
			for(i=0;i<5;i++){
				
				//pfb_Normalに時刻表示データを渡す
				//共有メモリを参照
				w = put_char(pfb_Normal,FONTFILE,AlarmSetTime[i],75,x,5,RGB(0UL,0UL,0UL),RGB(0UL,0UL,0UL));
				x += w +2;
			}

			//アラームON/OFF
			x=200;
			for(i=0;i<2;i++){
				
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_Normal,FONTFILE,timer[i],50,x,10,RGB(0UL,0UL,0UL),RGB(0UL,0UL,0UL));
				x += w +5;
			}
			for(i=0;i<1;i++){
				
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_Normal,FONTFILE,timer1[i],50,x,10,RGB(255UL,255UL,255UL),RGB(255UL,255UL,255UL));
				x += w +5;
			}
			for(i=0;i<3;i++){
				
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_Normal,FONTFILE,timer2[i],50,x,10,RGB(50UL,50UL,50UL),RGB(50UL,50UL,50UL));
				x += w +5;
			}
			//pfbに渡す
			memcpy(pfb,pfb_Normal,SCREENSIZE);
		}

		//共有メモリを確認。アラームOFFの時計画面のときの描画
		else if(psh->CS == eNoAlarm){
			load_jpeg( "sea.jpg", pfb_NoAlarm,0,0, SCREENWIDTH, SCREENHEIGHT ); //海中画像（時計画面仮）

			//時刻描画処理
			x=112;//文字幅を設定
			for(i=0;i<8;i++){
				
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_NoAlarm,FONTFILE,buf.UN_RAW.mtext[i],230,x,90,RGB(255UL,255UL,255UL),RGB(255UL,255UL,255UL));
				x += w +5;
			}
			//アラームON/OFF
			x=200;
			for(i=0;i<2;i++){
				
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_NoAlarm,FONTFILE,timer[i],50,x,10,RGB(50UL,50UL,50UL),RGB(50UL,50UL,50UL));
				x += w +5;
			}
			for(i=0;i<1;i++){
				
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_NoAlarm,FONTFILE,timer1[i],50,x,10,RGB(255UL,255UL,255UL),RGB(255UL,255UL,255UL));
				x += w +5;
			}
			for(i=0;i<3;i++){
				
				//pfb_Normalに時刻表示データを渡す
				w = put_char(pfb_NoAlarm,FONTFILE,timer2[i],50,x,10,RGB(0UL,0UL,0UL),RGB(0UL,0UL,0UL));
				x += w +5;

			}
			//pfbに渡す
			memcpy(pfb,pfb_NoAlarm,SCREENSIZE);
		}


	}
	//動的メモリを解放
	free(pfb_Normal);
	free(pfb_Alarm);
	free(pfb_NoAlarm);

	shmdt(psh);	//デタッチ
	if( shmctl( shmid, IPC_RMID, NULL ) == -1 ) {
        	perror( "msgctl" );
        	return;
	}


	if( msgctl( msgid, IPC_RMID, NULL ) == -1 ) {
        	perror( "msgctl" );
        	return;
	}
}
