#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <asm/bitops.h>
#include <linux/types.h>
#include <linux/sem.h>
#include <linux/mutex.h>
#include <linux/atomic.h>

#include "ioctl.h"
#include "sha256.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Torin Carey <tcarey1@sheffield.ac.uk>");

#define STATE_UNINIT 0
#define STATE_READY  1

static atomic_t dev_state = ATOMIC_INIT(STATE_UNINIT);
static DEFINE_MUTEX(state_lock);
static struct flag_key secrets;

#define FLAG_FLAG2 1
#define FLAG_FLAG3 2

struct file_state {
	volatile unsigned long auth;
};

static const char mymessage[] = "Nothing to see here...\n";

static int mod_open(struct inode *inode, struct file *file)
{
	struct file_state *state;
	
	state = kmalloc(sizeof(struct file_state), GFP_KERNEL);
	if (!state) {
		printk(KERN_ERR "ctfmod: open: failed to allocate memory\n");
		return -ENOMEM;
	}
	state->auth = 0;
	file->private_data = (void *)state;
	return 0;
}

static int mod_release(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	return 0;
}

static ssize_t mod_read(struct file *file, char __user *data, size_t size, loff_t *offset)
{
	unsigned int avail;
	unsigned long f;
	if (*offset >= sizeof(mymessage) || *offset < 0)
		return 0;
	avail = sizeof(mymessage) - *offset;
	avail = avail > size ? size : avail;
	f = copy_to_user(data, mymessage, avail);
	avail -= f;
	*offset += avail;
	return avail;
}

static ssize_t mod_write(struct file *file, const char __user *data, size_t size, loff_t *offset)
{
	return -EPERM;
}

static int authenticate(const struct auth_data *auth)
{
	struct sha256_state state;
	unsigned char digest[32];
	int ret;
	if (auth->message_len > sizeof(auth->message))
		return -EINVAL;
	sha256_init(&state);
	sha256_update(&state, secrets.key, sizeof(secrets.key));
	sha256_update(&state, auth->message, auth->message_len);
	sha256_final(&state, digest);
	ret = memcmp(digest, auth->digest, 32);
	return ret ? -EPERM : 0;
}

static long mod_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long ret;
	struct auth_data auth;
	struct file_state *state;
	void *sptr, *mptr, *eptr;
	kuid_t kuid;
	state = (struct file_state *)file->private_data;

	switch (cmd) {
	case IOCTL_GET_FLAG1:
		if (atomic_read(&dev_state) != STATE_READY) {
			ret = -EBUSY;
			goto finish;
		}
		ret = copy_to_user((char *)arg, secrets.flag[0], FLAG_LEN);
		ret = FLAG_LEN - ret;
		goto finish;
	case IOCTL_GET_FLAG2:
		if (atomic_read(&dev_state) != STATE_READY) {
			ret = -EBUSY;
			goto finish;
		}
		if (!test_bit(FLAG_FLAG2, &state->auth)) {
			ret = -EPERM;
			goto finish;
		}
		ret = copy_to_user((char *)arg, secrets.flag[1], FLAG_LEN);
		ret = FLAG_LEN - ret;
		goto finish;
	case IOCTL_GET_FLAG3:
		if (atomic_read(&dev_state) != STATE_READY) {
			ret = -EIO;
			goto finish;
		}
		if (!test_bit(FLAG_FLAG3, &state->auth)) {
			ret = -EPERM;
			goto finish;
		}
		ret = copy_to_user((char *)arg, secrets.flag[2], FLAG_LEN);
		ret = FLAG_LEN - ret;
		goto finish;
	case IOCTL_AUTHENTICATE:
		if (atomic_read(&dev_state) != STATE_READY) {
			ret = -EBUSY;
			goto finish;
		}
		if (copy_from_user(&auth, (void *)arg, sizeof(struct auth_data))) {
			printk(KERN_INFO "ctfmod: failed to authenticate\n");
			ret = -EINVAL;
			goto finish;
		}
		ret = authenticate(&auth);
		if (ret)
			goto finish;

		printk(KERN_DEBUG "ctfmod: data has correct mac\n");
		sptr = &auth.message;
		eptr = &auth.message[auth.message_len];
		while (sptr < eptr) {
			mptr = memchr(sptr, '\0', eptr - sptr);
			if (!mptr)
				mptr = eptr;

			if (mptr - sptr == 12) {
				if (!memcmp(sptr, "UNLOCK_FLAG2", 12)) {
					printk(KERN_INFO "ctfmod: flag2 unlocked\n");
					set_bit(FLAG_FLAG2, &state->auth);
				} else if (!memcmp(sptr, "UNLOCK_FLAG3", 12)) {
					printk(KERN_INFO "ctfmod: flag3 unlocked\n");
					set_bit(FLAG_FLAG3, &state->auth);
				}
			}
			sptr = mptr + 1;
		}

		ret = 0;
		goto finish;
	case IOCTL_LOAD_SECRETS:
		printk(KERN_DEBUG "ctfmod: attempting LOAD_SECRETS\n");
		kuid = make_kuid(current_user_ns(), 0);
		if (!uid_eq(current_cred()->uid, kuid) && !capable(CAP_SYS_ADMIN)) {
			printk(KERN_DEBUG "ctfmod: LOAD_SECRETS failed: permission denied\n");
			ret = -EPERM;
			goto finish;
		}
		if (mutex_lock_interruptible(&state_lock)) {
			printk(KERN_DEBUG "ctfmod: LOAD_SECRETS failed: could not obtain lock\n");
			ret = -ERESTARTSYS;
			goto finish;
		}
		if (atomic_read(&dev_state) == STATE_READY) {
			printk(KERN_DEBUG "ctfmod: LOAD_SECRETS failed: already initialised\n");
			ret = -EIO;
			goto rel_state_lock;
		}
		if (copy_from_user(&secrets, (void *)arg, sizeof(struct flag_key))) {
			printk(KERN_DEBUG "ctfmod: LOAD_SECRETS failed: could not obtain struct\n");
			ret = -EINVAL;
			goto rel_state_lock;;
		}
		printk(KERN_DEBUG "ctfmod: loaded flag1: %*pE\n", FLAG_LEN, secrets.flag[0]);
		printk(KERN_DEBUG "ctfmod: loaded flag2: %*pE\n", FLAG_LEN, secrets.flag[1]);
		printk(KERN_DEBUG "ctfmod: loaded flag3: %*pE\n", FLAG_LEN, secrets.flag[2]);
		printk(KERN_DEBUG "ctfmod: loaded key: %*ph\n", 16, secrets.key);

		atomic_set(&dev_state, STATE_READY);
		ret = 0;
		printk(KERN_DEBUG "ctfmod: LOAD_SECRETS succeeded\n");
		goto rel_state_lock;;
	case IOCTL_CHECK_STATUS:
		ret = atomic_read(&dev_state) == STATE_READY ? 0 : -EBUSY;
		goto finish;
	default:
		ret = -ENOTTY;
	}
rel_state_lock:
	mutex_unlock(&state_lock);
finish:
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

static int mod_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	add_uevent_var(env, "DEVMODE=%#o", 0444);
	return 0;
}

static dev_t mod_dev = 0;
static struct class mod_class = {
	.owner = THIS_MODULE,
	.name = "ctf",
	.dev_uevent = mod_uevent,
};
static struct cdev *mod_cdev;

static int __init mod_init(void)
{
	int result;
	result = alloc_chrdev_region(&mod_dev, 0, 1, "ctfmod");
	if (result < 0) {
		printk(KERN_ERR "ctfmod: initialisation failed: failed to alloc chrdev region\n");
		goto fail_alloc_region;
	}
	mod_cdev = cdev_alloc();
	if (!mod_cdev) {
		result = -ENOMEM;
		printk(KERN_ERR "ctfmod: initialisation failed: failed to alloc cdev\n");
		goto fail_alloc_cdev;
	}
	cdev_init(mod_cdev, &mod_fops);
	result = cdev_add(mod_cdev, mod_dev, 1);
	if (result < 0) {
		printk(KERN_ERR "ctfmod: initialisation failed: failed to add cdev\n");
		goto fail_add_cdev;
	}
	class_register(&mod_class);
	if (!device_create(&mod_class, NULL, mod_dev, NULL, "flag")) {
		result = -EINVAL;
		printk(KERN_ERR "ctfmod: initialisation failed: failed to create device\n");
		goto fail_create_device;
	}
	printk(KERN_DEBUG "ctfmod: module loaded\n");
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

static void __exit mod_exit(void)
{
	device_destroy(&mod_class, mod_dev);
	class_unregister(&mod_class);
	cdev_del(mod_cdev);
	unregister_chrdev_region(mod_dev, 1);
	printk(KERN_DEBUG "ctfmod: module unloaded\n");
}

module_init(mod_init);
module_exit(mod_exit);
