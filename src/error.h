#ifndef ERROR_H
#define ERROR_H

#define chkerrf(expr, fail) { int err = (expr); if (err) { fail; return err; } }
#define chkerr(expr) { int err = (expr); if (err) return err; }

#endif
