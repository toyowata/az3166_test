/* WiFi Example
 * Copyright (c) 2016-2020 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
//#ifndef MBED_TEST_MODE
#if 0

DigitalOut LED_USER(LED1, 0);
DigitalOut LED_AZURE(LED2, 0);
DigitalOut LED_WIFI(LED3, 0);

DigitalOut LED_R(LED_RED, 0);
DigitalOut LED_G(LED_GREEN, 0);
DigitalOut LED_B(LED_BLUE, 0);

InterruptIn button_a(USER_BUTTON_A);
InterruptIn button_b(USER_BUTTON_B);

// I2C OLDE 128x64
// PB_8 - SCL
// PB_9 - SDA

EventQueue queue;
WiFiInterface *wifi;

void idle_handler(void);
void button_a_handler(void);
void button_b_handler(void);

void idle_handler()
{
    LED_R = !LED_R;
}

void button_a_handler() 
{
    LED_G = !LED_G;
}

void button_b_handler() 
{
    LED_B = !LED_B;
}

const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

int scan_demo(WiFiInterface *wifi)
{
    WiFiAccessPoint *ap;

    printf("Scan:\n");

    int count = wifi->scan(NULL, 0);

    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }

    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;

    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count);

    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }

    LED_WIFI = 1;
    for (int i = 0; i < count; i++) {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d networks available.\n", count);

    delete[] ap;
    return count;
}

int main()
{
    printf("\nIoT DevKit AZ3166 Wi-Fi example\n");
    
#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }

    int count = scan_demo(wifi);
    if (count == 0) {
        printf("No WIFI APs found - can't continue further.\n");
        return -1;
    }

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    SocketAddress a;
    wifi->get_ip_address(&a);
    printf("IP: %s\n", a.get_ip_address());
    wifi->get_netmask(&a);
    printf("Netmask: %s\n", a.get_ip_address());
    wifi->get_gateway(&a);
    printf("Gateway: %s\n", a.get_ip_address());
    printf("RSSI: %d\n\n", wifi->get_rssi());

    wifi->disconnect();
    printf("\nDone\n");
    LED_WIFI = 0;

    button_a.mode(PullUp);
    button_a.fall(queue.event(button_a_handler));
    button_b.mode(PullUp);
    button_b.fall(queue.event(button_b_handler));

    queue.call_every(500, idle_handler);

    queue.dispatch();

}

#else

#include "mbed.h"
#include "QSPIFBlockDevice.h"

QSPIFBlockDevice block_device(QSPI_FLASH1_IO0, QSPI_FLASH1_IO1, QSPI_FLASH1_IO2, QSPI_FLASH1_IO3,
                              QSPI_FLASH1_SCK, QSPI_FLASH1_CSN, QSPIF_POLARITY_MODE_0, MBED_CONF_QSPIF_QSPI_FREQ);

int main()
{
    printf("QSPI SFDP Flash Block Device example\n");

    // Initialize the SPI flash device and print the memory layout
    block_device.init();
    bd_size_t sector_size_at_address_0 = block_device.get_erase_size(0);

    printf("QSPIF BD size: %llu\n",         block_device.size());
    printf("QSPIF BD read size: %llu\n",    block_device.get_read_size());
    printf("QSPIF BD program size: %llu\n", block_device.get_program_size());
    printf("QSPIF BD erase size (at address 0): %llu\n", sector_size_at_address_0);

    // Write "Hello World!" to the first block
    char *buffer = (char *) malloc(sector_size_at_address_0);
    sprintf(buffer, "Hello World!\n");
    block_device.erase(0, sector_size_at_address_0);
    block_device.program(buffer, 0, sector_size_at_address_0);

    // Read back what was stored
    block_device.read(buffer, 0, sector_size_at_address_0);
    printf("%s", buffer);

    // Deinitialize the device
    block_device.deinit();
    printf("Test finished.\n");
}
#endif
