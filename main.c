#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_demos/lv_demo.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "lv_drivers/indev/evdev.h"
#include "launcher/launcher.h"
#include "lv_fs_if/lv_fs_if.h"
#include "lv_lib_png/lv_png.h"

#define DISP_BUF_SIZE (256 * 1024)

int main(void)
{
    /*LittlevGL init*/
    lv_init();

    /*file style interface init*/
    lv_fs_if_init();

    /*png decode init*/
    lv_png_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf1[DISP_BUF_SIZE];
    static lv_color_t buf2[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE * 2);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 320;
    disp_drv.ver_res    = 240;
    lv_disp_drv_register(&disp_drv);

    evdev_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_drv_register(&indev_drv);

#if (LV_COLOR_DEPTH == 32)
    lv_indev_t *mouse_indev = lv_indev_get_next(NULL);
    lv_obj_t *cursor = lv_img_create(lv_scr_act());
    LV_IMG_DECLARE(mouse_black);
    lv_img_set_src(cursor, &mouse_black);
    lv_indev_set_cursor(mouse_indev, cursor);
#endif

    /*Create a Demo*/
    launcher_widgets();

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_task_handler();
        usleep(1000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
