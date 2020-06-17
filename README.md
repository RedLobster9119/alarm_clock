# alarm_clock

目覚まし時計アプリ開発

ポリテクセンター関東での総合課題（卒業制作）として、グループ開発で取り組みました。
コロナ禍のため、常に開発チームメンバーが欠けた状態（半数程度）ではありましたが、基本的な機能は完成させることが出来ました。
緊急事態宣言による突然の登校禁止となり、ファイル不足＋バージョンが最新ではありませんが、チームメンバーから集められたものをアップしました。


＜要件定義＞
- 時刻取得
- アラーム設定
- ユーザーに知らせる（サウンド、スクリーン）
- アラームを止める（ミニゲームでの勝利）
- 電源を入れ直しても、時刻正常、アラーム設定変化なし

＜基本設計＞

各プロセス間をメッセージを用いて管理する。
アラーム設定時刻の情報については、共有メモリで管理する。

【入力 （全てメインファイルにメッセージを送る）】
- タッチパネル
- 時刻（1秒毎にメッセージを送る）
-スイッチ
【出力 （全てメインファイルからメッセージを受信する）】
-スクリーン
-サウンド

【メイン】
-子プロセスの生成
-入力からのメッセージ受信
-出力へのメッセージ送信
-アラーム時刻の管理、メッセージ送信
