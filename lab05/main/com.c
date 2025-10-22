

#include "freertos/FreeRTOS.h"

#include "com.h"
#include <stdint.h>
#include "net.h"
#include "graphics.h"
#include "lcd.h"

// This component is a wrapper around a lower-level communication
// method such as a serial port (UART), Bluetooth, or WiFi. The
// complexities of establishing a communication channel and sending
// bytes through the channel are abstracted away. The read and write
// functions are non-blocking so they can be used in a tick function.

#define GROUP_ID 1
#define GROUP_WAIT 4000 // ms


// Initialize the communication channel.
// Return zero if successful, or non-zero otherwise.
int32_t com_init(void) {
    net_init();
    graphics_drawMessage("Waiting to join group...", WHITE, BLACK);
    net_group_open(GROUP_ID);
    vTaskDelay(pdMS_TO_TICKS(GROUP_WAIT));
    return net_group_close();
}

// Free resources used for communication.
// Return zero if successful, or non-zero otherwise.
int32_t com_deinit(void) {
    return net_deinit();
}

// Write data to the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to write
// Return number of bytes written, or negative number if error.
int32_t com_write(const void *buf, uint32_t size) {
    return net_send(NULL, buf, size, 0);
}

// Read data from the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to read
// Return number of bytes read, or negative number if error.
int32_t com_read(void *buf, uint32_t size) {
    uint8_t src[NET_ALEN];
    return net_recv(src, buf, size, 0);
}
