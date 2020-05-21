#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF 256

#define N 256 // 1行の最大文字数(バイト数)
 
int
main (int argc, char *argv[])
{


//============ GET ALARM TIME START ===================
	FILE *fp1; // FILE型構造体
	char fname[] = "test.txt"; //アラーム時刻を記載する。
	char str[N];

	fp1 = fopen(fname, "r"); // ファイルを開く。失敗するとNULLを返す。
	if(fp1 == NULL) {
		printf("%s file not open!\n", fname);
		return -1;
	}

	while(fgets(str, N, fp1) != NULL) {
//		printf("ALARM TIME	: %s", str);
	}


//======== GET CURRENT TIME START ========================
	FILE	*fp;
//	char	*cmdline = "date +\"%H%M%S\"";
	char	*cmdline = "date +\"%H%M\"";
	if ( (fp=popen(cmdline,"r")) ==NULL) {
		perror ("can not exec commad");
		exit(EXIT_FAILURE);
	}
	char buf[BUF];
 
//	while (!feof(fp)) {
		fgets(buf, sizeof(buf), fp);
//		printf("CURRENT TIME	: %s", buf);
//	}

//================================================== 
//======== COMPARISON START ========================

printf("ALARM TIME	: %s", str);
printf("CURRENT TIME	: %s", buf);

if(strcmp(str,buf)==0){
			char cmd[255];
			sprintf(cmd,"aplay %s","alarm02.wav");
			system(cmd);
			printf("HIT\n");
}

else printf("OUT\n");

//======== COMPARISON END ==========================
//==================================================

	(void) pclose(fp);
 
	exit (EXIT_SUCCESS);

//======== GET CURRENT TIME END ========================


//============  CLOSE ALARM TIME  ===================

	fclose(fp1); // ファイルを閉じる

//============  GET ALARM TIME END ===================

}
