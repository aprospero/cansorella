#include "ctrl/scbi_glue.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/sockios.h>
#include <sys/time.h>
#include <errno.h>

#include "ctrl/scbi_api.h"
#include "ctrl/com/mqtt.h"
#include "ctrl/logger.h"

struct scbi_glue_handle
{
  int                  soc;
  struct mqtt_handle * broker;
  struct scbi_handle * scbi;
  struct timeval       start;
};

static const char * param_type_translate[] = {
    "sensor",    /* SCBI_PARAM_TYPE_SENSOR     */
    "relay",     /* SCBI_PARAM_TYPE_RELAY      */
    "overview"   /* SCBI_PARAM_TYPE_OVERVIEW   */
};

static enum log_level lltranslate[] = {
  LL_CRITICAL, /* SCBI_LL_CRITICAL, */
  LL_ERROR,    /* SCBI_LL_ERROR,    */
  LL_WARN,     /* SCBI_LL_WARNING,  */
  LL_INFO,     /* SCBI_LL_INFO,     */
  LL_DEBUG     /* SCBI_LL_DEBUG     */
};



void scbi_glue_log(enum scbi_log_level scbi_ll, const char * format, ...)
{
  if (log_get_level_state(lltranslate[scbi_ll]))
  {
    va_list ap;
    va_start(ap, format);
    log_push_v(lltranslate[scbi_ll], format, ap);
    va_end(ap);
  }
}


struct scbi_glue_handle * scbi_glue_create (struct scbi_handle * scbi_hnd, const char *port, void * broker)
{
  struct ifreq ifr;
  struct sockaddr_can addr;
  struct scbi_glue_handle * hnd = calloc (1, sizeof(struct scbi_glue_handle));

  gettimeofday(&hnd->start, NULL);

  LG_INFO("Initializing Sorel CAN Msg parser.");

  if (hnd == NULL)
  {
    LG_CRITICAL("Could not allocate ressources for Sorel CAN Msg parser.");
    return NULL;
  }

  hnd->soc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (hnd->soc < 0)
  {
    LG_CRITICAL("Could not open CAN interface. Error: %s", strerror(errno));
    scbi_glue_destroy(hnd);
    return NULL;
  }

  addr.can_family = AF_CAN;
  strcpy (ifr.ifr_name, port);
  if (ioctl (hnd->soc, SIOCGIFINDEX, &ifr) < 0)
  {
    LG_CRITICAL("Could not address CAN interface. Error: %s", strerror(errno));
    scbi_glue_destroy(hnd);
    return NULL;
  }
  addr.can_ifindex = ifr.ifr_ifindex;
  fcntl (hnd->soc, F_SETFL, O_NONBLOCK);
  if (bind (hnd->soc, (struct sockaddr*) &addr, sizeof(addr)) < 0)
  {
    LG_CRITICAL("Could not bind to CAN interface. Error: %s", strerror(errno));
    scbi_glue_destroy(hnd);
    return NULL;
  }
  hnd->broker = broker;
  hnd->scbi = scbi_hnd;
  return hnd;
}


void scbi_glue_update (struct scbi_glue_handle * hnd)
{
  struct scbi_frame       frame;
  struct timeval          tstamp;
  struct scbi_param *     param;
  int                     rx;
  struct timeval timeout = { 1, 0 };
  fd_set readSet;

  FD_ZERO(&readSet);
  FD_SET(hnd->soc, &readSet);
  if (select ((hnd->soc + 1), &readSet, NULL, NULL, &timeout) >= 0)
  {
    if (FD_ISSET(hnd->soc, &readSet))
    {
      rx = read (hnd->soc, &frame.msg, sizeof(frame.msg));
      if (rx < 0)
      {
        LG_ERROR("Reading CAN Bus: Posix Error (%i) '%s'.\n", errno, strerror(errno));
        return;
      }

      if ((unsigned int) rx < sizeof(struct can_frame))
      {
        scbi_print_frame (hnd->scbi, SCBI_LL_ERROR, "FRAME", "too short", &frame);
        return;
      }

      /* get the timestamp for the received message */
      ioctl(hnd->soc, SIOCGSTAMP, &tstamp);
      timersub(&tstamp, &hnd->start, &tstamp);
      frame.recvd = (tstamp.tv_sec * 1000) + (tstamp.tv_usec / 1000);

      if (scbi_parse(hnd->scbi, &frame) == 0)
      {
        while ((param = scbi_pop_param(hnd->scbi)) != NULL)
        {
          if (param->type < SCBI_PARAM_TYPE_COUNT)
            mqtt_publish(hnd->broker, param_type_translate[param->type], param->name, param->value);
        }
      }
      fflush (stdout);
      fflush (stderr);
    }
    if (hnd->broker != NULL)
      mqtt_loop(hnd->broker, 0);
  }
}

void scbi_glue_destroy(struct scbi_glue_handle * hnd)
{
  if (hnd)
  {
    if (hnd->soc)
      close(hnd->soc);
    free(hnd);
  }
}
