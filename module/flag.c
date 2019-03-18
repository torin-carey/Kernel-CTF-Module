#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/semaphore.h>

#include "ioctl.h"
#include "sha256.h"

#include "flag.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Torin Carey <tcarey1@sheffield.ac.uk>");

#define FLAG_FLAG1 1
#define FLAG_FLAG2 2

static struct file_state {
	int auth;
	struct semaphore sem;
};

static const char mymessage[] = "Nothing to see here...\n";

static int mod_open(struct inode *inode, struct file *file) {
	struct file_state *state;
	
	state = kmalloc(sizeof(struct file_state), GFP_KERNEL);
	if (!state) {
		printk(KERN_ERR "failed to allocate memory\n");
		return -ENOMEM;
	}
	state->auth;
	sema_init(&state->sem, 1);
	file->private_data = (void *)state;
	return 0;
}

static int mod_release(struct inode *inode, struct file *file) {
	kfree(file->private_data);
	return 0;
}

static ssize_t mod_read(struct file *file, char __user *data, size_t size, loff_t *offset) {
	unsigned int avail;
	unsigned long w;
	if (*offset >= sizeof(mymessage) || *offset < 0)
		return 0;
	avail = sizeof(mymessage) - *offset;
	avail = avail > size ? size : avail;
	w = copy_to_user(data, mymessage, avail);
	*offset += w;
	return w;
}

static ssize_t mod_write(struct file *file, const char __user *data, size_t size, loff_t *offset) {
	return -EPERM;
}

static int authenticate(const struct auth_data *auth) {
	struct sha256_state state;
	unsigned char digest[32];
	int ret;
	if (auth->message_len > sizeof(auth->message))
		return -EINVAL;
	sha256_init(&state);
	sha256_update(&state, key, sizeof(key));
	sha256_update(&state, auth->message, auth->message_len);
	sha256_final(&state, digest);
	ret = memcmp(digest, auth->digest, 32);
	return ret ? -EPERM : 0;
}

static long mod_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	unsigned long ret;

	struct auth_data auth;
	struct file_state *state;
	void *sptr, *mptr, *eptr;
	state = (struct file_state *)file->private_data;

	printk(KERN_INFO "ioctl( %x , %lx )\n", cmd, arg);
	if (down_interruptible(&state->sem))
		return -EINTR;
	switch (cmd) {
	case IOCTL_GET_FLAG1:
		if (!(state->auth & FLAG_FLAG1)) {
			ret = -EPERM;
			goto finish;
		}
		ret = copy_to_user((char *)arg, flag1, sizeof(flag1));
		ret = sizeof(flag1) - ret;
		goto finish;
	case IOCTL_GET_FLAG2:
		if (!(state->auth & FLAG_FLAG2)) {
			ret = -EPERM;
			goto finish;
		}
		ret = copy_to_user((char *)arg, flag2, sizeof(flag2));
		ret = sizeof(flag2) - ret;
		goto finish;
	case IOCTL_AUTHENTICATE:
		printk(KERN_DEBUG "authenticating data...\n");
		if (copy_from_user(&auth, (void *)arg, sizeof(struct auth_data))) {
			printk(KERN_INFO "failed to authenticate\n");
			ret = -EINVAL;
			goto finish;
		}
		ret = authenticate(&auth);
		if (ret)
			goto finish;

		printk(KERN_DEBUG "data has correct mac\n");
		sptr = &auth.message;
		eptr = &auth.message[auth.message_len];
		while (sptr < eptr) {
			mptr = memchr(sptr, '\0', eptr - sptr);
			if (!mptr)
				mptr = eptr;

			if (mptr - sptr == 12) {
				if (!memcmp(sptr, "UNLOCK_FLAG1", 12)) {
					printk(KERN_INFO "flag1 unlocked\n");
					state->auth |= FLAG_FLAG1;
				} else if (!memcmp(sptr, "UNLOCK_FLAG2", 12)) {
					printk(KERN_INFO "flag2 unlocked\n");
					state->auth |= FLAG_FLAG2;
				}
			}
			sptr = mptr + 1;
		}

		ret = 0;
		goto finish;
	default:
		ret = -EINVAL;
	}
finish:
	up(&state->sem);
	return ret;
}


static struct file_operations mod_fops = {
	.owner = THIS_MODULE,
	.open = mod_open,
	.release = mod_release,
	.read = mod_read,
	.write = mod_write,
	.unlocked_ioctl = mod_ioctl,
	.llseek = no_llseek,
};

static int mod_uevent(struct device *dev, struct kobj_uevent_env *env) {
	add_uevent_var(env, "DEVMODE=%#o", 0444);
	return 0;
}

static dev_t mod_dev = 0;
static struct class mod_class = {
	.owner = THIS_MODULE,
	.name = "myflag",
	.dev_uevent = mod_uevent,
};
static struct cdev *mod_cdev;

static int __init mod_init(void) {
	int result;
	printk(KERN_DEBUG "initialising module\n");

	result = alloc_chrdev_region(&mod_dev, 0, 1, "mod_cdev");
	if (result < 0) {
		printk(KERN_ERR "failed to alloc chrdev region\n");
		goto fail_alloc_region;
	}
	mod_cdev = cdev_alloc();
	if (!mod_cdev) {
		result = -ENOMEM;
		printk(KERN_ERR "failed to alloc cdev\n");
		goto fail_alloc_cdev;
	}
	cdev_init(mod_cdev, &mod_fops);
	result = cdev_add(mod_cdev, mod_dev, 1);
	if (result < 0) {
		printk(KERN_ERR "failed to add cdev\n");
		goto fail_add_cdev;
	}
	class_register(&mod_class);
	if (!device_create(&mod_class, NULL, mod_dev, NULL, "flag")) {
		result = -EINVAL;
		printk(KERN_ERR "failed to create device\n");
		goto fail_create_device;
	}
	return 0;
fail_create_device:
	cdev_del(mod_cdev);
	class_unregister(&mod_class);
fail_add_cdev:
fail_alloc_cdev:
	unregister_chrdev_region(mod_dev, 1);
fail_alloc_region:
	return result;
}

static void __exit mod_exit(void) {
	printk(KERN_DEBUG "exiting module\n");
	device_destroy(&mod_class, mod_dev);
	class_unregister(&mod_class);
	cdev_del(mod_cdev);
	unregister_chrdev_region(mod_dev, 1);
}

module_init(mod_init);
module_exit(mod_exit);
