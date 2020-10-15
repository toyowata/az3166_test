/* IoT DevKit AZ3166 example
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
#include "NTPClient.h"
#include "HTS221Sensor.h"
#include "LPS22HBSensor.h"
#include "LIS2MDLSensor.h"
#include "LSM6DSLSensor.h"
#include "SSD1306.h"

DigitalOut LED_USER(LED1, 0);
DigitalOut LED_AZURE(LED2, 0);
DigitalOut LED_WIFI(LED3, 0);

DigitalOut LED_R(LED_RED, 0);
DigitalOut LED_G(LED_GREEN, 0);
DigitalOut LED_B(LED_BLUE, 0);

InterruptIn button_a(USER_BUTTON_A);
InterruptIn button_b(USER_BUTTON_B);

DevI2C devI2c(I2C_SDA, I2C_SCL);
HTS221Sensor hum_temp(&devI2c);
LPS22HBSensor press_temp(&devI2c, LPS22HB_ADDRESS_LOW);
LIS2MDLSensor magnetometer(&devI2c);
LSM6DSLSensor acc_gyro(&devI2c, LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW);
SSD1306 oled(&devI2c, (uint8_t)0x78);

EventQueue queue;
WiFiInterface *wifi;
static int toggle = 0;
time_t now;

void idle_handler(void);
void button_a_handler(void);
void button_b_handler(void);

void idle_handler()
{
    float value1, value2;
    int32_t axes[3];
    char buf[16];

    LED_R = !LED_R;
        
    time(&now);
    strftime(buf, sizeof(buf), "%Y/%m/%d %a", localtime(&now));
    oled.writeString(0, 0, buf);
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    oled.writeString(1, 0, buf);

    hum_temp.get_temperature(&value1);
    hum_temp.get_humidity(&value2);
    printf("HTS221:  [temp] %.2f C, [hum]   %.2f%%\r\n", value1, value2);
    sprintf(buf, "%.2f C, %.2f%%", value1, value2);
    oled.writeString(2, 0, buf);

    press_temp.get_temperature(&value1);
    press_temp.get_pressure(&value2);
    printf("LPS22HB: [temp] %.2f C, [press] %.2f mbar\r\n", value1, value2);
    sprintf(buf, "%.2f hPa", value2);
    oled.writeString(3, 0, buf);

    magnetometer.get_m_axes(axes);
    printf("LIS3MDL [mag/mgauss]:    %6ld, %6ld, %6ld\r\n", axes[0], axes[1], axes[2]);

    acc_gyro.get_x_axes(axes);
    printf("LSM6DSL [acc/mg]:        %6ld, %6ld, %6ld\r\n", axes[0], axes[1], axes[2]);
 
    acc_gyro.get_g_axes(axes);
    printf("LSM6DSL [gyro/mdps]:     %6ld, %6ld, %6ld\r\n", axes[0], axes[1], axes[2]);

    printf("\n");

    switch (toggle) {
        case 0:
            magnetometer.get_m_axes(axes);
            sprintf(buf, "[mag x] %6ld", axes[0]);
            oled.writeString(5, 0, buf);
            sprintf(buf, "[mag y] %6ld", axes[1]);
            oled.writeString(6, 0, buf);
            sprintf(buf, "[mag z] %6ld", axes[1]);
            oled.writeString(7, 0, buf);
            break;
        case 1:
            acc_gyro.get_x_axes(axes);
            sprintf(buf, "[acc x] %6ld", axes[0]);
            oled.writeString(5, 0, buf);
            sprintf(buf, "[acc y] %6ld", axes[1]);
            oled.writeString(6, 0, buf);
            sprintf(buf, "[acc z] %6ld", axes[1]);
            oled.writeString(7, 0, buf);
            break;
        case 2:
            acc_gyro.get_g_axes(axes);
            sprintf(buf, "[gyro x] %6ld", axes[0]);
            oled.writeString(5, 0, buf);
            sprintf(buf, "[gyro y] %6ld", axes[1]);
            oled.writeString(6, 0, buf);
            sprintf(buf, "[gyro z] %6ld", axes[1]);
            oled.writeString(7, 0, buf);
            break;
        default:
            break;
    }
}

void button_a_handler() 
{
    LED_G = !LED_G;

    toggle++;
    if (toggle > 2) {
        toggle = 0;
    }
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

void rtc_demo(NetworkInterface *network)
{
        // sync the real time clock (RTC)
    NTPClient ntp(network);
    const char* serverAddress = "time.google.com";
    int port = 123;
    ntp.set_server(const_cast<char *>(serverAddress), port);
    now = ntp.get_timestamp();
    now += (3600 * 9); // Adjust to JST timezone
    set_time(now);
    printf("Time is now %s", ctime(&now));
}

int main()
{
    printf("\nIoT DevKit AZ3166 Wi-Fi example\n");
    oled.writeString(0, 0, "[AZ3166 demo]");

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

    rtc_demo((NetworkInterface *)wifi);

    wifi->disconnect();
    printf("\nDone\n");
    LED_WIFI = 0;

    button_a.mode(PullUp);
    button_a.fall(queue.event(button_a_handler));
    button_b.mode(PullUp);
    button_b.fall(queue.event(button_b_handler));

    uint8_t id;
    hum_temp.init(NULL);
    hum_temp.enable();
    hum_temp.read_id(&id);
    printf("HTS221  humidity & temperature    = 0x%X\r\n", id);

    press_temp.init(NULL);
    press_temp.enable();
    press_temp.read_id(&id);
    printf("LPS22HB pressure & temperature    = 0x%X\r\n", id);

    magnetometer.init(NULL);
    magnetometer.enable();
    magnetometer.read_id(&id);
    printf("LIS3MDL magnetometer              = 0x%X\r\n", id);

    acc_gyro.init(NULL);
    acc_gyro.enable_g();acc_gyro.enable_x();
    acc_gyro.enable_g();
    acc_gyro.read_id(&id);
    printf("LSM6DSL accelerometer & gyroscope = 0x%X\r\n", id);

    printf("\n");
    
    queue.call_every(2000, idle_handler);

    queue.dispatch();

}
