#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>

#include "clock_B.h"//メインプロセス

/*それぞれのピンの入出力及び値読み取りのdefine部、ピンを増やすごとに追記　（現状 GPIO21ポートのみ）*/
#define D_DI21 "/sys/class/gpio/gpio21/direction"
#define D_VA21 "/sys/class/gpio/gpio21/value"



/*プロトタイプ宣言*/
int V_read(void);	//GPIO21番のValueの値を読み出す関数
int P_make(void);	//GPIO21番のPortファイルを作成するプログラム
void sw_main(void);	//SWのメインプログラム



/*************************************************
*	ポート作成関数（現状 GPIO21ポートのみ）
*************************************************/

int P_make(void){
/*echo catコマンドの一時置き場*/
	char cmd[100];
/*GPIO21ポート作成*/
	sprintf(cmd,"echo 21 > /sys/class/gpio/export");
	system(cmd);
/*GPIO21ポートの入力検出設定*/
	sprintf(cmd,"echo in > D_DI21");
	system(cmd);
}

/*************************************************
*	メイン関数
*************************************************/

void sw_main(void)
{
	int old_v=0, new_v=0;	//old_v:1つ前のスイッチの値、new_v:現在のスイッチの値 (OFF=0、ON=1)
				//old_v=0,new_v=1(スイッチが押された瞬間)になった時にmsgメインプロセスに送る



//////////////メッセージ設定////////////////

/*メッセージキュー用変数*/
	int msgid;//メッセージID
	key_t msgkey;//メッセージキー
	buf_t buf;

/*ポート作成*/
	P_make();

/*メッセージキーの初期設定*/
	if( ( msgkey = ftok( "clock_B", 'b' ) ) == -1 ) { //キー生成
        	//エラー表示
        	perror( "ftok" );
        	return;
	}

/*メッセージキューのIDを生成・取得*/
	if( ( msgid = msgget( msgkey, 0666 | IPC_CREAT ) ) == -1 ) {//メッセージキューの生成・取得
        	//エラー表示
        	perror( "msgget" );
        	return;
	}

/////////////メッセージ設定おわり////////////



//////////SW処理メインループ/////////////////////

	while(1){
		new_v = V_read();	//都度ファイルディスクリプタをCLOSEしないと値が変わらなかったので関数化

		if((new_v - old_v) == 1 ){		
			// 0から1に値が変わった時(スイッチが押された瞬間)にメッセージを送る
			// msgtypeは1にセット（はじめに送られてきたものを拾う）
			buf.mtype = eMain;	//メッセージのtype
			buf.flag = eSwOn;	//メッセージのflag
			if( msgsnd(  msgid, &buf, sizeof(buf), 1 ) == -1 ) { //メッセージ送信
				//エラー表示
				perror( "msgsnd" );
			}
			usleep(200000);	//チャタリング対策(0.2秒)
		}
		old_v = new_v;		//old_vに現在のスイッチ(new_v)の値を渡す
		usleep(10000);		//信号速度が速すぎるので制御(0.01秒)
	}
	return;
}


//////////SW処理メインループ終わり/////////////////////



/**********************************************************************
*	Valueの値読み出しをnew_vに出力する関数（現在 GPIO21ポートのみ）
**********************************************************************/

int V_read(void){
	int fd_va;	//GPIO21のvalue用ファイルディスクリプタ
	int value1;	//GPIO21のvalue値保管用変数

/*GPIO21のファイルディスクリプタを保管*/
	if((fd_va=open(D_VA21,O_RDONLY))==-1){//*GPIO21のファイルディスクリプタを開く
		//エラー表示
		printf("入力検出エラー");
		return 1;
	}
	read(fd_va,&value1,sizeof(value1));	//value値保管用変数にGPIO21のvalue用ファイルディスクリプタの値を読み込み
	close(fd_va);				//GPIO21のvalue用ファイルディスクリプタを閉じる
	return value1;
}
