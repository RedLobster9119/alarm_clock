/***************************************************
 * タッチパネル検知ヘッダ
 * tslibが必要です
 * コンパイル時は-ltsを付けてください
 *
 * 追加手順： 
 *	・E_BTNにボタン名を好きな名前で追加する
 *	・stLocに追加したいボタンの状態・上で追加したボタン名・
 *	  ボタンの左上の座標(x,y)・ボタンの大きさ(x,y)を追記
 *	・mainへはE_BTNを送るのでその値を見て処理を書いてください
 *	・テスト用のボタン名や座標配列は削除してかまいません
 *
 ***************************************************/

//================================================
// Enums
//================================================

//ボタンの種類を箇条書き（機能ごとに並べる）
typedef enum{	
/*** ここにボタン名を追加してください ***/
	E_BTN_UpLf,
	E_BTN_UpRi,
	E_BTN_DwLf,
	E_BTN_DwRi,	
	E_BTN_toAlarm,
	E_BTN_toNormal,
	E_BTN_tonoAlarm,
	E_BTN_toyesAlarm,
	E_BTN_setAlarm_h,
	E_BTN_setAlarm_m,
	E_BTN_setAlarm_s,
	E_BTN_MAX = 255,
} E_BTN;

//================================================
// Struct 
//================================================
typedef struct{
	int		cs;	//ClockState(現在の画面状態)
	E_BTN		btn;	//上にあるenumボタン
	unsigned short	xref;
	unsigned short	yref;
	unsigned short	xsize;
	unsigned short	ysize;	
} ST_LOCATE;


extern void tpanel_main(void);
