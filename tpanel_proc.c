
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <tslib.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include "clock_B.h"
#include "tpanel_proc.h"

//================================================
// マクロ定義
//================================================
#define D_DEVNAME "/dev/input/event0"


//================================================
// Const Tables
//================================================

//ボタン一覧表
/* 配列数の変更を忘れないこと１回目 */
const ST_LOCATE sttchLoc[11] = {

//	{現在の画面状態		,ボタン名	,x基準		,y基準  ,サイズx(横幅） ,サイズy（縦幅）}
  
	{0			,E_BTN_UpLf	          ,0	    ,0	  ,400	  ,240	},	//左上
	{0			,E_BTN_UpRi	          ,400		,0	  ,400  	,240	},	//右上
	{0			,E_BTN_DwLf	          ,0		  ,240  ,400	  ,240	},	//左下
	{0			,E_BTN_DwRi         	,400		,240	,400	  ,240	},	//右下	------」ここまでテスト用ボタン
	{eNormal		,E_BTN_toAlarm	  ,0		  ,0	  ,200		,100	},	//アラーム画面に移動
	{eAlarm			,E_BTN_toNormal	  ,700	 	,0	  ,100		,100	},	//時計画面に移動
	{eAlarm			,E_BTN_setAlarm_h	,90	    ,90	  ,200		,230	},	//アラームをセット（hour)
	{eAlarm			,E_BTN_setAlarm_m	,370	  ,90	  ,200		,230	},	//アラームをセット（min)
	{eAlarm			,E_BTN_setAlarm_s	,610	  ,90	  ,200		,230	},	//アラームをセット（sec)
	{eNormal		,E_BTN_tonoAlarm	,200	  ,0	  ,200		,60	  },	//アラームなし画面に移動
	{eNoAlarm		,E_BTN_toyesAlarm	,200	  ,0	  ,200		,60	  },	//アラームなしから時計画面に移動

/* 配列数の変更を忘れないこと2回目 */
};	



//================================================
// Global functions
//================================================
void tpanel_main(void){
	int	i,ret,msgid,shmid;
	key_t 	msgkey,shmkey;
	struct	tsdev* ts;
	struct	ts_sample samp;
	//タッチボタン（enum)
	E_BTN	eBtn;
	//メッセージの構造体
	buf_t	bufMsg = {0};

	//共有メモリのアドレス
	ShareMem	*psh;

	bufMsg.mtype = eScreen;

	ts = ts_open(D_DEVNAME,0);
	ret = ts_config(ts);

	if(-1 == (msgkey=ftok("clock_B",'b'))){
		perror("gTouch:ftok");
		_exit(1);
	}
	if(-1 == (msgid=msgget(msgkey,0666|IPC_CREAT))){
		perror("gTouch:msgget");
		_exit(1);
	}
	//共有メモリのkeyはclock,sとしています（mainと合わせること）
	if(-1 == (shmkey=ftok("clock_B",'s'))){
		perror("gTouch:ftok");
		_exit(1);
	}
	if(-1 == (shmid=shmget(shmkey,sizeof(char),0666|IPC_CREAT))){
		perror("gTouch:msgget");
		_exit(1);
	}


 	while(1){

		//共有メモリをアタッチ	
		psh = shmat(shmid,0,0);
		/* 送信データ初期化 */
		eBtn = E_BTN_MAX;

		/* タッチ座標取得（離した点を検知） */
		ret = ts_read(ts,&samp,1);
		if(255 == samp.pressure)continue;


		/* 取得座標がどのボタンに対応するか確認 */
		for(i=0; i<(sizeof(sttchLoc)/sizeof(ST_LOCATE));i++){
			//共有メモリの確認
			if(sttchLoc[i].cs == psh->CS){	//共有メモリ内の画面状態を参照
				//座標確認（x座標）
				if((samp.x >= sttchLoc[i].xref) && (samp.x < (sttchLoc[i].xref+sttchLoc[i].xsize))){
					//座標確認（y座標）
					if((samp.y >= sttchLoc[i].yref) && (samp.y < (sttchLoc[i].yref+sttchLoc[i].ysize))){
						
						eBtn = sttchLoc[i].btn;
						break;
					}
				}
			}
		}

		//ボタンが有効であるとき	※判定方法（画面が押下→リリースされたとき）
		if(E_BTN_MAX != eBtn){

			//宛先指定（main_procへ）
			bufMsg.mtype = eMain;
			
			//左上のアラーム設定画面に進むボタンが押されたとき
			if(eBtn == E_BTN_toAlarm){
				bufMsg.flag = eToAlarm;	
				psh->CS = eAlarm;
				//メッセージ送信
				*(int *)&bufMsg.UN_RAW.mtext[0] = (int)eBtn;
				if(-1 == msgsnd(msgid, &bufMsg, sizeof(bufMsg)-sizeof(long), 0)){//この処理でうまくいきますが、再考の余地あり
					perror("g:msgsnd");
					continue;
				}
				
			}
			//右上の時計画面に進むボタンが押されたとき
			else if(eBtn == E_BTN_toNormal){
				bufMsg.flag = eToNormal;
				psh->CS =  eNormal;
				//メッセージ送信
				*(int *)&bufMsg.UN_RAW.mtext[0] = (int)eBtn;
				if(-1 == msgsnd(msgid, &bufMsg, sizeof(bufMsg)-sizeof(long), 0)){
					perror("g:msgsnd");
					continue;
				}	
			}
			
			//offボタンが押されたとき(アラーム）
			else if(eBtn == E_BTN_tonoAlarm){
				bufMsg.flag = eToNoAlarm;
				psh->CS =  eNoAlarm;
				//メッセージ送信
				*(int *)&bufMsg.UN_RAW.mtext[0] = (int)eBtn;
				if(-1 == msgsnd(msgid, &bufMsg, sizeof(bufMsg)-sizeof(long), 0)){
					perror("g:msgsnd");
					continue;
				}	
			}
			//onボタンが押されたとき(アラーム）
			else if(eBtn == E_BTN_toyesAlarm){
				bufMsg.flag = eToNormal;
				psh->CS =  eNormal;	
				//メッセージ送信
				*(int *)&bufMsg.UN_RAW.mtext[0] = (int)eBtn;
				if(-1 == msgsnd(msgid, &bufMsg, sizeof(bufMsg)-sizeof(long), 0)){
					perror("g:msgsnd");
					continue;
				}
			}
			
			// ※ 時計画面のとき
			if(psh->CS == eAlarm){

				// アラーム画面のhourを触ったとき
				if(eBtn == E_BTN_setAlarm_h){
					bufMsg.flag = eAlarmSet_h;	//インクリメントの報告
					printf("touch_h\n");
					//メッセージ送信
					*(int *)&bufMsg.UN_RAW.mtext[0] = (int)eBtn;
					if(-1 == msgsnd(msgid, &bufMsg, sizeof(bufMsg)-sizeof(long), 0)){
						perror("g:msgsnd");
						continue;
					}
				}
				// アラーム画面のminを触ったとき
				else if(eBtn == E_BTN_setAlarm_m){
					bufMsg.flag = eAlarmSet_m;
					printf("touch_m\n");
					//メッセージ送信
					*(int *)&bufMsg.UN_RAW.mtext[0] = (int)eBtn;
					if(-1 == msgsnd(msgid, &bufMsg, sizeof(bufMsg)-sizeof(long), 0)){
						perror("g:msgsnd");
						continue;
					}
				}

				// アラーム画面のsecを触ったとき
				else if(eBtn == E_BTN_setAlarm_s){
					bufMsg.flag = eAlarmSet_s;
					printf("touch_s\n");
					//メッセージ送信
					*(int *)&bufMsg.UN_RAW.mtext[0] = (int)eBtn;
					if(-1 == msgsnd(msgid, &bufMsg, sizeof(bufMsg)-sizeof(long), 0)){
						perror("g:msgsnd");
						continue;
					}
				}
			}
		}
		
	}
	shmdt(psh);	//共有メモリのでタッチ、共有メモリの削除
	if( shmctl( shmid, IPC_RMID, NULL ) == -1 ) {
        	perror( "msgctl" );
        	return;
	}
	return;
}
