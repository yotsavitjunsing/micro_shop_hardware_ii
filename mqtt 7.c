#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "Lab11.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTT";
#define GPIO_INPUT_IO_2  5
#define GPIO_INPUT_PIN_INTR_SEL (1ULL<<GPIO_INPUT_IO_2)

//set uart
#define ECHO_TEST_TXD (17)
#define ECHO_TEST_RXD (16)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)
 
#define ECHO_UART_PORT_NUM  (2)
#define ECHO_UART_BAUD_RATE  (9600)
#define ECHO_TASK_STACK_SIZE    (2048)
static const char *TAG2 = "UART TEST";
#define BUF_SIZE (1024)
QueueHandle_t queue;
esp_mqtt_client_handle_t client;
uint8_t *sender;
static void echo_task(void *arg)
{
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;
 
    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));
 
 
  
    
    while (1) {
       
        
        
        
        
         
       
 
        //uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
        uart_read_bytes(ECHO_UART_PORT_NUM, sender, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        // int len = strlen((const char *)sender);

        // if (len) 
        // {
        //     sender[len] = '\0';
        //     ESP_LOGI(TAG2, "Recv str: %s \n", (char *)sender);
            
        // } 

        // if(gpio_get_level(GPIO_INPUT_IO_2)==0)
        // {
            
        // }
        // printf("button = %d\n",gpio_get_level(GPIO_INPUT_IO_2));
        vTaskDelay(pdMS_TO_TICKS(1000)); 
        
        
        

        
       
    }
}

void taskConsumer(void *pvParameter)
{
    
    while(1)
    {
        if(gpio_get_level(GPIO_INPUT_IO_2)==0)
        {  
          esp_mqtt_client_publish(client, "presence", (char *)sender, 0, 1, 0);
          printf("sender data");
          vTaskDelay(pdMS_TO_TICKS(5000)); 
          
        }
           int len = strlen((const char *)sender);

          if (len) 
          {
              sender[len] = '\0';
              ESP_LOGI(TAG2, "[CONSUMER]: str: %s \n", (char *)sender);
              vTaskDelay(pdMS_TO_TICKS(1000));
          } 
         
                   
        
    }
}

static void log_error_if_nonzero(const char *message, int error_code)
{
  if(error_code != 0)
  {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  
  switch((esp_mqtt_event_id_t)event_id)
  {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      esp_mqtt_client_publish(client, "presence", "data for qos1", 0, 1, 0);
      esp_mqtt_client_subscribe(client, "presence", 0);
      
      break;

    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
      break;

    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      esp_mqtt_client_publish(client, "presence", "data for qos0", 0, 0, 0);
      break;

    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
      break;

    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      
      break;

    case MQTT_EVENT_DATA:
      ESP_LOGI(TAG, "MQTT_EVENT_DATA");
      printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
      printf("DATA=%.*s\r\n", event->data_len, event->data);
      break;

    case MQTT_EVENT_ERROR:
      ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
      if(event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
      {
        log_error_if_nonzero("reported from esp_tls", event->error_handle->esp_tls_last_esp_err);
        log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
        log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
        ESP_LOGI(TAG, "Last error string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

      }
      break;

    default:
      ESP_LOGI(TAG, "Other event id:%d", event->event_id);
      break;
  }
}

void mqtt_app_start(void)
{
  esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = CONFIG_BROKER_URL,
  };

    sender = (uint8_t *) malloc(BUF_SIZE);

   client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
  esp_mqtt_client_start(client);
  //  queue = xQueueCreate( 10, sizeof( uint32_t) );
    gpio_config_t io_conf ={};
    
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_INTR_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
      
    
       xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
       xTaskCreate(taskConsumer, "Consumer", 2048, NULL, 10, NULL);
    //vTaskDelay(pdMS_TO_TICKS(1000)); 
    
   
}
