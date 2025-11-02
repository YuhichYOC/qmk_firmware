#include QMK_KEYBOARD_H

enum layer_number {
  _QWERTY = 0,
  _LOWER,
  _RAISE,
  _ADJUST,
  _LOWER_ADJUST,
  _RAISE_ADJUST,
};

#define MO_L MO(_LOWER)
#define MO_R MO(_RAISE)
#define TG_L TG(_LOWER)
#define TG_R TG(_RAISE)
#define MO_LA MO(_LOWER_ADJUST)
#define MO_RA MO(_RAISE_ADJUST)
#define LSFT_SPC LSFT_T(KC_SPC)
#define RSTF_ENT RSFT_T(KC_ENT)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_QWERTY] = LAYOUT(
         KC_ESC,            KC_1,            KC_2,            KC_3,            KC_4,            KC_5,                                              KC_6,            KC_7,            KC_8,            KC_9,            KC_0,         KC_MINS,
        KC_LALT,            KC_Q,            KC_W,            KC_E,            KC_R,            KC_T,                                              KC_Y,            KC_U,            KC_I,            KC_O,            KC_P,         KC_LBRC,
         KC_TAB,            KC_A,            KC_S,            KC_D,            KC_F,            KC_G,                                              KC_H,            KC_J,            KC_K,            KC_L,         KC_SCLN,         KC_QUOT,
        KC_LCTL,            KC_Z,            KC_X,            KC_C,            KC_V,            KC_B,         KC_BSPC,          KC_DEL,            KC_N,            KC_M,         KC_COMM,          KC_DOT,         KC_SLSH,         KC_INT1,
                                                           KC_LGUI,            TG_R,            MO_R,        LSFT_SPC,        RSTF_ENT,            MO_L,            TG_L,          KC_APP
),
[_LOWER] = LAYOUT(
        _______,         _______,         _______,         _______,         _______,         _______,                                           _______,         _______,         _______,         _______,         _______,         _______,
         KC_APP,         S(KC_1),         S(KC_2),         S(KC_3),         S(KC_4),         S(KC_5),                                           S(KC_6),         S(KC_7),         S(KC_8),         S(KC_9),         XXXXXXX,         XXXXXXX,
        KC_LGUI,         XXXXXXX,         KC_HOME,         KC_PGUP,         KC_PGDN,          KC_END,                                           KC_LEFT,         KC_DOWN,           KC_UP,         KC_RGHT,          KC_EQL,         KC_MINS,
        _______,         KC_LALT,         XXXXXXX,         XXXXXXX,         KC_INT5,         KC_BSPC,         _______,         _______,          KC_DEL,         KC_INT4,         KC_RBRC,         KC_BSLS,       S(KC_EQL),      S(KC_INT3),
                                                           _______,         _______,         _______,         _______,         _______,         _______,         _______,         _______
),
[_RAISE] = LAYOUT(
          KC_F1,           KC_F2,           KC_F3,           KC_F4,           KC_F5,           KC_F6,                                             KC_F7,           KC_F8,           KC_F9,          KC_F10,          KC_F11,          KC_F12,
        XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,                                           XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,
         KC_GRV,            KC_1,            KC_2,            KC_3,            KC_4,            KC_5,                                              KC_6,            KC_7,            KC_8,            KC_9,            KC_0,         _______,
        _______,         _______,         MS_WHLL,         MS_WHLR,         _______,         _______,         _______,         _______,         _______,         _______,         KC_COMM,          KC_DOT,         KC_SLSH,         KC_INT1,
                                                           _______,         _______,         _______,         _______,         _______,         _______,         _______,         _______
),
[_ADJUST] = LAYOUT(
         KC_ESC,            KC_1,            KC_2,            KC_3,            KC_4,            KC_5,                                              KC_6,            KC_7,            KC_8,            KC_9,            KC_0,         KC_MINS,
         KC_TAB,            KC_Q,            KC_W,            KC_E,            KC_R,            KC_T,                                              KC_Y,            KC_U,            KC_I,            KC_O,            KC_P,         KC_LBRC,
        KC_LCTL,            KC_A,            KC_S,            KC_D,            KC_F,            KC_G,                                              KC_H,            KC_J,            KC_K,            KC_L,         KC_SCLN,         KC_QUOT,
        KC_LSFT,            KC_Z,            KC_X,            KC_C,            KC_V,            KC_B,         _______,         _______,            KC_N,            KC_M,         _______,         _______,         _______,         _______,
                                                           _______,            TG_R,          KC_SPC,           MO_RA,           MO_LA,          KC_ENT,            TG_L,         _______
),
[_LOWER_ADJUST] = LAYOUT(
        _______,         _______,         _______,         _______,         _______,         _______,                                           _______,         _______,         _______,         _______,         _______,         _______,
        XXXXXXX,         S(KC_1),         S(KC_2),         S(KC_3),         S(KC_4),         S(KC_5),                                           S(KC_6),         S(KC_7),         S(KC_8),         S(KC_9),         XXXXXXX,         XXXXXXX,
        XXXXXXX,         XXXXXXX,         KC_HOME,         KC_PGUP,         KC_PGDN,          KC_END,                                           KC_LEFT,         KC_DOWN,           KC_UP,         KC_RGHT,          KC_EQL,         KC_MINS,
        KC_LCTL,         KC_LALT,         XXXXXXX,         XXXXXXX,         KC_INT5,         KC_BSPC,         _______,         _______,          KC_DEL,         KC_INT4,         KC_RBRC,         KC_BSLS,       S(KC_EQL),      S(KC_INT3),
                                                           _______,         _______,         _______,         _______,         _______,         _______,         _______,         _______
),
[_RAISE_ADJUST] = LAYOUT(
          KC_F1,           KC_F2,           KC_F3,           KC_F4,           KC_F5,           KC_F6,                                             KC_F7,           KC_F8,           KC_F9,          KC_F10,          KC_F11,          KC_F12,
        XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,                                           XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,         XXXXXXX,
        XXXXXXX,            KC_1,            KC_2,            KC_3,            KC_4,            KC_5,                                              KC_6,            KC_7,            KC_8,            KC_9,            KC_0,         _______,
        _______,         _______,         KC_VOLD,         KC_VOLU,         _______,         _______,         _______,         _______,         _______,         _______,         KC_COMM,          KC_DOT,         KC_SLSH,         KC_INT1,
                                                           _______,         _______,         _______,         _______,         _______,         _______,         _______,         _______
)
};

layer_state_t layer_state_set_user(layer_state_t state) {
  return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
}

#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
  if (!is_keyboard_master())
    return OLED_ROTATION_180;
  return rotation;
}

const char *read_layer_state(void);
const char *read_logo(void);
void set_keylog(uint16_t keycode, keyrecord_t *record);
const char *read_keylog(void);
const char *read_keylogs(void);

bool oled_task_user(void) {
  if (is_keyboard_master()) {
    oled_write_ln(read_layer_state(), false);
    oled_write_ln(read_keylog(), false);
    oled_write_ln(read_keylogs(), false);
  } else {
    oled_write(read_logo(), false);
  }
  return false;
}

#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
#ifdef OLED_ENABLE
    set_keylog(keycode, record);
#endif
  }
  return true;
}
