#include "keypoll.h"
#include <dirent.h>             /* opendir, readdir */
#include <fcntl.h>              /* open */
#include <unistd.h>             /* close */
#include <libevdev-1.0/libevdev/libevdev.h>           /* libevdev_new_from_fd */
#include <stdlib.h>             /* free */
#include <string.h>             /* strlen, strcat */
#include <stdio.h>              /* puts */

enum {
  MAX_DEVICES = 32
};

struct kp_ctx {
  size_t n_devices;
  int fds[MAX_DEVICES];
  struct libevdev *devs[MAX_DEVICES];
};

int supported_device(char *name) {
  char *supported_devices[] = {
    "event-kbd",
    "event-mouse"
  };
  size_t i, n_devices, len;

  len = strlen(name);
  n_devices = sizeof(supported_devices) / sizeof(supported_devices[0]);
  for (i = 0; i < n_devices; ++i) {
    char *device;
    int match;

    device = supported_devices[i];
    match = !strcmp(name + (len - strlen(device)), device);
    if (match) return 1;
  }
  return 0;
}

int process_devices(struct kp_ctx *kp, size_t n_devices, char **devices) {
  enum {
    filepath_end = 19
  };

  char filepath[256] = "/dev/input/by-path/";
  int rc;
  size_t i;

  kp->n_devices = n_devices;
  for (i = 0; i < n_devices; ++i) {
    struct libevdev *dev = NULL;

    strcat(filepath, devices[i]);
    kp->fds[i] = open(filepath, O_RDONLY | O_NONBLOCK);
    /* we're going to reuse filepath for each device */
    filepath[filepath_end] = '\0';
    if (kp->fds[i] < 0) goto err;
    rc = libevdev_new_from_fd(kp->fds[i], &dev);
    if (rc < 0) goto err;
    kp->devs[i] = dev;
    printf("registered input: %s\n", libevdev_get_name(dev));
    continue;

  err:
    /* close all previous file descriptors and free libevdev objects */
    while (i--) {
      libevdev_free(kp->devs[i]);
      close(kp->fds[i]);
    }
    return -1;
  }
  return 0;
}

/* **************************************** */
/* Public */
/* **************************************** */

struct kp_ctx *kp_new(void) {
  char *devices[MAX_DEVICES] = { 0 };
  size_t n_devices = 0;
  DIR *input_dir = NULL;
  struct dirent *input = NULL;
  struct kp_ctx *kp;

  kp = malloc(sizeof(struct kp_ctx));
  if (!kp) return NULL;
  input_dir = opendir("/dev/input/by-path");
  if (!input_dir) goto err;
  while ((input = readdir(input_dir))) {
    if (!supported_device(input->d_name)) continue;
    if (n_devices >= MAX_DEVICES) break;
    devices[n_devices++] = input->d_name;
  }
  if (process_devices(kp, n_devices, devices)) goto err;
  closedir(input_dir);
  return kp;

 err:
  free(kp);
  return NULL;
}

void kp_del(struct kp_ctx *kp) {
  size_t i;

  if (!kp) return;
  for (i = 0; i < kp->n_devices; ++i) {
    libevdev_free(kp->devs[i]);
    close(kp->fds[i]);
  }
  free(kp);
}

void kp_update(struct kp_ctx *kp) {
  /* no null check */
  size_t i;

  for (i = 0; i < kp->n_devices; ++i) {
    int rc;
    struct input_event e;

    rc = libevdev_next_event(kp->devs[i], LIBEVDEV_READ_FLAG_NORMAL, &e);
    if (rc != LIBEVDEV_READ_STATUS_SUCCESS) continue;
    printf("event: %s\n", libevdev_event_type_get_name(e.type));
  }
}
