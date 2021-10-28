/*
Copyright 2012,2013 Jun Wako <wakojun@gmail.com>

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

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "keyboard.h"
#include "keycode.h"
#include "action_code.h"
#include "action_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Disable macro and function features when LTO is enabled, since they break */
#ifdef LTO_ENABLE
#    ifndef NO_ACTION_MACRO
#        define NO_ACTION_MACRO
#    endif
#    ifndef NO_ACTION_FUNCTION
#        define NO_ACTION_FUNCTION
#    endif
#endif

#ifndef TAP_CODE_DELAY
#    define TAP_CODE_DELAY 0
#endif
#ifndef TAP_HOLD_CAPS_DELAY
#    define TAP_HOLD_CAPS_DELAY 80
#endif

/* tapping count and state */
typedef struct {
    bool    interrupted : 1;
    bool    reserved2 : 1;
    bool    reserved1 : 1;
    bool    reserved0 : 1;
    uint8_t count : 4;
} tap_t;

/* Key event container for recording */
typedef struct {
    keyevent_t event;
#ifndef NO_ACTION_TAPPING
    tap_t tap;
#endif
#ifdef COMBO_ENABLE
    uint16_t keycode;
#endif
} keyrecord_t;

/* Execute action per keyevent */
void action_exec(keyevent_t event);

/* action for key */
action_t action_for_key(uint8_t layer, keypos_t key);
action_t action_for_keycode(uint16_t keycode);

/* macro */
const macro_t *action_get_macro(keyrecord_t *record, uint8_t id, uint8_t opt);

/* user defined special function */
void action_function(keyrecord_t *record, uint8_t id, uint8_t opt);

/* keyboard-specific key event (pre)processing */
bool process_record_quantum(keyrecord_t *record);

/* Utilities for actions.  */
#if !defined(NO_ACTION_LAYER) && !defined(STRICT_LAYER_RELEASE)
extern bool disable_action_cache;
#endif

/* Code for handling one-handed key modifiers. */
#ifdef SWAP_HANDS_ENABLE
extern bool                   swap_hands;
extern const keypos_t PROGMEM hand_swap_config[MATRIX_ROWS][MATRIX_COLS];
#    if (MATRIX_COLS <= 8)
typedef uint8_t swap_state_row_t;
#    elif (MATRIX_COLS <= 16)
typedef uint16_t swap_state_row_t;
#    elif (MATRIX_COLS <= 32)
typedef uint32_t swap_state_row_t;
#    else
#        error "MATRIX_COLS: invalid value"
#    endif

void process_hand_swap(keyevent_t *record);
#endif

void process_record_nocache(keyrecord_t *record);
void process_record(keyrecord_t *record);
void process_record_handler(keyrecord_t *record);
void post_process_record_quantum(keyrecord_t *record);
void process_action(keyrecord_t *record, action_t action);
void register_code(uint8_t code);
void unregister_code(uint8_t code);
void tap_code(uint8_t code);
void tap_code_delay(uint8_t code, uint16_t delay);
void register_mods(uint8_t mods);
void unregister_mods(uint8_t mods);
void register_weak_mods(uint8_t mods);
void unregister_weak_mods(uint8_t mods);
// void set_mods(uint8_t mods);
void clear_keyboard(void);
void clear_keyboard_but_mods(void);
void clear_keyboard_but_mods_and_keys(void);
void layer_switch(uint8_t new_layer);
bool is_tap_key(keypos_t key);
bool is_tap_record(keyrecord_t *record);
bool is_tap_action(action_t action);

#ifndef NO_ACTION_TAPPING
void process_record_tap_hint(keyrecord_t *record);
#endif

typedef struct _action_processor {
    keyrecord_t * record;
    action_t * action;

#ifndef NO_ACTION_ONESHOT
    bool * do_release_oneshot;
#endif
#ifdef SWAP_HANDS_ENABLE
    bool * swap_hands;
#    ifndef NO_ACTION_TAPPING
    bool * swap_held;
#    endif
#endif
#ifdef RETRO_TAPPING
    int * retro_tapping_counter;
#elifdef RETRO_TAPPING_PER_KEY
    int * retro_tapping_counter;
#endif

    uint8_t mods;
#ifdef ONESHOT_TAP_TOGGLE
    bool oneshot_tap_toggle;
#endif

    void (* act_mods)(struct _action_processor *);
    void (* act_mods_tap_oneshot)(struct _action_processor *);
    void (* act_mods_tap_toggle)(struct _action_processor *);
    void (* act_mods_tap_default)(struct _action_processor *);
    void (* act_usage_system)(struct _action_processor *);
    void (* act_usage_consumer)(struct _action_processor *);
    void (* act_mousekey)(struct _action_processor *);
    void (* act_layer_bitop_on)(struct _action_processor *);
    void (* act_layer_bitop_other)(struct _action_processor *);
    void (* act_layer_mods)(struct _action_processor *);
    void (* act_layer_tap_or_tap_ext_toggle)(struct _action_processor *);
    void (* act_layer_tap_or_tap_ext_on_off)(struct _action_processor *);
    void (* act_layer_tap_or_tap_ext_off_on)(struct _action_processor *);
    void (* act_layer_tap_or_tap_ext_set_clear)(struct _action_processor *);
    void (* act_layer_tap_or_tap_ext_oneshot)(struct _action_processor *);
    void (* act_layer_tap_or_tap_ext_default)(struct _action_processor *);
    void (* act_macro)(struct _action_processor *);
    void (* act_swap_hands_toggle)(struct _action_processor *);
    void (* act_swap_hands_on_off)(struct _action_processor *);
    void (* act_swap_hands_off_on)(struct _action_processor *);
    void (* act_swap_hands_on)(struct _action_processor *);
    void (* act_swap_hands_off)(struct _action_processor *);
    void (* act_swap_hands_oneshot)(struct _action_processor *);
    void (* act_swap_hands_tap_toggle)(struct _action_processor *);
    void (* act_swap_hands_default)(struct _action_processor *);
    void (* act_function)(struct _action_processor *);
    void (* layer_led)(struct _action_processor *);
    void (* retro_tapping)(struct _action_processor *);
    void (* oneshot_swaphands)(struct _action_processor *);
    void (* release_oneshot)(struct _action_processor *);

    void (* debug_print)(char *);
    void (* wait_ms)(int);

    void (* run)(struct _action_processor *);
} action_processor;

action_processor action_processor_init(keyrecord_t * record, action_t * action);

/* debug */
void debug_event(keyevent_t event);
void debug_record(keyrecord_t record);
void debug_action(action_t action);

#ifdef __cplusplus
}
#endif
