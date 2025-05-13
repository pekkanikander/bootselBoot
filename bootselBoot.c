//
//  main.c
//  libusbtest
//
//  Created by decafish on 2021/05/02.
//

#include <stdio.h>
#include <libusb.h>
#include <sysexits.h>

#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))
#endif

enum usb_type { UF2, CMSIS, PYTHON, CDC, HUB };

/* See https://github.com/raspberrypi/usb-pid */
const uint16_t vendor_id = 0x2E8A;
const struct { uint16_t id; enum usb_type type; } product_ids[] = {
    { 0x0003, UF2    }, /* RP2040 UF2 bootloader (mass-storage) */
    { 0x0004, CMSIS  }, /* PicoProbe (CMSIS-DAP debug probe) */
    { 0x0005, PYTHON }, /* Pico MicroPython firmware (CDC serial) */
    { 0x0009, CDC    }, /* Pico SDK CDC-UART */
    { 0x000A, CDC    }, /* Pico SDK CDC-UART (alternate RP2040) */
    { 0x000B, PYTHON }, /* Pico CircuitPython firmware */
    { 0x000C, CMSIS  }, /* RP2040 CMSIS-DAP debug adapter */
    { 0x000D, HUB    }, /* USB3HUB — USB2-hub function */
    { 0x000E, HUB    }, /* USB3HUB — USB3-hub function */
    { 0x000F, UF2    }, /* RP2350 UF2 bootloader */
};

void    logcallback(libusb_context *ctx, enum libusb_log_level level, const char *str)
{
    fprintf(stderr, "level%1d: %s\n", level, str);
}

// reset to BOOTSEL
#define RESET_REQUEST_BOOTSEL 0x01
// regular flash boot
#define RESET_REQUEST_FLASH 0x02

int main(int argc, const char * argv[])
{
    uint8_t     request = RESET_REQUEST_BOOTSEL;
    if (argc > 1) {
        switch (*argv[1]) {
            case 'B':
            case 'b':
                request = RESET_REQUEST_BOOTSEL;
                break;
            case 'F':
            case 'f':
                request = RESET_REQUEST_FLASH;
                break;
            default:
                fprintf(stderr, "%s: Reset Pi Pico via. USB\n", argv[0]);
                fprintf(stderr, "\t$ %s B[OOTSEL] to reset to BOOTSEL mode.\n", argv[0]);
                fprintf(stderr, "\t$ %s F[LASH] to reset to regular boot mode.\n", argv[0]);
                fprintf(stderr, "\twith no argument means B[OOTSEL] specified.\n");
                return EX_OK;
        }
    }
    int ret = libusb_init(NULL);
    if (ret != 0) {
        return EX_IOERR;
    }
    //  libusb_set_log_cb(NULL, logcallback, LIBUSB_LOG_CB_GLOBAL);
    
    libusb_device_handle    *handle = NULL;

    int i = 0;
    while (i < ARRAY_SIZE(product_ids) && handle == NULL) {
        handle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_ids[i].id);
        if (handle != NULL)
            break;
        i++;
    }
    if (handle == NULL) {
        fprintf(stderr, "No pico device available or a device has no standard pico_stdio_usb module.\n");
        libusb_exit(NULL);
        return EX_UNAVAILABLE;
    }
    switch (product_ids[i].type) {
    case CDC:
        /* libusb based firmware, proceed to reboot */
        break;
    case UF2:
        /* Already in UF2 mode, nothing to do */
        libusb_exit(NULL);
        return EX_OK;
    case CMSIS:
    case PYTHON:
    case HUB:
    default:
        fprintf(stderr,
            "Pico device VID/PID %04x/%04x found, but does not have a known pico_stdio_usb module.\n",
            vendor_id, product_ids[i].id);
        libusb_exit(NULL);
        return EX_TEMPFAIL;
    }
    
    int interface_number = 2;
    ret = libusb_claim_interface(handle, interface_number);
    if (ret != 0) {
        fprintf(stderr,
            "Pico device VID/PID 0x%04x/0x%0xx found, but does not have USB interface #2.\n",
            vendor_id, product_ids[i].id);
        libusb_exit(NULL);
        return EX_PROTOCOL;
    }
    
    uint8_t     reqtype = (0 << 6)  //  standard request host to device
                            | (1);  // to interface
    uint16_t    windex = interface_number;
    uint16_t    wvalue = 0;
    //unsigned char   data[4];
    ret = libusb_control_transfer(handle, reqtype, request, wvalue, windex, NULL, 0, 10);
    
    //printf("return value from control transfer = %d\n", ret);
    libusb_exit(NULL);
    
    return EX_OK;
}
