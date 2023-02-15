#ifndef TEST_ITEM_H_
#define TEST_ITEM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define FACTORY_VERSION "1.2"

enum STB_MODE {
        MODE_APP,
        MODE_TEST
};

void *do_audio_capture_test(void *param);
void *do_audio_playback_test(void *param);
void *do_can_test(void *param);
void *do_display_test(void *param);
void *do_flash_test(void *param);
void *do_hdmiin_test(void *param);
void *do_ir_test(void *param);
void *do_msg_router(void *param);
void *do_net_test(void *param);
void *do_rtc_test(void *param);
void *do_uart_test(void *param);
void *do_gps_test(void *param);
void *do_usb_test(void *param);
void *do_4g_test(void *param);
void *get_firmware_version(void *param);
void *handle_stb_mode(void *param);
void *handle_bsn(void *param);
void *handle_psn(void *param);
void *handle_mac(void *param);
void *run_command(void *param);

void display_test_pattern(void);

#define PROP_SLAVE_IP "test.config.slaveip"
#define SLAVE_MAGIC_NUM 20

#ifdef __cplusplus
}
#endif

#endif
