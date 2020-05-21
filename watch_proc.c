#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include "clock_B.h"		// ① メインヘッダをインクルードする

void tmh(int signum);

void watch_main(void)		// ② プロセス名_main()に書き換える
{	
	int msgid;
	key_t msgkey;

    buf_t buf;				// ③ メッセージの構造体(共通)の宣言

	time_t t;

	struct tm tm;


	if((msgkey=ftok("clock_B",'b'))==-1){	// ④キー生成
		perror("ftok");
		return ;
	}

	if((msgid=msgget(msgkey,0666 | IPC_CREAT))==-1){//メッセージキューの生成・取得
		perror("msgget");
		return ;
	}

	//インターバルタイマー
	struct sigaction sig={0};
	struct itimerval val;
	sig.sa_handler=tmh;
	sigaction(SIGALRM,&sig,NULL);
	val.it_interval.tv_sec=val.it_value.tv_sec=0;	// 0秒スタート
	val.it_interval.tv_usec=val.it_value.tv_usec=300000;	// 0.3秒間隔（30fpsくらいをイメージ）
	setitimer(ITIMER_REAL,&val,NULL);


	while(1){
		pause(); // 基本待ち。シグナルを受けると待ち解除 今回は１秒間隔

		t = time(NULL);
		localtime_r(&t, &tm);

		//メッセージバッファに現在時刻を記録
		buf.UN_RAW.time.hh = tm.tm_hour;
		buf.UN_RAW.time.mm = tm.tm_min;
		buf.UN_RAW.time.ss = tm.tm_sec;

		
		buf.mtype=eMain;	//送り先設定
		buf.flag=eUpdateTime;	

		if(msgsnd(msgid,&buf,sizeof(buf),0)==-1){//メッセージの送信
			perror("msgsnd");
			return ;
		}
	}
	return ;
}

//インターバルタイマー
void tmh(int signum)
{
	return;
}
