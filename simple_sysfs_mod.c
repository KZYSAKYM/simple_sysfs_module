#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/stat.h>

/* This is your dedicated struct 
 * We can declare kobj_attribute members 
 * as static variables.
 * (Maybe, such way is popular 
 * and enough in almost cases)
 * e.g.
 * static struct kobj_attribute 
 * attr1 = __ATTR(...);
 * static struct kobj_attribute 
 * attr2 == __ATTR(...);
 * static struct attribute* 
 * attrs[] = { &attr1.attr, &attr2.attr };
 * struct attribute_group attr_group = {
 *   .attrs = attrs,
 * };
 **/
struct simple_sysfs_data {
    struct kobject* kobj;
    struct kobj_attribute attr1;
    struct kobj_attribute attr2;
    struct attribute *attrs[3];
    struct attribute_group attr_group;
    int data_1;
    int data_2;
};

static struct simple_sysfs_data simple_sysfs_mod;

#define SIMPLE_SYSFS_DATA_MAX 1000
#define SIMPLE_SYSFS_DATA_MIN 0

/* The callback for `read`. 
 * You should check the definition of kobj_attribute.
 * It's possible to find the definition of callback and its types. 
 **/
static ssize_t simple_sysfs_mod_show(struct kobject* kobj,
                                     struct kobj_attribute* attr,
                                     char* buf)
{
    bool is_one = strcmp(attr->attr.name, simple_sysfs_mod.attr1.attr.name);
    printk("Call %s\n", __func__);
    /* Need to return the size of written data to buf.
     * I think scnprintf is safer than snprintf.
     **/
    if (is_one)
        return scnprintf(buf, PAGE_SIZE, "Current Data: %d\n", simple_sysfs_mod.data_1);
    else
        return scnprintf(buf, PAGE_SIZE, "Current Data: %d\n", simple_sysfs_mod.data_2);
}

/* The callback for `write`.
 * Need to return count in any case.
 **/
static ssize_t simple_sysfs_mod_store(struct kobject* kobj,
                                      struct kobj_attribute* attr,
                                      const char* buf, size_t count)
{
    int ret, data;
    bool is_one = strcmp(attr->attr.name, simple_sysfs_mod.attr1.attr.name);
    printk("Call %s\n", __func__);
    ret = kstrtoint(buf, 10, &data);
    printk("Finish parse val %d ret %d\n", data, ret);
    if (ret < 0){
        printk("[%s] Error: Invalid data\n", __func__);
        goto finish;
    }
    if (SIMPLE_SYSFS_DATA_MIN > data || SIMPLE_SYSFS_DATA_MAX < data){
        printk("[%s] Error: Invalid data\n", __func__);
        goto finish;
    }
    if (is_one){
        simple_sysfs_mod.data_1 = data;
        printk("Store data_1\n");
    } else {
        simple_sysfs_mod.data_2 = data;
        printk("Store data_2\n");
    }
finish:
    return count;
}

/* Note: If you wanna know how to get your struct instance
 * in case you dynamically allocate your data with using e.g. kmalloc.
 * In such case, it's possible to get your instance by using container_of.
 * struct simple_sysfs_data* simple_sysfs_mod = container_of(
 *   kobj, struct simple_sysfs_data, kobj);
 **/


/* This is initialization function for this module.
 * When creating kernel modules, you need to create such functions and
 * notify to kernel that this is initialization of your module
 * with using e.g. module_init().
 * It is same in exit function,
 **/
static int simple_sysfs_mod_init(void)
{
    int ret;
    struct kobj_attribute attr = __ATTR(
        simple_sysfs_data, 
        0664, 
        simple_sysfs_mod_show, 
        simple_sysfs_mod_store
    );
    /* create kobject(=directory) and add 
     * the parent(=kobject of this module) into it.
     **/
    simple_sysfs_mod.kobj = kobject_create_and_add(
        "simple_sysfs", &THIS_MODULE->mkobj.kobj);
    if (!simple_sysfs_mod.kobj)
        return -ENOMEM;
    /* attr.name reperesents a file name */
    simple_sysfs_mod.attr1 = attr;
    simple_sysfs_mod.attr1.attr.name = "simple_sysfs_data_1";
    simple_sysfs_mod.attr2 = attr;
    simple_sysfs_mod.attr2.attr.name = "simple_sysfs_data_2";
    simple_sysfs_mod.attrs[0] = &simple_sysfs_mod.attr1.attr;
    simple_sysfs_mod.attrs[1] = &simple_sysfs_mod.attr2.attr;
    simple_sysfs_mod.attrs[2] = NULL;
    simple_sysfs_mod.attr_group.attrs = simple_sysfs_mod.attrs;

    ret = sysfs_create_group(simple_sysfs_mod.kobj, &simple_sysfs_mod.attr_group);
    if(ret)
        goto free_kobj;
    printk("Load simple_sysfs_mod\n");
    return 0;

free_kobj:
    kobject_put(simple_sysfs_mod.kobj);
    return ret;
}

static void simple_sysfs_mod_exit(void)
{
    
     kobject_put(simple_sysfs_mod.kobj);
     printk("Unload simple_sysfs_mod\n");
}

module_init(simple_sysfs_mod_init);
module_exit(simple_sysfs_mod_exit);

MODULE_LICENSE("GPL v2");
/* We should sign with the name and email.
 * But, I don't want to make my email address open.
 **/
MODULE_AUTHOR("KZYSAKYM <https://github.com/KZYSAKYM>");
MODULE_DESCRIPTION("Simple Sysfs file create module");
