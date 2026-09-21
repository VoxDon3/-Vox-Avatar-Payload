/* vox.avatar — Vox Manager PS5 Profile Avatar Injector.
 *
 * A PS5 payload built with the prospero payload SDK. Staged profile/avatar
 * images are placed under /data/ps5upload/profile/<uid>/ (each user folder is
 * named after its account id in hex, e.g. 0x0000000100000002). When injected
 * the payload copies those images into
 * /system_data/priv/cache/profile/<uid>/ so the system uses them as the user
 * avatar, and shows a TV notification with the result.
 */

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct notify_request {
  char useless1[45];
  char message[3075];
} notify_request_t;

int sceKernelSendNotificationRequest(int, notify_request_t *, size_t, int);

static const char STAGE_DIR[] = "/data/ps5upload/profile";
static const char CACHE_DIR[] = "/system_data/priv/cache/profile";

static void
notify(const char *text) {
  notify_request_t req;

  bzero(&req, sizeof req);
  snprintf(req.message, sizeof req.message, "%s", text);
  sceKernelSendNotificationRequest(0, &req, sizeof req, 0);
}

static void
make_dirs(void) {
  mkdir("/system_data", 0755);
  mkdir("/system_data/priv", 0755);
  mkdir("/system_data/priv/cache", 0755);
  mkdir(CACHE_DIR, 0755);
}

static int
copy_file(const char *src, const char *dst) {
  char buf[0x10000];
  ssize_t nr, nw;
  int in, out, done = 0;

  in = open(src, O_RDONLY);
  if (in < 0) {
    return 0;
  }

  out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (out < 0) {
    close(in);
    return 0;
  }

  for (;;) {
    nr = read(in, buf, sizeof buf);
    if (nr <= 0) {
      if (nr == 0) {
        done = 1;
      }
      break;
    }
    nw = write(out, buf, (size_t)nr);
    if (nw != nr) {
      break;
    }
  }

  close(out);
  close(in);
  return done;
}

static int
stage_user(const char *dir, const char *uid, int *files) {
  char src_path[PATH_MAX + 1];
  char dst_path[PATH_MAX + 1];
  struct dirent *dp;
  struct stat st;
  DIR *d;

  if (snprintf(src_path, sizeof src_path, "%s/%s", dir, uid)
      >= (int)sizeof src_path) {
    return 0;
  }
  if (snprintf(dst_path, sizeof dst_path, "%s/%s", CACHE_DIR, uid)
      >= (int)sizeof dst_path) {
    return 0;
  }

  if (stat(src_path, &st)) {
    return 0;
  }
  if (!S_ISDIR(st.st_mode)) {
    return 0;
  }

  if (!(d = opendir(src_path))) {
    return 0;
  }

  mkdir(dst_path, 0755);

  *files = 0;
  while ((dp = readdir(d))) {
    char sf[PATH_MAX + 1];
    char df[PATH_MAX + 1];

    if (dp->d_name[0] == '.') {
      continue;
    }
    if (snprintf(sf, sizeof sf, "%s/%s", src_path, dp->d_name)
        >= (int)sizeof sf) {
      continue;
    }
    if (stat(sf, &st)) {
      continue;
    }
    if (!S_ISREG(st.st_mode)) {
      continue;
    }
    if (snprintf(df, sizeof df, "%s/%s", dst_path, dp->d_name)
        >= (int)sizeof df) {
      continue;
    }

    if (copy_file(sf, df)) {
      (*files)++;
    }
  }

  closedir(d);
  return 1;
}

int
main(void) {
  struct dirent *dp;
  DIR *d;
  int users = 0;
  int files = 0;

  notify("vox.avatar: start");

  if (!(d = opendir(STAGE_DIR))) {
    notify("vox.avatar: no stage folder");
    return 1;
  }

  while ((dp = readdir(d))) {
    int n;

    if (dp->d_name[0] != '0') {
      continue;
    }
    if (dp->d_name[1] != 'x' && dp->d_name[1] != 'X') {
      continue;
    }
    if (dp->d_name[2] == '\0') {
      continue;
    }

    make_dirs();
    if (stage_user(STAGE_DIR, dp->d_name, &n)) {
      users++;
      files += n;
    }
  }

  closedir(d);

  if (users > 0) {
    char msg[0xa0];

    snprintf(msg, sizeof msg, "vox.avatar OK: %d user(s), %d file(s)", users,
             files);
    notify(msg);
    return 0;
  }

  notify("vox.avatar: nothing staged");
  return 1;
}