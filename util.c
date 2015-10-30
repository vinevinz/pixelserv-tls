#include "util.h"

// make gcc happy
#ifdef DEBUG
void dummy() {
  SET_LINE_NUMBER(__LINE__)
}
#endif

// stats data
// note that child processes inherit a snapshot copy
// public data (should probably change to a struct)
volatile sig_atomic_t count = 0;
volatile sig_atomic_t avg = 0;
volatile sig_atomic_t act = 0;
volatile sig_atomic_t rmx = 0;
volatile sig_atomic_t tct = 0;
volatile sig_atomic_t tav = 0;
volatile sig_atomic_t tmx = 0;
volatile sig_atomic_t err = 0;
volatile sig_atomic_t tmo = 0;
volatile sig_atomic_t cls = 0;
volatile sig_atomic_t nou = 0;
volatile sig_atomic_t pth = 0;
volatile sig_atomic_t nfe = 0;
volatile sig_atomic_t ufe = 0;
volatile sig_atomic_t gif = 0;
volatile sig_atomic_t bad = 0;
volatile sig_atomic_t txt = 0;
volatile sig_atomic_t jpg = 0;
volatile sig_atomic_t png = 0;
volatile sig_atomic_t swf = 0;
volatile sig_atomic_t ico = 0;
volatile sig_atomic_t sta = 0;
volatile sig_atomic_t stt = 0;
volatile sig_atomic_t noc = 0;
volatile sig_atomic_t rdr = 0;
volatile sig_atomic_t pst = 0;
volatile sig_atomic_t hed = 0;

volatile sig_atomic_t slh = 0;
volatile sig_atomic_t slm = 0;
volatile sig_atomic_t sle = 0;
volatile sig_atomic_t slu = 0;

// private data
static struct timespec startup_time = {0, 0};
static clockid_t clock_source = CLOCK_MONOTONIC;

void get_time(struct timespec *time) {
  if (clock_gettime(clock_source, time) < 0) {
    if (errno == EINVAL &&
        clock_source == CLOCK_MONOTONIC) {
      clock_source = CLOCK_REALTIME;
      syslog(LOG_WARNING, "clock_gettime() reports CLOCK_MONOTONIC not supported; switching to less accurate CLOCK_REALTIME");
      get_time(time); // try again with new clock setting
    } else {
      // this should never happen
      syslog(LOG_ERR, "clock_gettime() reported failure getting time: %m");
      time->tv_sec = time->tv_nsec = 0;
    }
  }
}

char* get_version(int argc, char* argv[]) {
  char* retbuf = NULL;
  char* optbuf = NULL;
  unsigned int optlen = 0, i = 1, freeoptbuf = 0;
  unsigned int arglen[argc];

  // capture startup_time if not yet set
  if (!startup_time.tv_sec) {
    get_time(&startup_time);
  }

  // determine total size of all arguments
  for (i = 1; i < argc; ++i) {
    arglen[i] = strlen(argv[i]) + 1; // add 1 for leading space
    optlen += arglen[i];
  }
  if (optlen > 0) {
    // allocate a buffer to hold all arguments
    optbuf = malloc((optlen * sizeof(char)) + 1);
    if (optbuf) {
      freeoptbuf = 1;
      // concatenate arguments into buffer
      for (i = 1, optlen = 0; i < argc; ++i) {
        optbuf[optlen] = ' '; // prepend a space to each argument
        strncpy(optbuf + optlen + 1, argv[i], arglen[i]);
        optlen += arglen[i];
      }
      optbuf[optlen] = '\0';
    } else {
      optbuf = " <malloc error>";
    }
  } else {
    optbuf = " <none>";
  }

  if (asprintf(&retbuf, "%s version: %s compiled: %s options:%s", argv[0], VERSION, __DATE__ " " __TIME__, optbuf) < 1) {
    retbuf = " <asprintf error>";
  }

  if (freeoptbuf) {
    free(optbuf);
    freeoptbuf = 0;
  }

  return retbuf;
}

char* get_stats(const int sta_offset, const int stt_offset) {
    char* retbuf = NULL;
    struct timespec current_time;
    double uptime;

    const char* sta_fmt = "<table><tr><td>uts</td><td>%.0f</td></tr><tr><td>req</td><td>%d</td></tr><tr><td>avg</td><td>%d</td></tr><tr><td>rmx</td><td>%d</td></tr><tr><td>tav</td><td>%d</td></tr><tr><td>tmx</td><td>%d</td></tr><tr><td>err</td><td>%d</td></tr><tr><td>tmo</td><td>%d</td></tr><tr><td>cls</td><td>%d</td></tr><tr><td>nou</td><td>%d</td></tr><tr><td>pth</td><td>%d</td></tr><tr><td>nfe</td><td>%d</td></tr><tr><td>ufe</td><td>%d</td></tr><tr><td>gif</td><td>%d</td></tr><tr><td>bad</td><td>%d</td></tr><tr><td>txt</td><td>%d</td></tr><tr><td>jpg</td><td>%d</td></tr><tr><td>png</td><td>%d</td></tr><tr><td>swf</td><td>%d</td></tr><tr><td>ico</td><td>%d</td></tr><tr><td>slh</td><td>%d</td></tr><tr><td>slm</td><td>%d</td></tr><tr><td>sle</td><td>%d</td></tr><tr><td>slu</td><td>%d</td></tr><tr><td>sta</td><td>%d</td></tr><tr><td>stt</td><td>%d</td></tr><tr><td>204</td><td>%d</td></tr><tr><td>rdr</td><td>%d</td></tr><tr><td>pst</td><td>%d</td></tr><tr><td>hed</td><td>%d</td></tr></table>";
    const char* stt_fmt = "%.0f uts, %d req, %d avg, %d rmx, %d tav, %d tmx, %d err, %d tmo, %d cls, %d nou, %d pth, %d nfe, %d ufe, %d gif, %d bad, %d txt, %d jpg, %d png, %d swf, %d ico, %d slh, %d slm, %d sle, %d slu, %d sta, %d stt, %d 204, %d rdr, %d pst, %d hed";
  
    get_time(&current_time);
    uptime = difftime(current_time.tv_sec, startup_time.tv_sec);

    if (asprintf(&retbuf, (stt_offset) ? stt_fmt : sta_fmt,
        uptime, count, avg, rmx, tav, tmx, err, tmo, cls, nou, pth, nfe, ufe, gif, bad, txt, jpg, png, swf, ico, slh, slm, sle, slu, sta + sta_offset, stt + stt_offset, noc, rdr, pst, hed
        ) < 1)
        retbuf = " <asprintf error>";

    return retbuf;
}
