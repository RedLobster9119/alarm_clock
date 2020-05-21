//===========================================
// 画面の状態を表す
//	(共有メモリで作成）/30
//	画面状態によって、ボタンの機能が変わる。
//	mainに入れると便利
//===========================================
enum ClockState {
	eNormal=1,	//時計画面（時刻表記）
	eAlarm,		//アラーム画面
	eNoAlarm,	//アラームなしノーマル画面
//	eAlarmRing,	
	eAlarmSet,
	eGame
};

typedef struct{
	int AlarmTime_h;	//アラーム設定時間（hour)
	int AlarmTime_m;	//アラーム設定時間（minute)
	int AlarmTime_s;	//アラーム設定時間（second)
	
	enum ClockState CS;	//目覚まし時計の画面状態を列挙 
}ShareMem;






//===========================================
// メッセージType（宛先一覧）
//===========================================
enum mtype {
	eMain=1,
	eScreen,
	eSound,
	eSW,
	eTouchP
};


//===========================================
// メッセージflag（処理の場合分け用）
//（入出力ごとに、1000ずつ区別）
//===========================================
enum msgflg {
	eSwOn = 1001,

	eToAlarm = 2001,
	eToNormal,
	eToNoAlarm,
	eAlarmOn,
	eAlarmOff,
	eAlarmSet_h,
	eAlarmSet_m,
	eAlarmSet_s,

	eUpdateTime = 3001,
	eDispTimeNow = 4001,

	eDispDebug
};


//===========================================
// メッセージバッファ（送る中身）
//===========================================
// 時刻用の構造体
typedef struct {
	int	hh;
	int	mm;
	int ss;
} clocks_t;


typedef struct {
	long mtype;	//ここに宛先（enum)を格納
	int flag; 	//処理の場合分け用
	
	//
	union {
		char mtext[ 256 ];	//メッセージに情報を持たせるときに使用する
		clocks_t time; 		//時刻の参照方法　buf.UN_RAW.time.hh
	}UN_RAW;
} buf_t;




//=======================================
// グローバル変数
//=======================================
