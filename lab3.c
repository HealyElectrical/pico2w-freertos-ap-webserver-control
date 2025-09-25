//this code works with android web browser. the big thing is you need make the ip address static., but there is a modification from the original code
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"
#include "lwip/api.h"

#include "FreeRTOS.h"
#include "task.h"

#define AP_SSID     "PicoAP-Glen"
#define AP_PW       "password"
#define LED_PIN    16
#define TIMER_PIN  17

// Global variables
int alarm;                 // which alarm to use (0-3)
int timer_delay;           // delay between alarm interrupts in us
int led_val;   // <--- ADD THIS

// HTML page for timer input
static const char html_page[] =
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<!doctype html><html>"
"<h1>" AP_SSID "</h1>"
"<form action=/ method=get>"
"Timer period (us): <input name=q>"
"<input type=submit value=\"Update\">"
"</form>"
"</html>";


static const char error_page[] =
"HTTP/1.0 404 Not Found\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n\r\n"
"<html><body><h1>404 Not Found</h1></body></html>";

void ap_task() {
    
    // Start AP
    cyw43_arch_init() && printf("Wi-Fi init failed\n");
    cyw43_arch_enable_ap_mode(AP_SSID, AP_PW, CYW43_AUTH_WPA2_AES_PSK);
    printf("Started AP " AP_SSID " password=" AP_PW "\n");

    // server socket
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = PP_HTONS(80),
        .sin_addr.s_addr = PP_HTONL(INADDR_ANY) } ;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(fd, 1);

    while (1) {
        char buf[256];
        printf("Waiting for connection\n");

        int client_fd = accept(fd, NULL, NULL);
        int n = recv(client_fd, buf, sizeof(buf) - 1, 0);

        printf("Request: %s\n", buf);

        if (n > 0) {
            buf[n] = 0;

            // Check for timer period input (?q=nnn)
            int new_delay;
            if (strncmp(buf, "GET / ", 6) == 0 || strncmp(buf, "GET /HTTP", 9) == 0) {
    send(client_fd, html_page, sizeof(html_page) - 1, 0);
}
else if (strncmp(buf, "GET /generate_204", 17) == 0 ||
         strncmp(buf, "GET /favicon.ico", 16) == 0) {
    // ignore Android/Chrome background checks
    send(client_fd, html_page, sizeof(html_page) - 1, 0);
}
else if (sscanf(buf, "GET /?q=%d", &new_delay) == 1) {
    if (new_delay >= 2) timer_delay = new_delay;
    printf("Updated timer period to %d us\n", timer_delay);
    send(client_fd, html_page, sizeof(html_page) - 1, 0);
}
else if (sscanf(buf, "GET /%d", &led_val) == 1) {
    gpio_put(LED_PIN, led_val);
    printf("LED set to %d\n", led_val);
    send(client_fd, html_page, sizeof(html_page) - 1, 0);
}
else {
    send(client_fd, error_page, sizeof(error_page) - 1, 0);
}


            // flush rest of request
            shutdown(client_fd, SHUT_WR);
            while (recv(client_fd, buf, sizeof(buf), 0) > 0);
            close(client_fd);
        }
    }
}

// timer alarm ISR
void alarm_isr(void) {
    static absolute_time_t next = 0;

    gpio_set_mask(1u << TIMER_PIN);                // ISR begins: set pin high
    hw_clear_bits(&timer_hw->intr, 1u << alarm);   // clear interrupt

    next = delayed_by_us(next, timer_delay);       // set next IRQ time

    // fix up if next alarm in the past or too far in future
    absolute_time_t delay = absolute_time_diff_us(get_absolute_time(), next);
    if (delay > timer_delay || delay < 10)
        next = delayed_by_us(get_absolute_time(), timer_delay);

    hardware_alarm_set_target(alarm, next);

    gpio_clr_mask(1u << TIMER_PIN);                // ISR ends: set pin low
}

void timer_task() {
    // set up timer output pin (for 'scope)
    gpio_init(TIMER_PIN);
    gpio_set_dir(TIMER_PIN, true);

    // get a free timer alarm (0-3) and its ISR (also 0-3)
    alarm = hardware_alarm_claim_unused(1);
    int irq = timer_hardware_alarm_get_irq_num(timer_hw, alarm);

    // set ISR, IRQ at highest priority, and enable on NVIC and alarm
    irq_set_exclusive_handler(irq, alarm_isr);
    irq_set_priority(irq, 0);
    irq_set_enabled(irq, true);
    hw_set_bits(&timer_hw->inte, 1u << alarm);

    // start alarms with 100us delays
    timer_delay = 100;
    alarm_isr();

    vTaskDelete(NULL);  // all done
}

void serial_task() {
    while (1) {
        printf("Enter LED value (0=off, 1=on): ");
        if (fscanf(stdin, "%d", &led_val) == 1) {
            gpio_put(LED_PIN, led_val);
            printf("LED set to %d\n", led_val);
        }
    }
}

int main(void) {
    stdio_init_all();

    // set up LED GPIO pin once
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);

    // Create tasks
    xTaskCreate(timer_task, "Timer_Task", 1024, NULL, 1, NULL);
    xTaskCreate(serial_task, "Serial_Task", 1024, NULL, 1, NULL);
    xTaskCreate(ap_task, "AP_Task", 4096, NULL, 1, NULL);

    // Start FreeRTOS scheduler
    vTaskStartScheduler();
}
