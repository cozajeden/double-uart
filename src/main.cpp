#include <Arduino.h>
#include <driver/uart.h>

char* test_str = "This is a test string.\n";

const uart_port_t uart_num = UART_NUM_2;
uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
};
// Setup UART buffered IO with event queue
QueueHandle_t uart_queue;
const int uart_buffer_size = (1024 * 2);
const int queue_size = 20;

void setup() {
  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, 0, 26, -1, -1));
  // Install UART driver using an event queue here
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, \
                                          uart_buffer_size, strlen(test_str), &uart_queue, 0));
}

void loop() {
  // Write data to UART.
  // uart_write_bytes(uart_num, (const char*)test_str, strlen(test_str));
  uart_write_bytes_with_break(uart_num, (const char*)test_str, strlen(test_str), 100);
  delay(1000);
}