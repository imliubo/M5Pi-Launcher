/*                                                                   */
/*  lvgl_nes file : porting to lvgl                                  */
/*                                                                   */
/*  2001/05/18  InfoNES Project ( Sound is based on DarcNES )        */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

#include "InfoNES.h"
#include "InfoNES_System.h"
#include "InfoNES_pAPU.h"

// bool define
#define TRUE 1
#define FALSE 0

/* lcd 操作相关 头文件 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <termios.h>
#include "lvgl/lvgl.h"
#include "../launcher.h"

#include <fcntl.h>

static snd_pcm_t *playback_handle;

static int fb_fd = 1;
static int line_width;

static int *zoom_x_tab;
static int *zoom_y_tab;

extern int InitJoypadInput(void);
extern int GetJoypadInput(void);

/*-------------------------------------------------------------------*/
/*  ROM image file information                                       */
/*-------------------------------------------------------------------*/

char szRomName[256];
char szSaveName[256];
int nSRAM_SaveFlag;

/*-------------------------------------------------------------------*/
/*  Constants ( Linux specific )                                     */
/*-------------------------------------------------------------------*/

#define VBOX_SIZE 7
#define SOUND_DEVICE "/dev/dsp"
#define VERSION "InfoNES v0.91J"

/*-------------------------------------------------------------------*/
/*  Global Variables ( Linux specific )                              */
/*-------------------------------------------------------------------*/

/* Emulation thread */
pthread_t nes_tid;
int bThread;

/* Pad state */
DWORD dwKeyPad1;
DWORD dwKeyPad2;
DWORD dwKeySystem;

/* For Sound Emulation */
BYTE final_wave[2048];
int waveptr;
int wavflag;
int sound_fd;

/*-------------------------------------------------------------------*/
/*  Function prototypes ( Linux specific )                           */
/*-------------------------------------------------------------------*/

int make_zoom_tab(void);

void *emulation_thread(void *args);

void start_application(char *filename);

int LoadSRAM();

int SaveSRAM();

/* Palette data */
#if (LV_COLOR_DEPTH == 16)
WORD NesPalette[] = {
    0xff000000 | ( 0x80 << 16 ) | (0x80 << 8) | ( 0x80 << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x00 << 8) | ( 0xBB << 0),
    0xff000000 | ( 0x37 << 16 ) | (0x00 << 8) | ( 0xBF << 0),
    0xff000000 | ( 0x84 << 16 ) | (0x00 << 8) | ( 0xA6 << 0),
    0xff000000 | ( 0xBB << 16 ) | (0x00 << 8) | ( 0x6A << 0),
    0xff000000 | ( 0xB7 << 16 ) | (0x00 << 8) | ( 0x1E << 0),
    0xff000000 | ( 0xB3 << 16 ) | (0x00 << 8) | ( 0x00 << 0),
    0xff000000 | ( 0x91 << 16 ) | (0x26 << 8) | ( 0x00 << 0),
    0xff000000 | ( 0x7B << 16 ) | (0x2B << 8) | ( 0x00 << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x3E << 8) | ( 0x00 << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x48 << 8) | ( 0x0D << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x3C << 8) | ( 0x22 << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x2F << 8) | ( 0x66 << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x00 << 8) | ( 0x00 << 0),
    0xff000000 | ( 0x05 << 16 ) | (0x05 << 8) | ( 0x05 << 0),
    0xff000000 | ( 0x05 << 16 ) | (0x05 << 8) | ( 0x05 << 0),
    0xff000000 | ( 0xC8 << 16 ) | (0xC8 << 8) | ( 0xC8 << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x59 << 8) | ( 0xFF << 0),
    0xff000000 | ( 0x44 << 16 ) | (0x3C << 8) | ( 0xFF << 0),
    0xff000000 | ( 0xB7 << 16 ) | (0x33 << 8) | ( 0xCC << 0),
    0xff000000 | ( 0xFF << 16 ) | (0x33 << 8) | ( 0xAA << 0),
    0xff000000 | ( 0xFF << 16 ) | (0x37 << 8) | ( 0x5E << 0),
    0xff000000 | ( 0xFF << 16 ) | (0x37 << 8) | ( 0x1A << 0),
    0xff000000 | ( 0xD5 << 16 ) | (0x4B << 8) | ( 0x00 << 0),
    0xff000000 | ( 0xC4 << 16 ) | (0x62 << 8) | ( 0x00 << 0),
    0xff000000 | ( 0x3C << 16 ) | (0x7B << 8) | ( 0x00 << 0),
    0xff000000 | ( 0x1E << 16 ) | (0x84 << 8) | ( 0x15 << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x95 << 8) | ( 0x66 << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x84 << 8) | ( 0xC4 << 0),
    0xff000000 | ( 0x11 << 16 ) | (0x11 << 8) | ( 0x11 << 0),
    0xff000000 | ( 0x09 << 16 ) | (0x09 << 8) | ( 0x09 << 0),
    0xff000000 | ( 0x09 << 16 ) | (0x09 << 8) | ( 0x09 << 0),
    0xff000000 | ( 0xFF << 16 ) | (0xFF << 8) | ( 0xFF << 0),
    0xff000000 | ( 0x00 << 16 ) | (0x95 << 8) | ( 0xFF << 0),
    0xff000000 | ( 0x6F << 16 ) | (0x84 << 8) | ( 0xFF << 0),
    0xff000000 | ( 0xD5 << 16 ) | (0x6F << 8) | ( 0xFF << 0),
    0xff000000 | ( 0xFF << 16 ) | (0x77 << 8) | ( 0xCC << 0),
    0xff000000 | ( 0xFF << 16 ) | (0x6F << 8) | ( 0x99 << 0),
    0xff000000 | ( 0xFF << 16 ) | (0x7B << 8) | ( 0x59 << 0),
    0xff000000 | ( 0xFF << 16 ) | (0x91 << 8) | ( 0x5F << 0),
    0xff000000 | ( 0xFF << 16 ) | (0xA2 << 8) | ( 0x33 << 0),
    0xff000000 | ( 0xA6 << 16 ) | (0xBF << 8) | ( 0x00 << 0),
    0xff000000 | ( 0x51 << 16 ) | (0xD9 << 8) | ( 0x6A << 0),
    0xff000000 | ( 0x4D << 16 ) | (0xD5 << 8) | ( 0xAE << 0),
    0xff000000 | ( 0x00 << 16 ) | (0xD9 << 8) | ( 0xFF << 0),
    0xff000000 | ( 0x66 << 16 ) | (0x66 << 8) | ( 0x66 << 0),
    0xff000000 | ( 0x0D << 16 ) | (0x0D << 8) | ( 0x0D << 0),
    0xff000000 | ( 0x0D << 16 ) | (0x0D << 8) | ( 0x0D << 0),
    0xff000000 | ( 0xFF << 16 ) | (0xFF << 8) | ( 0xFF << 0),
    0xff000000 | ( 0x84 << 16 ) | (0xBF << 8) | ( 0xFF << 0),
    0xff000000 | ( 0xBB << 16 ) | (0xBB << 8) | ( 0xFF << 0),
    0xff000000 | ( 0xD0 << 16 ) | (0xBB << 8) | ( 0xFF << 0),
    0xff000000 | ( 0xFF << 16 ) | (0xBF << 8) | ( 0xEA << 0),
    0xff000000 | ( 0xFF << 16 ) | (0xBF << 8) | ( 0xCC << 0),
    0xff000000 | ( 0xFF << 16 ) | (0xC4 << 8) | ( 0xB7 << 0),
    0xff000000 | ( 0xFF << 16 ) | (0xCC << 8) | ( 0xAE << 0),
    0xff000000 | ( 0xFF << 16 ) | (0xD9 << 8) | ( 0xA2 << 0),
    0xff000000 | ( 0xCC << 16 ) | (0xE1 << 8) | ( 0x99 << 0),
    0xff000000 | ( 0xAE << 16 ) | (0xEE << 8) | ( 0xB7 << 0),
    0xff000000 | ( 0xAA << 16 ) | (0xF7 << 8) | ( 0xEE << 0),
    0xff000000 | ( 0xB3 << 16 ) | (0xEE << 8) | ( 0xFF << 0),
    0xff000000 | ( 0xDD << 16 ) | (0xDD << 8) | ( 0xDD << 0),
    0xff000000 | ( 0x11 << 16 ) | (0x11 << 8) | ( 0x11 << 0),
    0xff000000 | ( 0x11 << 16 ) | (0x11 << 8) | ( 0x11 << 0)
};
#else
WORD NesPalette[] = {
    0x738E, 0x88C4, 0xA800, 0x9808, 0x7011, 0x1015, 0x0014, 0x004F, 0x0148, 0x0200, 0x0280, 0x11C0, 0x59C3,
    0x0000, 0x0000, 0x0000, 0xBDD7, 0xEB80, 0xE9C4, 0xF010, 0xB817, 0x581C, 0x015B, 0x0A59, 0x0391, 0x0480,
    0x0540, 0x3C80, 0x8C00, 0x0000, 0x0000, 0x0000, 0xFFDF, 0xFDC7, 0xFC8B, 0xFC48, 0xFBDE, 0xB39F, 0x639F,
    0x3CDF, 0x3DDE, 0x1690, 0x4EC9, 0x9FCB, 0xDF40, 0x0000, 0x0000, 0x0000, 0xFFDF, 0xFF15, 0xFE98, 0xFE5A,
    0xFE1F, 0xDE1F, 0xB5DF, 0xAEDF, 0xA71F, 0xA7DC, 0xBF95, 0xCFD6, 0xF7D3, 0x0000, 0x0000, 0x0000,
};
#endif

/*===================================================================*/
/*                                                                   */
/*                main() : Application main                          */
/*                                                                   */
/*===================================================================*/
/* Application main */
int nes_init(char *nes_file)
{
    char cmd;

    /*-------------------------------------------------------------------*/
    /*  Pad Control                                                      */
    /*-------------------------------------------------------------------*/

    /* Initialize a pad state */
    dwKeyPad1   = 0;
    dwKeyPad2   = 0;
    dwKeySystem = 0;

    /*-------------------------------------------------------------------*/
    /*  Load Cassette & Create Thread                                    */
    /*-------------------------------------------------------------------*/

    /* Initialize thread state */
    bThread = FALSE;

    int i;

    // InitJoypadInput();

    //初始化 zoom 缩放表
    make_zoom_tab();

    assert(nes_file);
    start_application(nes_file);
    return (0);
}

/**
 * 生成zoom 缩放表
 */
int make_zoom_tab(void)
{
    int i;
    zoom_x_tab = (int *)malloc(sizeof(int) * M5PI_SCREEN_WIDTH);

    if (NULL == zoom_x_tab) {
        printf("make zoom_x_tab error\n");
        return -1;
    }
    for (i = 0; i < M5PI_SCREEN_WIDTH; i++) {
        zoom_x_tab[i] = i * NES_DISP_WIDTH / M5PI_SCREEN_WIDTH;
    }
    zoom_y_tab = (int *)malloc(sizeof(int) * M5PI_SCREEN_HEIGHT );
    if (NULL == zoom_y_tab) {
        printf("make zoom_y_tab error\n");
        return -1;
    }
    for (i = 0; i < M5PI_SCREEN_HEIGHT ; i++) {
        zoom_y_tab[i] = i * NES_DISP_HEIGHT / M5PI_SCREEN_HEIGHT ;
    }
    return 1;
}

/*===================================================================*/
/*                                                                   */
/*           emulation_thread() : Thread Hooking Routine             */
/*                                                                   */
/*===================================================================*/

void *emulation_thread(void *args)
{
    InfoNES_Main();
}

/*===================================================================*/
/*                                                                   */
/*     start_application() : Start NES Hardware                      */
/*                                                                   */
/*===================================================================*/
void start_application(char *filename)
{
    /* Set a ROM image name */
    strcpy(szRomName, filename);

    /* Load cassette */
    if (InfoNES_Load(szRomName) == 0) {
        /* Load SRAM */
        LoadSRAM();

        /* Create Emulation Thread */
        bThread = TRUE;
        pthread_create(&nes_tid, NULL, emulation_thread, NULL);
    }else{
        assert(NULL);
    }
}

/*===================================================================*/
/*                                                                   */
/*           LoadSRAM() : Load a SRAM                                */
/*                                                                   */
/*===================================================================*/
int LoadSRAM()
{
    /*
     *  Load a SRAM
     *
     *  Return values
     *     0 : Normally
     *    -1 : SRAM data couldn't be read
     */

    FILE *fp;
    unsigned char pSrcBuf[SRAM_SIZE];
    unsigned char chData;
    unsigned char chTag;
    int nRunLen;
    int nDecoded;
    int nDecLen;
    int nIdx;

    /* It doesn't need to save it */
    nSRAM_SaveFlag = 0;

    /* It is finished if the ROM doesn't have SRAM */
    if (!ROM_SRAM) {
        return (0);
    }

    /* There is necessity to save it */
    nSRAM_SaveFlag = 1;

    /* The preparation of the SRAM file name */
    strcpy(szSaveName, szRomName);
    strcpy(strrchr(szSaveName, '.') + 1, "srm");

    /*-------------------------------------------------------------------*/
    /*  Read a SRAM data                                                 */
    /*-------------------------------------------------------------------*/

    /* Open SRAM file */
    fp = fopen(szSaveName, "rb");
    if (fp == NULL) {
        return (-1);
    }

    /* Read SRAM data */
    fread(pSrcBuf, SRAM_SIZE, 1, fp);

    /* Close SRAM file */
    fclose(fp);

    /*-------------------------------------------------------------------*/
    /*  Extract a SRAM data                                              */
    /*-------------------------------------------------------------------*/

    nDecoded = 0;
    nDecLen  = 0;

    chTag = pSrcBuf[nDecoded++];

    while (nDecLen < 8192) {
        chData = pSrcBuf[nDecoded++];

        if (chData == chTag) {
            chData  = pSrcBuf[nDecoded++];
            nRunLen = pSrcBuf[nDecoded++];
            for (nIdx = 0; nIdx < nRunLen + 1; ++nIdx) {
                SRAM[nDecLen++] = chData;
            }
        } else {
            SRAM[nDecLen++] = chData;
        }
    }

    /* Successful */
    return (0);
}

/*===================================================================*/
/*                                                                   */
/*           SaveSRAM() : Save a SRAM                                */
/*                                                                   */
/*===================================================================*/
int SaveSRAM()
{
    /*
     *  Save a SRAM
     *
     *  Return values
     *     0 : Normally
     *    -1 : SRAM data couldn't be written
     */

    FILE *fp;
    int nUsedTable[256];
    unsigned char chData;
    unsigned char chPrevData;
    unsigned char chTag;
    int nIdx;
    int nEncoded;
    int nEncLen;
    int nRunLen;
    unsigned char pDstBuf[SRAM_SIZE];

    if (!nSRAM_SaveFlag) {
        return (0);    /* It doesn't need to save it */
    }

    /*-------------------------------------------------------------------*/
    /*  Compress a SRAM data                                             */
    /*-------------------------------------------------------------------*/

    memset(nUsedTable, 0, sizeof nUsedTable);

    for (nIdx = 0; nIdx < SRAM_SIZE; ++nIdx) {
        ++nUsedTable[SRAM[nIdx++]];
    }
    for (nIdx = 1, chTag = 0; nIdx < 256; ++nIdx) {
        if (nUsedTable[nIdx] < nUsedTable[chTag]) {
            chTag = nIdx;
        }
    }

    nEncoded = 0;
    nEncLen  = 0;
    nRunLen  = 1;

    pDstBuf[nEncLen++] = chTag;

    chPrevData = SRAM[nEncoded++];

    while (nEncoded < SRAM_SIZE && nEncLen < SRAM_SIZE - 133) {
        chData = SRAM[nEncoded++];

        if (chPrevData == chData && nRunLen < 256) {
            ++nRunLen;
        } else {
            if (nRunLen >= 4 || chPrevData == chTag) {
                pDstBuf[nEncLen++] = chTag;
                pDstBuf[nEncLen++] = chPrevData;
                pDstBuf[nEncLen++] = nRunLen - 1;
            } else {
                for (nIdx = 0; nIdx < nRunLen; ++nIdx) {
                    pDstBuf[nEncLen++] = chPrevData;
                }
            }

            chPrevData = chData;
            nRunLen    = 1;
        }
    }
    if (nRunLen >= 4 || chPrevData == chTag) {
        pDstBuf[nEncLen++] = chTag;
        pDstBuf[nEncLen++] = chPrevData;
        pDstBuf[nEncLen++] = nRunLen - 1;
    } else {
        for (nIdx = 0; nIdx < nRunLen; ++nIdx) {
            pDstBuf[nEncLen++] = chPrevData;
        }
    }

    /*-------------------------------------------------------------------*/
    /*  Write a SRAM data                                                */
    /*-------------------------------------------------------------------*/

    /* Open SRAM file */
    fp = fopen(szSaveName, "wb");
    if (fp == NULL) {
        return (-1);
    }

    /* Write SRAM data */
    fwrite(pDstBuf, nEncLen, 1, fp);

    /* Close SRAM file */
    fclose(fp);

    /* Successful */
    return (0);
}

/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Menu() : Menu screen                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_Menu()
{
    /*
     *  Menu screen
     *
     *  Return values
     *     0 : Normally
     *    -1 : Exit InfoNES
     */

    /* If terminated */
    if (bThread == FALSE) {
        return (-1);
    }

    /* Nothing to do here */
    return (0);
}

/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
int InfoNES_ReadRom(const char *pszFileName)
{
    /*
     *  Read ROM image file
     *
     *  Parameters
     *    const char *pszFileName          (Read)
     *
     *  Return values
     *     0 : Normally
     *    -1 : Error
     */

    FILE *fp;

    /* Open ROM file */
    fp = fopen(pszFileName, "rb");
    if (fp == NULL) {
        return (-1);
    }

    /* Read ROM Header */
    fread(&NesHeader, sizeof NesHeader, 1, fp);
    if (memcmp(NesHeader.byID, "NES\x1a", 4) != 0) {
        /* not .nes file */
        fclose(fp);
        return (-1);
    }

    /* Clear SRAM */
    memset(SRAM, 0, SRAM_SIZE);

    /* If trainer presents Read Triner at 0x7000-0x71ff */
    if (NesHeader.byInfo1 & 4) {
        fread(&SRAM[0x1000], 512, 1, fp);
    }

    /* Allocate Memory for ROM Image */
    ROM = (BYTE *)malloc(NesHeader.byRomSize * 0x4000);

    /* Read ROM Image */
    fread(ROM, 0x4000, NesHeader.byRomSize, fp);

    if (NesHeader.byVRomSize > 0) {
        /* Allocate Memory for VROM Image */
        VROM = (BYTE *)malloc(NesHeader.byVRomSize * 0x2000);

        /* Read VROM Image */
        fread(VROM, 0x2000, NesHeader.byVRomSize, fp);
    }

    /* File close */
    fclose(fp);

    /* Successful */
    return (0);
}

/*===================================================================*/
/*                                                                   */
/*           InfoNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*===================================================================*/
void InfoNES_ReleaseRom()
{
    /*
     *  Release a memory for ROM
     *
     */

    if (ROM) {
        free(ROM);
        ROM = NULL;
    }

    if (VROM) {
        free(VROM);
        VROM = NULL;
    }
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemoryCopy(void *dest, const void *src, int count)
{
    /*
     *  memcpy
     *
     *  Parameters
     *    void *dest                       (Write)
     *      Points to the starting address of the copied block's destination
     *
     *    const void *src                  (Read)
     *      Points to the starting address of the block of memory to copy
     *
     *    int count                        (Read)
     *      Specifies the size, in bytes, of the block of memory to copy
     *
     *  Return values
     *    Pointer of destination
     */

    memcpy(dest, src, count);
    return (dest);
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : memset                          */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemorySet(void *dest, int c, int count)
{
    /*
     *  memset
     *
     *  Parameters
     *    void *dest                       (Write)
     *      Points to the starting address of the block of memory to fill
     *
     *    int c                            (Read)
     *      Specifies the byte value with which to fill the memory block
     *
     *    int count                        (Read)
     *      Specifies the size, in bytes, of the block of memory to fill
     *
     *  Return values
     *    Pointer of destination
     */

    memset(dest, c, count);
    return (dest);
}

/*===================================================================*/
/*                                                                   */
/*      InfoNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*===================================================================*/
void InfoNES_LoadFrame()
{
    int x, y;
    int line_width;
    WORD wColor;

    static lv_color_t cbuf_temp[LV_CANVAS_BUF_SIZE_TRUE_COLOR(M5PI_SCREEN_WIDTH, M5PI_SCREEN_HEIGHT)];
    static lv_color_t pixel;
    //修正 即便没有 LCD 也可以出声
    int count = 0;
    if (0 < fb_fd) {
        for (y = 0; y < 240; y++) {
            line_width = zoom_y_tab[y] * NES_DISP_WIDTH;
            for (x = 0; x < 320; x++) {
                wColor = WorkFrame[line_width + zoom_x_tab[x]];
#if (LV_COLOR_DEPTH == 32)
                // pixel.ch.red = ((wColor >> 16) & 0xff);
                // pixel.ch.green = ((wColor >> 8) & 0xff);
                // pixel.ch.blue = ((wColor >> 0) & 0xff);
                pixel.ch.alpha = ((wColor >> 24) & 0xff);
                pixel.ch.red = ((wColor >> 16) & 0xff);
                pixel.ch.green = ((wColor >> 8) & 0xff);
                pixel.ch.blue = ((wColor >> 0) & 0xff);
#else
                pixel.ch.blue = ((wColor >> 11) & 0x1f);
                pixel.ch.green = ((wColor >> 5) & 0x3f);
                pixel.ch.red = ((wColor >> 0) & 0x1f);
#endif
                cbuf_temp[count] = pixel;
                count += 1;
            }
        }
        lv_canvas_set_buffer(nes_canvas, cbuf_temp, M5PI_SCREEN_WIDTH, M5PI_SCREEN_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    }
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState(DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem)
{
    /*
     *  Get a joypad state
     *
     *  Parameters
     *    DWORD *pdwPad1                   (Write)
     *      Joypad 1 State
     *
     *    DWORD *pdwPad2                   (Write)
     *      Joypad 2 State
     *
     *    DWORD *pdwSystem                 (Write)
     *      Input for InfoNES
     *
     */

    /* Transfer joypad state */
    *pdwPad1   = dwKeyPad1;
    *pdwPad2   = dwKeyPad2;
    *pdwSystem = dwKeySystem;

    //取消重置手柄 在 输入函数中自行处理
    // dwKeyPad1 = 0;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit(void)
{}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen(int samples_per_sync, int sample_rate)
{
    // //sample_rate 采样率 44100
    // //samples_per_sync  735
    // unsigned int rate      = sample_rate;
    // snd_pcm_hw_params_t *hw_params;

    // if(0 > snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0))
    // {
    //  printf("snd_pcm_open err\n");
    //  return -1;
    // }

    // if(0 > snd_pcm_hw_params_malloc(&hw_params))
    // {
    //  printf("snd_pcm_hw_params_malloc err\n");
    //  return -1;
    // }

    // if(0 > snd_pcm_hw_params_any(playback_handle, hw_params))
    // {
    //  printf("snd_pcm_hw_params_any err\n");
    //  return -1;
    // }
    // if(0 > snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED))
    // {
    //  printf("snd_pcm_hw_params_any err\n");
    //  return -1;
    // }

    // //8bit PCM 数据
    // if(0 > snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_U8))
    // {
    //  printf("snd_pcm_hw_params_set_format err\n");
    //  return -1;
    // }

    // if(0 > snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0))
    // {
    //  printf("snd_pcm_hw_params_set_rate_near err\n");
    //  return -1;
    // }

    // //单声道 非立体声
    // if(0 > snd_pcm_hw_params_set_channels(playback_handle, hw_params, 1))
    // {
    //  printf("snd_pcm_hw_params_set_channels err\n");
    //  return -1;
    // }

    // if(0 > snd_pcm_hw_params(playback_handle, hw_params))
    // {
    //  printf("snd_pcm_hw_params err\n");
    //  return -1;
    // }

    // snd_pcm_hw_params_free(hw_params);

    // if(0 > snd_pcm_prepare(playback_handle))
    // {
    //  printf("snd_pcm_prepare err\n");
    //  return -1;
    // }
    return 1;
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose(void)
{
    // snd_pcm_close(playback_handle);
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output 5 Waves           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput(int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5)
{
    // int i;
    // int ret;
    // unsigned char wav;
    // unsigned char *pcmBuf = (unsigned char *)malloc(samples);

    // for (i=0; i <samples; i++)
    // {
    //  wav = (wave1[i] + wave2[i] + wave3[i] + wave4[i] + wave5[i]) / 5;
    //  //单声道 8位数据
    //  pcmBuf[i] = wav;
    // }
    // ret = snd_pcm_writei(playback_handle, pcmBuf, samples);
    // if(-EPIPE == ret)
    // {
    //     snd_pcm_prepare(playback_handle);
    // }
    // free(pcmBuf);
    // return ;
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait()
{}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox(const char *pszMsg, ...)
{
    printf("MessageBox: %s \n", pszMsg);
}
