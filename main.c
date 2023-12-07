/*
Никита, задачи ниже.
Нужно что-то одно. Линукс наверное в приоритете.

По Linux:
Написать модуль ядра линукс, который каждую минуту сохраняет текущее время в файле /tmp/current_time в формате hh:mm (например, 11:58 или 22:07).

По UEFI:
Написать программу для запуска в среде EFI Shell (есть эмулятор), которая находит все съёмные носители в системе и выводит их системные пути (device path).
Перед тем, как приступить к решению проверочной задачи, изучите пожалуйста спецификацию UEFI.
Хотя бы общее представление, механизмы тестирования: 
 Можно ориентироваться на EDK2.
https://www.tianocore.org/ 
В качестве эмулятора можно использовать QEMU, либо любая виртуальная машина с UEFI, любой реальный компьютер с UEFI. 
В EDK2 описано, как это сделать. 
Проверять нужно в EFI Shell.

Будут вопросы, то постараюсь ответить)
*/
#include <linux/module.h>       // Needed by all modules
#include <linux/kernel.h>       // Needed for KERN_INFO
#include <linux/init.h>         // Needed for the macros
#include <linux/timer.h>        // Needed for timers
#include <linux/fs.h>           // Needed for file operations
#include <linux/timekeeping.h>  // For getting system time
#include <linux/file.h>         // For struct file

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A Linux kernel module to write time to specified path and system log within delay.");
MODULE_VERSION("1.0");

const char* timestamp_path = "/home/nikit/current_time";
const size_t delay = 60000; // in milliseconds

static struct timer_list my_timer;
static char time_buffer[6]; // Buffer to hold time string

// Function to update time
static void update_time(struct timer_list *t) {
    struct timespec64 ts;
    struct tm tm;
    
    // Get the current time
    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &tm);

    // Format the time into the buffer
    snprintf(time_buffer, 6, "%02d:%02d", tm.tm_hour, tm.tm_min);

    // Set up the timer for the next minute
    mod_timer(t, jiffies + msecs_to_jiffies(delay));

    // Write time to a file in /tmp
    struct file *f = filp_open(timestamp_path, O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (!IS_ERR(f)) {
        loff_t pos = 0;
        ssize_t written = kernel_write(f, time_buffer, 5, &pos);
        if (written < 0) {
            printk(KERN_ERR "Failed to write to file %s\n", timestamp_path);
        } else {
            vfs_fsync(f, 0);
            printk(KERN_INFO "Wrote current Time(%s) to the timestamp path(%s)\n", time_buffer, timestamp_path);
        }
        filp_close(f, NULL);
    } else {
        printk(KERN_ERR "Failed to open file %s\n", timestamp_path);
    }
}

static int __init time_module_init(void) {
    printk(KERN_INFO "TimeModule: Initialized\n");

    // Initialize the timer
    timer_setup(&my_timer, update_time, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(delay));

    return 0;
}

static void __exit time_module_exit(void) {
    del_timer(&my_timer);
    printk(KERN_INFO "TimeModule: Removed\n");
}

module_init(time_module_init);
module_exit(time_module_exit);