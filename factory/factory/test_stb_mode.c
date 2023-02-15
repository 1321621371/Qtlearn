#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
//#include "cutils/properties.h"
//#include "minui_pcba/minui.h"
#include "packet.h"
#include "test_item.h"
//#include "ui.h"

static int stb_mode = MODE_APP;

#if 0
void display_test_pattern(void)
{
        int x =  0;
        int y = 0;
        int w =  gr_fb_width();
        int h = gr_fb_height();

        FillColor(255,255,255,255,x,y,w/8,h);           //white
        FillColor(0,255,255,255,x+w/8,y,w/8,h);         //yellow
        FillColor(255,255,0,255,x+(2*w)/8,y,w/8,h);     //cyan
        FillColor(0,255,0,255,x+(3*w)/8,y,w/8,h);       //green
        FillColor(139,34,104,255,x+(4*w)/8,y,w/8,h);    //dark orchid
        FillColor(0,0,255,255,x+(5*w)/8,y,w/8,h);       //red
        FillColor(255,0,0,255,x+(6*w)/8,y,w/8,h);       //blue
        FillColor(0,0,0,255,x+(7*w)/8,y,w/8,h);         //black
}
#endif

int get_stb_mode()
{
	return stb_mode;
}

static int set_stb_mode(char *param)
{
        LOGV("%s\n", __func__);

	if (param[0] == 'A') {
		stb_mode = MODE_APP;
	} else if (param[0] == 'T') {
		stb_mode = MODE_TEST;
		//display_test_pattern();

		char cmd[128] = {0};
		sprintf(cmd, "ps -aux | grep monitor | head -1 | awk '{print $2}' | xargs kill -9");
                system(cmd);

		sprintf(cmd, "ps -aux | grep BTU | head -1 | awk '{print $2}' | xargs kill -9");
                system(cmd);

		sprintf(cmd, "ps -aux | grep pppd | head -1 | awk '{print $2}' | xargs kill -9");
                system(cmd);

		sprintf(cmd, "ps -aux | grep \"xinput_calibrator --geometry\" | head -1 | awk '{print $2}' | xargs kill -9");
                system(cmd);

		sprintf(cmd, "fD=$(ps -aux | grep factory_display | grep -v grep); [ -z $fD ] && DISPLAY=:0.0 /usr/local/bin/factory_display &");
                system(cmd);
	} else {
		LOGE("unknown mode [%s]\n", param);
		return -1;
	}

        return 0;
}

void *handle_stb_mode(void *param)
{
	struct packet_info info;
	enum ITEM_TYPE type;
	char ret_string[32] = {0};
	int ret = -1;

        LOGV("%s\n", __func__);

	memcpy(&info, param, sizeof(struct packet_info));

	type = info.type;
	if (type == ITEM_TYPE_GET) {
		get_stb_mode();
	} else {
		set_stb_mode(info.param);
	}

        if (stb_mode == MODE_APP) {
                sprintf(ret_string, "App");
        } else {
                sprintf(ret_string, "Test");
        }

        send_response_packet(&info, ret_string, strlen(ret_string));

	return (void *)NULL;
}
