/*
Copyright 2019 @foostan
Copyright 2020 Drashna Jaelre <@drashna>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ビルド時のコマンド : qmk compile -kb crkbd -km YuhichYOC

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
[_QWERTY] = LAYOUT_split_3x6_3(
         KC_ESC,            KC_Q,            KC_W,            KC_E,            KC_R,            KC_T,                                              KC_Y,            KC_U,            KC_I,            KC_O,            KC_P,         KC_LBRC,
         KC_TAB,            KC_A,            KC_S,            KC_D,            KC_F,            KC_G,                                              KC_H,            KC_J,            KC_K,            KC_L,         KC_SCLN,         KC_QUOT,
        KC_LCTL,            KC_Z,            KC_X,            KC_C,            KC_V,            KC_B,                                              KC_N,            KC_M,         KC_COMM,          KC_DOT,         KC_SLSH,         KC_INT1,
                                                                               TG_R,            MO_R,        LSFT_SPC,        RSTF_ENT,            MO_L,            TG_L
),
[_LOWER] = LAYOUT_split_3x6_3(
         KC_APP,         S(KC_1),         S(KC_2),         S(KC_3),         S(KC_4),         S(KC_5),                                           S(KC_6),         S(KC_7),         S(KC_8),         S(KC_9),         XXXXXXX,      S(KC_LBRC),
        KC_LGUI,         XXXXXXX,         KC_HOME,         KC_PGUP,         KC_PGDN,          KC_END,                                           KC_LEFT,         KC_DOWN,           KC_UP,         KC_RGHT,          KC_EQL,         KC_MINS,
        _______,         KC_LALT,         XXXXXXX,         XXXXXXX,         KC_INT5,         KC_BSPC,                                            KC_DEL,         KC_INT4,         KC_RBRC,         KC_BSLS,       S(KC_EQL),      S(KC_INT3),
                                                                            _______,         _______,         _______,         _______,         _______,         _______
),
[_RAISE] = LAYOUT_split_3x6_3(
          KC_F1,           KC_F2,           KC_F3,           KC_F4,           KC_F5,           KC_F6,                                             KC_F7,           KC_F8,           KC_F9,          KC_F10,          KC_F11,          KC_F12,
         KC_GRV,            KC_1,            KC_2,            KC_3,            KC_4,            KC_5,                                              KC_6,            KC_7,            KC_8,            KC_9,            KC_0,         _______,
        _______,         _______,         MS_WHLL,         MS_WHLR,         _______,         _______,                                           _______,         _______,         KC_COMM,          KC_DOT,         KC_SLSH,         KC_INT1,
                                                                            _______,         _______,         _______,         _______,         _______,         _______
),
[_ADJUST] = LAYOUT_split_3x6_3(
         KC_ESC,            KC_Q,            KC_W,            KC_E,            KC_R,            KC_T,                                              KC_Y,            KC_U,            KC_I,            KC_O,            KC_P,         KC_LBRC,
         KC_TAB,            KC_A,            KC_S,            KC_D,            KC_F,            KC_G,                                              KC_H,            KC_J,            KC_K,            KC_L,         KC_SCLN,         KC_QUOT,
        _______,            KC_Z,            KC_X,            KC_C,            KC_V,            KC_B,                                              KC_N,            KC_M,         _______,         _______,         _______,         _______,
                                                                               TG_R,          KC_SPC,           MO_RA,           MO_LA,          KC_ENT,            TG_L
),
[_LOWER_ADJUST] = LAYOUT_split_3x6_3(
        XXXXXXX,         S(KC_1),         S(KC_2),         S(KC_3),         S(KC_4),         S(KC_5),                                           S(KC_6),         S(KC_7),         S(KC_8),         S(KC_9),         XXXXXXX,      S(KC_LBRC),
        XXXXXXX,         XXXXXXX,         KC_HOME,         KC_PGUP,         KC_PGDN,          KC_END,                                           KC_LEFT,         KC_DOWN,           KC_UP,         KC_RGHT,          KC_EQL,         KC_MINS,
        _______,         KC_LALT,         XXXXXXX,         XXXXXXX,         KC_INT5,         KC_BSPC,                                            KC_DEL,         KC_INT4,         KC_RBRC,         KC_BSLS,       S(KC_EQL),      S(KC_INT3),
                                                                            _______,         _______,         _______,         _______,         _______,         _______
),
[_RAISE_ADJUST] = LAYOUT_split_3x6_3(
          KC_F1,           KC_F2,           KC_F3,           KC_F4,           KC_F5,           KC_F6,                                             KC_F7,           KC_F8,           KC_F9,          KC_F10,          KC_F11,          KC_F12,
        XXXXXXX,            KC_1,            KC_2,            KC_3,            KC_4,            KC_5,                                              KC_6,            KC_7,            KC_8,            KC_9,            KC_0,         _______,
        _______,         _______,         KC_VOLD,         KC_VOLU,         _______,         _______,                                           _______,         _______,         KC_COMM,          KC_DOT,         KC_SLSH,         KC_INT1,
                                                                            _______,         _______,         _______,         _______,         _______,         _______
)
};

#ifdef RGBLIGHT_ENABLE

const rgblight_segment_t PROGMEM led_layer_qwerty[] = RGBLIGHT_LAYER_SEGMENTS({0, 54, HSV_OFF});
const rgblight_segment_t PROGMEM led_layer_lower[] = RGBLIGHT_LAYER_SEGMENTS({0, 54, HSV_CHARTREUSE});
const rgblight_segment_t PROGMEM led_layer_raise[] = RGBLIGHT_LAYER_SEGMENTS({0, 54, HSV_TURQUOISE});
const rgblight_segment_t PROGMEM led_layer_adjust[] = RGBLIGHT_LAYER_SEGMENTS({0, 54, HSV_CORAL});
const rgblight_segment_t PROGMEM led_layer_lower_adjust[] = RGBLIGHT_LAYER_SEGMENTS({0, 54, HSV_OFF});
const rgblight_segment_t PROGMEM led_layer_raise_adjust[] = RGBLIGHT_LAYER_SEGMENTS({0, 54, HSV_OFF});
const rgblight_segment_t * const PROGMEM led_layers[] = RGBLIGHT_LAYERS_LIST(led_layer_qwerty, led_layer_lower, led_layer_raise, led_layer_adjust, led_layer_lower_adjust, led_layer_raise_adjust);

void keyboard_post_init_user() {
  rgblight_layers = led_layers;
}

bool led_update_user(led_t led_state) { return true; }

layer_state_t default_layer_state_set_user(layer_state_t state) {
  rgblight_set_layer_state(_QWERTY, layer_state_cmp(state, _QWERTY));
  return state;
}

layer_state_t layer_state_set_user(layer_state_t state) {
  state = update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
  rgblight_set_layer_state(_QWERTY, layer_state_cmp(state, _QWERTY));
  rgblight_set_layer_state(_LOWER, layer_state_cmp(state, _LOWER));
  rgblight_set_layer_state(_RAISE, layer_state_cmp(state, _RAISE));
  rgblight_set_layer_state(_ADJUST, layer_state_cmp(state, _ADJUST));
  rgblight_set_layer_state(_LOWER_ADJUST, layer_state_cmp(state, _LOWER_ADJUST));
  rgblight_set_layer_state(_RAISE_ADJUST, layer_state_cmp(state, _RAISE_ADJUST));
  return state;
}

#else

layer_state_t layer_state_set_user(layer_state_t state) {
  return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
}

#endif
