//====================================================================
//@title      dpmatching_prototype001.c
//description city011とcity012のすべてのファイルのDPマッチングを行う
//--------------------------------------------------------------------
//@version    0.01 (2019/06/06) プログラム作成
//@version    0.10 (2019/06/19) 単体のテキストファイルの読み込み(4行目以降が辛かった...)
//@version    0.11 (2019/06/20) city011内全てのファイルの読み込み(memory不足)
//@version    0.20 (2019/06/21) city011、city012のファイルを一つずつ
//                              読み込み、ファイル単語間距離を計算
//@version    0.20 (2019/06/21) city011_001.txtと、city012_001~100.txtの
//                              単語間距離を計算して、最小の組み合わせを表示
//@version    0.30 (2019/06/21) city011_001~100.txtと、city012_001~100.txtの
//                              単語間距離を計算して、それぞれの最小の組み合わせを表示
//@version    0.40 (2019/06/22) 認識率をtxt形式で出力
//@version    0.41 (2019/06/23) 割合と百分率の混合を百分率に統一
//@version    0.42 (2019/06/23) dpマッチングa~cが最小のものを選ぶ部分をデバック
//@version    0.43 (2019/06/23) powlを通常の乗算にして実行速度を向上
//@version    1.00 (2019/06/24) バグの除去、同一話者で正答率99%を出す
//@version    1.01 (2019/06/24) コンソールで誤認識したデータを確認するように変更
//@version    1.02 (2019/06/24) 読み込むファイル名をマクロで定義
//@author     KenTOKURA
//====================================================================

#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#define MAXSIZE   254
#define DIMENTION 15
#define NUMOFFILE 100
#define FNAME_OUTPUT "./output/output001.txt"
#define TEMP_NUM 11 //11,12,21,22から選択, %03dで指定すること。
#define MITI_NUM 21 //11,12,21,22から選択, %03dで指定すること。

//ファイルデータをmcepdata_t型へ格納
typedef struct {
    char name[20];
    char onso[50];
    int  flame;
    double mcepdata[MAXSIZE][DIMENTION];
} mcepdata_t;

int main(void)
{
	int  h0, h, i, j, k;
	FILE *fp_temp, *fp_miti, *fp_output;
	mcepdata_t city_temp, city_miti; //テンプレートのデータと未知入力データ
	char ch0[200]; 
	double d[MAXSIZE][MAXSIZE];      //局所距離
	double g[MAXSIZE][MAXSIZE];      //累積距離
	double tangokankyori[NUMOFFILE]; //単語間距離
	double tangokankyori_min; 
	int    num_matchfname = 0;
	int    count = 0;

    //認識するデータを確認
    printf("city%03dとcity%03dの認識実験を開始します。\n", TEMP_NUM, MITI_NUM);

	/* city_temp に100個のデータを入れて計算 */

	for (h0 = 0; h0 < NUMOFFILE; h0++) {

		/* city_tempを読み込む */

		sprintf(ch0, "./city_mcepdata/city%03d/city%03d_%03d.txt",TEMP_NUM, TEMP_NUM, h0 + 1);
		//printf("%s\n", ch0); //デバック用
		if ((fp_temp = fopen(ch0, "r")) == NULL) {
			printf("temp file open error!!\n");
			exit(EXIT_FAILURE);
		}
		//ヘッダーの読み込み
		fgets(city_temp.name, sizeof(city_temp.name), fp_temp);
		fgets(city_temp.onso, sizeof(city_temp.onso), fp_temp);
		fgets(ch0,sizeof(ch0), fp_temp);
		city_temp.flame = atoi(ch0);
		//mcepdataの取り込み
		for (i = 0; i < city_temp.flame; i++) {
            for (j = 0; j < DIMENTION; j++) {
				fscanf(fp_temp, "%lf", &city_temp.mcepdata[i][j]);
			}
		}

        /* city_mini に100個のデータを入れて計算 */

		for (h = 0; h < NUMOFFILE; h++) {

			/* city_mitiを読み込む */

            //ファイルをオープン
			sprintf(ch0, "./city_mcepdata/city%03d/city%03d_%03d.txt",MITI_NUM, MITI_NUM, h + 1);
			//printf("%s\n", ch0); //デバック用
			if ((fp_miti = fopen(ch0, "r")) == NULL) {
				printf("miti file open error!!\n");
				exit(EXIT_FAILURE);
			}
			//ヘッダーの読み込み
			fgets(city_miti.name, sizeof(city_miti.name), fp_miti);
			fgets(city_miti.onso, sizeof(city_miti.onso), fp_miti);
			fgets(ch0,sizeof(ch0), fp_miti);
			city_miti.flame = atoi(ch0);
			//mcepdataの取り込み
			for (i = 0; i < city_miti.flame; i++) {
				for (j = 0; j < DIMENTION; j++) {
					fscanf(fp_miti, "%lf", &city_miti.mcepdata[i][j]);
				}
			}

			/* DPマッチングで単語間距離を求める */

			//フレームごとの局所距離dを計算
			for (i = 0; i < city_temp.flame; i++) {
				for (j = 0; j < city_miti.flame; j++) {
                    d[i][j] = 0;
					for (int k = 0; k < DIMENTION; k++) {
						//printf("%f\n", kyokusyokyori);
						d[i][j] += (city_temp.mcepdata[i][k] - city_miti.mcepdata[j][k]) * (city_temp.mcepdata[i][k] - city_miti.mcepdata[j][k]); 
					}
                    sqrtl(d[i][j]);
					//printf("d[%d][%d] = %f\n", i, j, d[i][j]); //デバック用
				}
			}
			//初期条件g(0,0)を求める
			g[0][0] = d[0][0];
			//境界条件g(i,0),g(0,j)を求める
			for (i = 1; i < city_temp.flame; i++) {
				g[i][0] = g[i - 1][0] + d[i][0];
			}
			for (j = 1; j < city_miti.flame; j++) {
				g[0][j] = g[0][j - 1] + d[0][j];
			}
			//累積距離g(i,j)を求める
			for (i = 1; i < city_temp.flame; i++) {
				for (j = 1; j < city_miti.flame; j++) {
					double a = g[i][j - 1] + d[i][j];
					double b = g[i - 1][j - 1] + 2 * d[i][j];
					double c = g[i - 1][j] + d[i][j];
					g[i][j] = a;
					if (b < g[i][j]) {
						g[i][j] = b;
					}
					if (c < g[i][j]) {
						g[i][j] = c;
					}
				}
			}

			//単語間距離を求める
			tangokankyori[h] = g[city_temp.flame - 1][city_miti.flame - 1] / (city_temp.flame + city_miti.flame);
			//printf("tangokankyori[%d] = %f\n", h, tangokankyori[h]); //デバック用
			
			//ファイルのクローズ
			fclose(fp_miti);
		}

		//単語間距離が一番小さいcity_mitiを見つける
        tangokankyori_min = tangokankyori[0];
        num_matchfname = 0;
		for (h = 1; h < NUMOFFILE; h++) {
			if (tangokankyori_min > tangokankyori[h] ) {
				tangokankyori_min = tangokankyori[h];
				num_matchfname = h;
			}
		}

		//ファイルのクローズ
		fclose(fp_temp);

		// 正答率の算出
		if (num_matchfname == h0 ) {
			count++;
		}

		/* 出力 */
        
        //マッチしなかったtempの詳細を表示
		if (num_matchfname != h0 ) {
            printf("===========Result NOT Matchng===========\n");
            printf("city_temp     : city%03d/city%03d_%03d.txt\n", TEMP_NUM, TEMP_NUM, h0 + 1);
            printf("city_miti     : city%03d/city%03d_%03d.txt\n", MITI_NUM, MITI_NUM, num_matchfname + 1);
            printf("tangokankyori : %f\n", tangokankyori_min);
        }

	}

	/* 出力 */

	// 出力ファイルを作成
	sprintf(ch0, FNAME_OUTPUT);
	//printf("%s\n", ch0); //デバック用
	if ((fp_output = fopen(ch0, "a")) == NULL) {
		printf("output file open error!!\n");
		exit(EXIT_FAILURE);
	}
	//ファイルに出力
	fprintf(fp_output, "正答率%d%%です。\n", count);
    printf("\nファイルを作成しました。\n");
	printf("正答率 %d%% です。\n", count); 

	// ファイルのクローズ
	fclose(fp_output);

	return 0;
}
//@memo
//debug用関数
// 文字の色を変更
/*
    printf("\x1b[34m"); //青
    printf("\x1b[39m"); //元に戻す
*/

//cityファイルについて
/*
    "city_mcepdata/city011/city011_001.txt"のように管理されている。
    "city011,012,021,022"がある。それぞれ同様の100単語が収録されている。
    1行目にはファイル名、2行目には音素名、3行目はフレーム数である。
    4行目以降は15次からなるメルケプストラム特徴量。
*/
