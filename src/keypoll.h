#ifndef KEYPOLL_H
#define KEYPOLL_H

struct kp_ctx;

struct kp_ctx *kp_new(void);
void kp_del(struct kp_ctx *);
void kp_update(struct kp_ctx *);

#endif
