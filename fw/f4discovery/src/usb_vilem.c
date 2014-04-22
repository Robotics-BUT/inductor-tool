#include <stdlib.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>
#include "usb_vilem.h"

static int cdcacm_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
        uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    (void)complete;
    (void)buf;
    (void)usbd_dev;


    switch (req->bRequest)
    {
        case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
        {
        //
         //* This Linux cdc_acm driver requires this to be implemented
        // * even though it's optional in the CDC spec, and we don't
        // * advertise it in the ACM functional descriptor.
        //
            return 1;
        }
        case USB_CDC_REQ_SET_LINE_CODING:
            if (*len < sizeof(struct usb_cdc_line_coding))
                return 0;
            return 1;
        default:
            return 1;
    }

    return 0;
}


static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{

    (void)ep;
    char buf[64];
    int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);

    if (len) {
        while (usbd_ep_write_packet(usbd_dev, 0x82, buf, len) == 0)
            ;
    }

    gpio_toggle(GPIOC, GPIO5);
}


void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue)//puvodne static
{
    (void)wValue;

    usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
    usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, NULL);
    usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

    usbd_register_control_callback(
                usbd_dev,
                USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
                USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
                cdcacm_control_request);
}


