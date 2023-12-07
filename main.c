#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

char binary_to_byte(const char *bin) {
    char byte = 0;
    for (int i = 0; i < 8; i++) {
        if (bin[i] == '1') {
            byte |= (1 << (7 - i));
        }
    }
    return byte;
}

#define CYPHERED_MESSAGE "01001000 01100101 01101100 01101100 01101111 00100000 01110111 01101111 01110010 01101100 01100100 00100001"

static int __init kernel_init(void) {
    
    size_t cyphered_len = strlen(CYPHERED_MESSAGE) + 1;
    size_t cyphered_size = cyphered_len * sizeof(char);

    char* cyphered = (char*)kmalloc(cyphered_size, GFP_KERNEL);
    char* decyphered = (char*)kmalloc(cyphered_size / 8 + 1, GFP_KERNEL);
    
    strcpy(cyphered, CYPHERED_MESSAGE);

    size_t result_pos = 0;
    for (int i = 0; i < cyphered_len; i += 8, i++) {
        decyphered[result_pos++] = binary_to_byte(cyphered + i);
    }
    decyphered[result_pos] = '\0';
    printk(KERN_INFO "%s\n", decyphered);

    kfree(cyphered);
    kfree(decyphered);                                  
    return 0;
}

static void __exit kernel_exit(void) {
    printk(KERN_INFO "Module removed\n");
}

module_init(kernel_init);
module_exit(kernel_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Novikov Nikita");
MODULE_DESCRIPTION("A simple Hello World kernel module");
