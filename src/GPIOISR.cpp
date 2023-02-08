#include <GPIOISR.h>

static xQueueHandle gpio_evt_queue = NULL; //定义一个队列返回变量

void IRAM_ATTR gpio_isr_handler(void* arg) 
{
    //把中断消息插入到队列的后面，将gpio的io参数传递到队列中
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void gpio_ISR_task(void* arg) 
{
    ESP_LOGI(INFO_DEBUG, "start gpio ISR ...");
    Flash_read_flag = 1;
}

void gpio_ISR_init(void)
{
    gpio_config_t io_conf;  // 定义一个gpio_config类型的结构体，下面的都算对其进行的配置

    io_conf.intr_type = GPIO_INTR_LOW_LEVEL;      // 低电平触发
    io_conf.mode = GPIO_MODE_INPUT;             // 选择输入模式
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;  // 配置GPIO_IN寄存器
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;    // 内部上拉

    gpio_config(&io_conf);                      // 最后配置使能
}

void gpio_intr_init(void)
{
    // 注册中断服务
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // 设置GPIO的中断回调函数
    gpio_isr_handler_add(GPIO_ISR, gpio_isr_handler, (void*) GPIO_ISR);
    // 创建一个消息队列，从中获取队列句柄
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // 创建GPIO检测任务
    xTaskCreate(gpio_ISR_task         // 任务函数
            , "gpio_task_example" // 任务名字
            , 256               // 任务堆栈大小
            , NULL                // 传递给任务函数的参数
            , 10                  // 任务优先级
            , NULL);              // 任務句柄
}

