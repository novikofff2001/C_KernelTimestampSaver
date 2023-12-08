#include <linux/module.h>       // Needed by all modules
#include <linux/kernel.h>       // Needed for KERN_INFO
#include <linux/init.h>         // Needed for the macros
#include <linux/timer.h>        // Needed for timers
#include <linux/fs.h>           // Needed for file operations
#include <linux/timekeeping.h>  // For getting system time
#include <linux/file.h>         // For struct file

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nikita Novikov");
MODULE_DESCRIPTION("A Linux kernel module to write time to specified path and system log within delay.");
MODULE_VERSION("1.0");

const char* timestamp_path = "/tmp/current_time";
const size_t delay = 5000; // in milliseconds

static struct timer_list my_timer;
static char time_buffer[6]; // Buffer to hold time string

static int timezone_offset = 180; // Timezone offset in minutes (GMT+3 for Moscow)
module_param(timezone_offset, int, 0644);
MODULE_PARM_DESC(timezone_offset, "Offset in minutes from UTC");

// Function to update time
static void update_time(struct timer_list *t) {
    struct timespec64 ts;
    struct tm tm;
    
    // Get the current time
    ktime_get_real_ts64(&ts);
    ts.tv_sec += timezone_offset * 60; // Adjust time with GMT offset in seconds
    time64_to_tm(ts.tv_sec, 0, &tm);

    // Format the time into the buffer
    snprintf(time_buffer, 6, "%02d:%02d", tm.tm_hour, tm.tm_min);

    // Set up the timer for the next minute
    mod_timer(t, jiffies + msecs_to_jiffies(delay));

    struct file *f = filp_open(timestamp_path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (IS_ERR(f)) {
        printk(KERN_ERR "Failed to open file %s\n", timestamp_path);
        return;
    }

    // Write time to a file in /tmp
    loff_t pos = 0;
    ssize_t written = kernel_write(f, time_buffer, 5, &pos);

    if (written < 0) {
        printk(KERN_ERR "Failed to write to file %s\n", timestamp_path);
    } else {
        printk(KERN_INFO "Wrote current Time(%s) to the timestamp path(%s)\n", time_buffer, timestamp_path);
    }
    filp_close(f, NULL);
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