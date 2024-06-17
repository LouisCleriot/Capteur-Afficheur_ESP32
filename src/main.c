#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdlib.h>
#include <time.h>
#include "freertos/semphr.h"
#include "driver/gpio.h"

#define BUFFER_SIZE 10

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;
SemaphoreHandle_t write_slot;
SemaphoreHandle_t read_slot;

void ProducerTask(void *pvParameter) {
    int i;
    srand(time(NULL));
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(125);

    while(1) {
        xSemaphoreTake(write_slot, portMAX_DELAY); // wait for a write slot
        i = rand() % 2; // 0 or 1
        buffer[in] = i;
        in = (in + 1) % BUFFER_SIZE;

        printf("Produced %d\n", i);  
        xSemaphoreGive(read_slot); // signal a read slot
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void ConsumerTask(void *pvParameter) {
    int message;
    gpio_set_pull_mode(GPIO_NUM_18, GPIO_PULLUP_ONLY);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(250);

    while(1) {
        xSemaphoreTake(read_slot, portMAX_DELAY); // wait for a read slot
        message = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        printf("Consumed %d\n", message);
        xSemaphoreGive(write_slot); // signal a write slot
        gpio_set_level(GPIO_NUM_18, message);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void ConsumerTask2(void *pvParameter) {
    int message;
    gpio_set_pull_mode(GPIO_NUM_19, GPIO_PULLUP_ONLY);
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(250);

    while(1) {
        xSemaphoreTake(read_slot, portMAX_DELAY); // wait for a read slot
        message = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        printf("Consumed 2 %d \n", message);
        xSemaphoreGive(write_slot); // signal a write slot
        gpio_set_level(GPIO_NUM_19, message);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}



void app_main() {
    write_slot = xSemaphoreCreateCounting(10,10);
    read_slot = xSemaphoreCreateCounting(10,0);

    xTaskCreatePinnedToCore(ProducerTask, "ProducerTask", 10000, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(ConsumerTask, "ConsumerTask", 10000, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(ConsumerTask2, "ConsumerTask2", 10000, NULL, 1, NULL, 0);

    for (;;){
    };
}