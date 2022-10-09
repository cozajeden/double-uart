#include <Arduino.h>
#include <driver/uart.h>

char test_str[] = "This is a test string.\n";
static const char *TAG = "uart_events";
const uart_port_t uart_num = UART_NUM_2;
const uart_port_t uart_num2 = UART_NUM_1;
uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
};
// Setup UART buffered IO with event queue
QueueHandle_t queue;
QueueHandle_t uart_queue1;
QueueHandle_t uart_queue2;
const int uart_buffer_size = (1024 * 2);
const int queue_size = 20;

struct event_data {
    int uart_num;
    uart_event_t event;
};

void rec_task(void *pvParameters)
{
    event_data event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(uart_buffer_size);
    for(;;) {
        // Waiting for UART event.
        if(xQueueReceive(queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            switch(event.event.type) {
                case UART_DATA:
                    bzero(dtmp, uart_buffer_size);
                    uart_get_buffered_data_len(event.uart_num, &buffered_size);
                    uart_read_bytes(event.uart_num, dtmp, buffered_size, portMAX_DELAY);
                    printf("uart[%d] read: %s", event.uart_num, dtmp);
                    break;
                default:
                    break;
            }
        }
    }
}

void uart_task(void *pvParameters)
{
    QueueHandle_t* uart_queue = (QueueHandle_t*) pvParameters;
    event_data event;
    event.uart_num = uart_num;
    for(;;) {
        // Waiting for UART event.
        if(xQueueReceive(*uart_queue, (void * )&event.event, (portTickType)portMAX_DELAY)) {
            xQueueSend(queue, (void * )&event, (portTickType)portMAX_DELAY);
        }
    }
}


void configure_uart(uart_port_t uart, QueueHandle_t* uart_queue, int tx_pin, int rx_pin) {
  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(uart, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(uart, tx_pin, rx_pin, -1, -1));
  // Install UART driver using an event queue here
  ESP_ERROR_CHECK(uart_driver_install(uart, uart_buffer_size*2, uart_buffer_size*2, queue_size, uart_queue, 0));
}


void setup() {
  queue = xQueueCreate(queue_size, sizeof(event_data));
  Serial.begin(115200);
  
  // Configure UART parameters
  configure_uart(uart_num, &uart_queue1, 0, 26);
  configure_uart(uart_num2, &uart_queue2, 32, 33);

  // Create a task to handler UART event from ISR
  xTaskCreate(rec_task, "uart_rec_task", 2048, NULL, 12, NULL);
  xTaskCreate(uart_task, "u1t", 2048, (void*)&uart_queue1, 15, NULL);
  xTaskCreate(uart_task, "u2t", 2048, (void*)&uart_queue2, 18, NULL);
}

void loop() {
  // Write data to UART.
  uart_write_bytes_with_break(uart_num, (const char*)test_str, strlen(test_str), 100);
  delay(1000);
}