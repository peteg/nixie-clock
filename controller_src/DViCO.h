/*
 * IR remote control daemon.
 *
 * Peter Gammie, peteg42 at gmail dot com
 * Commenced March 2010.
 *
 * This provides ese are the magic numbers for the DViCO Ultraview remote.
 *
 */

#ifndef _DViCO_H_
#define _DViCO_H_

/* We only pay attention to HID events with this label: */
enum {
  significant_hid_event_id = 0x10046
};

/*
 * The keys.
 *
 *  - All have 0xF9 in the lower byte
 *  - The second-lowest byte is significant (listed below).
 *  - The top bit is set on key release.
 *
 */

#define control_event_normalise(e) (((e) >> 8) & ~0x80)
#define control_event_is_release(e) ((e) & 0x8000)

typedef enum {
  btn_chup = 0x00,
  btn_chdown = 0x01,
  btn_volup = 0x02,
  btn_voldown = 0x03,
  btn_zoom = 0x04,
  btn_red = 0x05,
  btn_atvdtv = 0x06,
  btn_aspect = 0x07,
  btn_menu = 0x08,
  btn_green = 0x09,
  btn_power = 0x0A,
  btn_epg = 0x0B,
  btn_pcoff = 0x0C,
  btn_mute = 0x0D,
  btn_back = 0x0E,
  btn_hd = 0x0F,
  btn_0 = 0x10,
  btn_1 = 0x11,
  btn_2 = 0x12,
  btn_3 = 0x13,
  btn_4 = 0x14,
  btn_5 = 0x15,
  btn_6 = 0x16,
  btn_7 = 0x17,
  btn_8 = 0x18,
  btn_9 = 0x19,
  btn_camera = 0x1A,
  btn_record = 0x1B,
  btn_rew = 0x1C,
  btn_ff = 0x1D,
  btn_stop = 0x1E,
  btn_playpause = 0x1F,
  btn_folder = 0x40,
  btn_dvhs = 0x41,
  btn_yellow = 0x42,
  btn_blue = 0x43
} DViCO_button_t;

#endif /* _DViCO_H_ */
