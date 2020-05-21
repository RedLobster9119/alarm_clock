//--------------------------------------------------------------------------------------------------
//	clock_B.cを軸に、マルチプロセス処理（共有メモリ、メッセージキュー）を用いて目覚まし時計の
//	開発を行う。
//	main関数の仕事は、各プロセスを実行させる（子プロセスを生成）のみとする。
//	入出力の管理や処理は、main_proc関数内で行う。
//	
//--------------------------------------------------------------------------------------------------


#include <stdio.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <unistd.h>
#include <wait.h>
#include <sys/shm.h>
#include <string.h>

#include "clock_B.h"
#include "screen_proc.h"
#include "sound_proc.h"
#include "swinout_proc.h"
#include "watch_proc.h"
#include "tpanel_proc.h"

//***********
//マクロ定義
//***********


//kill用追記
struct sigaction sig = {0};

//wait用
int st;
//プロセスIDの記録用変数
pid_t  pid_main_proc;
pid_t  pid_sound_proc;
pid_t  pid_screen_proc;
pid_t  pid_swinout_proc;
pid_t  pid_watch_proc;
pid_t  pid_tpanel_proc;


//----------------
//プロトタイプ宣言
//----------------
void main_proc(void);//ここで処理を行う
void inthandler(int signum);

//----------------------------------------------------------------------------------------------------------------//
//****************************************************************************************************************//
//						エントリーポイント						　//
//****************************************************************************************************************//
//----------------------------------------------------------------------------------------------------------------//
int main( void ) {

//割り込み処理	
	//ハンドラ関数を設定
	sig.sa_handler= inthandler;	
	//シグナルintを受けたら、関数inthandlerを呼び出す（教科書通り）
	sigaction(SIGINT,&sig,NULL);


// ==============================
// 以下に子プロセスを記載する
// ==============================

    //　メインプロセス（管理）
	if((pid_main_proc=fork())==0){
	main_proc();
	}


    //　スクリーンプロセス（出力）
	else if((pid_screen_proc=fork())==0){
        screen_proc();
	}


    //　サウンドプロセス（出力）
	else if((pid_sound_proc=fork())==0){
        	sound_main();
	}


    //　時計プロセス（入力）
	else if( (pid_watch_proc=fork()) == 0 ) {
		watch_main();
	}


//　タッチパネルプロセス（入力）
	else if((pid_tpanel_proc=fork())==0){
		tpanel_main();
	}


    //　SWプロセス（入力・（出力））
        else if( (pid_swinout_proc=fork()) == 0 ) {
		sw_main();
	}

// == 子プロセスここまで=========

	//割り込み待ち
	wait(&st);
    
	return 0;
}




//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝//
// 		メインプロセス	(各プロセスの管理、処理を担当。）
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝//
void main_proc(void){

	//メッセージキューのid, key, buffer
	int   shmid,msgid;
	key_t msgkey,shmkey;
	buf_t buf,sendbuf;			//bufはrcv用、sndbufはsnd用

	//共有メモリのアドレス
	ShareMem	*psh;
	
	//共有メモリ内のアラーム設定時刻を0で初期化
	psh->AlarmTime_h = psh->AlarmTime_m = psh->AlarmTime_s = 0;


//共有メモリ設定//
	//共有メモリのkeyはclock,sとしています（mainに合わせてください）
	if(-1 == (shmkey=ftok("clock_B",'s'))){
		perror("gTouch:ftok");
		_exit(1);
	}
	if(-1 == (shmid=shmget(shmkey,sizeof(char),0666|IPC_CREAT))){
		perror("gTouch:msgget");
		_exit(1);
	}
	//共有メモリをアタッチ	
	psh = shmat(shmid,0,0);


//メッセージ設定//
    // msgsendはこのプロセスのメインファイル名
    // Key取得・生成
    if( ( msgkey = ftok( "clock_B", 'b' ) ) == -1 ) { //教科書通り
        //エラー表示
        perror( "ftok" );
        return ;
    }
    //メッセージキューのIDを生成・取得
    if( ( msgid = msgget( msgkey, 0666 | IPC_CREAT ) ) == -1 ) {
        //エラー表示
        perror( "msgget" );
        return ;
    }


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝

//	本来は、ここではなく、処理に合わせた場所に挿入すること。（sound処理）


	// for 音チームテスト用
    buf.mtype = eSound; //(clock_B.hに記述あり)
	buf.flag = 1; //サウンドのフラッグ　(sound_proc.c内 -> if(buf.flag == 1){)
    if( msgsnd( msgid, &buf, sizeof(buf), 1 ) == -1 ) {
        perror( "msgsnd" );
        return ;
    }

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝



	
//************************//
//	メインループ	　//
//************************//

	while(1){

		// msgtypeは0にセット（はじめに送られてきたものを拾う）
		if( msgrcv( msgid, &buf,sizeof(buf), eMain, 0 ) == -1 ) {
			perror( "msgrcv" );
			return;
		}

		//-----------		
		//時刻の通知
		//-----------
		if(buf.flag == eUpdateTime){
			//送る用
	   		sendbuf.mtype = eScreen;
			sendbuf.flag = eDispTimeNow;
			sprintf(sendbuf.UN_RAW.mtext,"%02d:%02d:%02d\n",buf.UN_RAW.time.hh,buf.UN_RAW.time.mm, buf.UN_RAW.time.ss);

			if( msgsnd( msgid, &sendbuf, sizeof(sendbuf), 1 ) == -1 ) {
				perror( "msgsnd" );
				return ;
			}
		}


		//--------------------
		//アラーム画面の設定
		//--------------------

		if(buf.flag == eAlarmSet_h){	//hourを変更
			//宛先、フラグを指定
	   		sendbuf.mtype = eScreen;
			sendbuf.flag = eAlarmSet_h;
			psh->CS = eAlarm;	//画面状態を指定
			
			//アラーム記録変数の処理
			psh->AlarmTime_h++;//１増やす
			psh->AlarmTime_h %= 24;	//0～23の値に抑える
			
			//mtextに書き込み処理
			sprintf(sendbuf.UN_RAW.mtext,"%02d:%02d:%02d\n",psh->AlarmTime_h,psh->AlarmTime_m, psh->AlarmTime_s);
			printf("B_eAlarmSet_h\n");
			
			if( msgsnd( msgid, &sendbuf, sizeof(sendbuf), 1 ) == -1 ) {
				perror( "msgsnd" );
				return ;
			}
		}
		else if(buf.flag == eAlarmSet_m){	//minを変更
			//宛先、フラグを指定
	   		sendbuf.mtype = eScreen;
			sendbuf.flag = eAlarmSet_m;
			psh->CS = eAlarm;
			psh->AlarmTime_m++;//１増やす
			psh->AlarmTime_m %= 60;//0～60
			//mtextに書き込み処理
			sprintf(sendbuf.UN_RAW.mtext,"%02d:%02d:%02d\n",psh->AlarmTime_h,psh->AlarmTime_m, psh->AlarmTime_s);
	
			if( msgsnd( msgid, &sendbuf, sizeof(sendbuf), 1 ) == -1 ) {
				perror( "msgsnd" );
				return ;
			}
		}

		else if(buf.flag == eAlarmSet_s){	//secを変更
			//宛先、フラグを指定
	   		sendbuf.mtype = eScreen;
			sendbuf.flag = eAlarmSet_s;
			psh->CS = eAlarm;
			psh->AlarmTime_s++;//１増やす
			psh->AlarmTime_s %= 60;	//0～60
			//mtextに書き込み処理
			sprintf(sendbuf.UN_RAW.mtext,"%02d:%02d:%02d\n",psh->AlarmTime_h,psh->AlarmTime_m, psh->AlarmTime_s);
			printf("B_eAlarmSet_s\n");
			if( msgsnd( msgid, &sendbuf, sizeof(sendbuf), 1 ) == -1 ) {
				perror( "msgsnd" );
				return ;
			}
		}	



		//以下のelse if4つはテスト用。処理を加える場合は、上記のメッセージ通信を真似して作る。
		else if(buf.flag == eToAlarm){
			printf("to Alarm画面\n");
		}
		else if(buf.flag == eToNormal){
			printf("to 時計画面\n");
		}
		else if(buf.flag == eToNoAlarm){
			printf("to ノーアラーム時計画面\n");
		}

		else if(buf.flag == eSwOn){
			printf("Sw On\n");
		}

	}

	shmdt(psh);	//共有メモリ終了、削徐
	if( shmctl( shmid, IPC_RMID, NULL ) == -1 ) {
        	perror( "msgctl" );
        	return;
	}

	//メッセージキュー削除
	if( msgctl( msgid, IPC_RMID, NULL ) == -1 ) {
        	perror( "msgctl" );
        	return;
	}

	return;
}

//kill用のシグナル
void inthandler(int signum){
	kill(pid_main_proc,SIGTERM);
	kill(pid_screen_proc,SIGTERM);
	kill(pid_sound_proc,SIGTERM);
	kill(pid_swinout_proc,SIGTERM);
	kill(pid_watch_proc,SIGTERM);
	kill(pid_tpanel_proc,SIGTERM);
}
