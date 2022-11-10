#ifndef _CTRL_SCBI__H
#define _CTRL_SCBI__H

#include <stdint.h>

struct scbi_handle;

enum scbi_protocol
{
	CAN_FORMAT_0      = 0x00,   /* CAN Msgs size <= 8 */
	CAN_FORMAT_BULK   = 0x01,   /* CAN Msgs size >  8 */
	CAN_FORMAT_UPDATE = 0x02    /* CAN Msg transmitting firmware update */
};

enum scbi_msg_type
{
  MSG_REQUEST  = 0x00,
  MSG_RESERVE  = 0x01,
  MSG_RESPONSE = 0x02,
  MSG_ERROR    = 0x03
};


struct scbi_id_format
{
  uint8_t prog;
  uint8_t sender;
  uint8_t func;
  uint8_t prot:3;
  uint8_t msg:2;
  uint8_t err:1;
  uint8_t rtr:1;
  uint8_t fff:1;
} __attribute__((packed));


union scbi_address_id
{
  uint32_t               address_id;
  struct scbi_id_format  scbi_id;
};

/* CAN_FORMAT_0 protocol definitions */

enum scbi_prog_type
{
  PRG_CONTROLLER              = 0x0B,
  PRG_DATALOGGER_MONITOR      = 0x80,
  PRG_REMOTESENSOR            = 0x83,
  PRG_DATALOGGER_NAMEDSENSORS = 0x84, /* REMOTESENSOR  (for compatibility) */
  PRG_HCC                     = 0x85,
  PRG_AVAILABLERESOURCES      = 0x8C,
  PRG_PARAMETERSYNCCONFIG     = 0x90,
  PRG_ROOMSYNC                = 0x91,
  PRG_MSGLOG                  = 0x94,
  PRG_CBCS                    = 0x95
};

enum scbi_dlg_function_type
{
  DLF_UNDEFINED            = 0x00,
  DLF_SENSOR               = 0x01,
  DLF_RELAY                = 0x02,
  DLG_HYDRAULIC_PROGRAM    = 0x03,
  DLG_ERROR_MESSAGE        = 0x04,
  DLG_PARAM_MONITORING     = 0x05,
  DLG_STATISTIC            = 0x06,
  DLG_OVERVIEW             = 0x07,
  DLG_HYDRAULIC_CONFIG     = 0x08
};


// message definition for sensor data

struct scbi_dlg_sensor_msg
{
  uint8_t  id;
  uint16_t value;
  uint8_t  type;
  uint8_t  subtype;
} __attribute__((packed));

enum scbi_dlg_sensor_type
{
  DST_UNKNOWN          = 0x00,
  DST_FLOW             = 0x01,           // (VFS SENSOR)
  DST_RELPRESSURE      = 0x02,           // (RPS SENSOR)
  DST_DIFFPRESSURE     = 0x03,           // (DPS SENSOR)
  DST_TEMPERATURE      = 0x04,
  DST_HUMIDIDY         = 0x05,
  DST_ROOM_CTRL_WHEEL  = 0x06,
  DST_ROOM_CTRL_SWITCH = 0x07
};


// message definition for relay data

struct scbi_dlg_relay_msg
{
  uint8_t id;
  uint8_t mode;
  uint8_t is_on;
  uint8_t exfunc[2];
} __attribute__((packed));

enum scbi_dlg_relay_mode
{
  DRM_RELAYMODE_SWITCHED = 0x00,
  DRM_RELAYMODE_PHASE    = 0x01,
  DRM_RELAYMODE_PWM      = 0x02,
  DRM_RELAYMODE_VOLTAGE  = 0x03
};

enum scbi_dlg_relay_ext_func
{
  DRE_DISABLED            = 0xFE,
  DRE_UNSELECTED          = 0xFF,

  DRE_SOLARBYPASS         = 0x00,
  DRE_HEATING             = 0x01,
  DRE_HEATING2            = 0x02,
  DRE_COOLING             = 0x03,
  DRE_RET_FLOW_INCREASE   = 0x04,
  DRE_DISSIPATION         = 0x05,
  DRE_ANTILEGIO           = 0x06,
  DRE_REVERSE_LOADING     = 0x07,
  DRE_DIFFERENCE          = 0x08,
  DRE_WOOD_BOILER         = 0x09,

  DRE_SAFETY_FCT          = 0x10,
  DRE_PRESSURE_CTRL       = 0x11,
  DRE_BOOSTER             = 0x12,
  DRE_R1PARALLEL_OP       = 0x13,
  DRE_R2PARALLEL_OP       = 0x14,
  DRE_ALWAYS_ON           = 0x15,
  DRE_HEATING_CIRCUIT_RC21= 0x16,
  DRE_CIRCULATION         = 0x17,
  DRE_STORAGEHEATING      = 0x18,
  DRE_STORAGESTACKING     = 0x19,

  DRE_R_V1_PARALLEL       = 0x20,
  DRE_R_V2_PARALLEL       = 0x21,
  DRE_R1_PERMANENTLY_ON   = 0x22,
  DRE_R2_PERMANENTLY_ON   = 0x23,
  DRE_R3_PERMANENTLY_ON   = 0x24,

  DRE_V2_PERMANENTLY_ON   = 0x25,
  DRE_EXTERNALALHEATING   = 0x26,
  DRE_NEWLOGMESSAGE       = 0x27,

  DRE_EXTRAPUMP           = 0x28,
  DRE_PRIMARYMIXER_UP     = 0x29,
  DRE_PRIMARYMIXER_DOWN   = 0x30,
  DRE_SOLAR               = 0x31,
  DRE_CASCADE             = 0x32
};


// message definition for overview data

struct scbi_dlg_overview_msg
{
   uint8_t  id:5;
   uint8_t  type:3;
   uint16_t hours;
   uint32_t heat_yield;
} __attribute__((packed));

enum scbi_dlg_overview_type
{
  DOT_DAYS   = 0x01,
  DOT_WEEKS  = 0x02,
  DOT_MONTHS = 0x03,
  DOT_YEARS  = 0x04,
  DOT_TOTAL  = 0x05,
  DOT_STATUS = 0x06
};



union scbi_msg_content
{
    uint8_t                      raw[8];
    struct scbi_dlg_sensor_msg   sensor;
    struct scbi_dlg_relay_msg    relay;
    struct scbi_dlg_overview_msg overview;
};




struct scbi_handle * scbi_open(struct scbi_handle * hnd, const char * port);
void                 scbi_update(struct scbi_handle * hnd);
int                  scbi_close(struct scbi_handle * hnd);

#endif   // _CTRL_SCBI__H
