#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftmodapi.h>
#include "font.h"

#define SCREENWIDTH	800
#define SCREENHEIGHT	480

/* 点描写関数 */
static void pset(unsigned long *pfb,int x,int y,unsigned long color)
{
	if(y*SCREENWIDTH+x < SCREENWIDTH*SCREENHEIGHT)
		*(pfb+y*SCREENWIDTH+x) = color;
	return;
}

/*文字表示*/
int put_char(unsigned long *pfb, char fontfile[],
	unsigned long moji,unsigned long size,int x,int y,
	unsigned long color, unsigned long bkcolor){
	FT_Library library;
	FT_Face    face;

	FT_GlyphSlot slot;
	FT_UInt glyph_index;
	FT_Bitmap bitmap;
	int xx,yy,i;
	unsigned char r, g, b;
	int error;
	
	// FreeTypeの初期化とTrueTypeフォントの読み込み
	FT_Init_FreeType( &library );
	error=FT_New_Face( library, fontfile, 0, &face);
	if(error !=0){
		printf("ERROR : FT_New_Face (%d) \n",error);
		//FT_Done_Library(library);
		return(error);
	}
	slot = face->glyph;

	FT_Set_Pixel_Sizes ( face, 0, size);

	// 文字をビットマップ化
	FT_Load_Char( face, moji, FT_LOAD_RENDER);
	bitmap = slot->bitmap;
	//printf("rows=%d width=%d \n",bitmap.rows , bitmap.width);
	//ベースラインの調整
	y=y+size-slot->bitmap_top;
	i=0;
	for(yy = 0; yy < bitmap.rows; yy++){
		for(xx = 0; xx < bitmap.width; xx++){
			if(bitmap.buffer[i])
				pset(pfb, xx + x, yy + y, color);
			else if(color !=bkcolor)
				pset(pfb, xx + x, yy + y, bkcolor);
			i++;
		}
	}
	FT_Done_Face(face);
	FT_Done_Library(library);
	return bitmap.width;
}

