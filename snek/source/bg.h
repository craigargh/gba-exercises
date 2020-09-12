
//{{BLOCK(bg)

//======================================================================
//
//	bg, 8x8@8, 
//	+ palette 256 entries, not compressed
//	+ 1 tiles not compressed
//	Total size: 512 + 64 = 576
//
//	Time-stamp: 2020-09-12, 16:41:32
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.16
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_BG_H
#define GRIT_BG_H

#define bgTilesLen 64
extern const unsigned int bgTiles[16];

#define bgPalLen 512
extern const unsigned short bgPal[256];

#define bgBlocksLen 2048
extern const unsigned short bgBlocks[1024];

#endif // GRIT_BG_H

//}}BLOCK(bg)
