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
#include "host.h"
#include "keycode.h"
#include "keyboard.h"
#include "mousekey.h"
#include "command.h"
#include "led.h"
#include "action_layer.h"
#include "action_tapping.h"
#include "action_macro.h"
#include "action_util.h"
#include "action.h"
#include "wait.h"

#ifdef BACKLIGHT_ENABLE
#    include "backlight.h"
#endif

#ifdef DEBUG_ACTION
#    include "debug.h"
#else
#    include "nodebug.h"
#endif

#ifdef POINTING_DEVICE_ENABLE
#    include "pointing_device.h"
#endif

int tp_buttons;

#if defined(RETRO_TAPPING) || defined(RETRO_TAPPING_PER_KEY)
int retro_tapping_counter = 0;
#endif

#ifdef IGNORE_MOD_TAP_INTERRUPT_PER_KEY
__attribute__((weak)) bool get_ignore_mod_tap_interrupt(uint16_t keycode, keyrecord_t *record) { return false; }
#endif

#ifdef RETRO_TAPPING_PER_KEY
__attribute__((weak)) bool get_retro_tapping(uint16_t keycode, keyrecord_t *record) { return false; }
#endif

__attribute__((weak)) bool pre_process_record_quantum(keyrecord_t *record) { return true; }

/** \brief Called to execute an action.
 *
 * FIXME: Needs documentation.
 */
void action_exec(keyevent_t event) {
    if (!IS_NOEVENT(event)) {
        dprint("\n---- action_exec: start -----\n");
        dprint("EVENT: ");
        debug_event(event);
        dprintln();
#if defined(RETRO_TAPPING) || defined(RETRO_TAPPING_PER_KEY)
        retro_tapping_counter++;
#endif
    }

    if (event.pressed) {
        // clear the potential weak mods left by previously pressed keys
        clear_weak_mods();
    }

#ifdef SWAP_HANDS_ENABLE
    if (!IS_NOEVENT(event)) {
        process_hand_swap(&event);
    }
#endif

    keyrecord_t record = {.event = event};

#ifndef NO_ACTION_ONESHOT
#    if (defined(ONESHOT_TIMEOUT) && (ONESHOT_TIMEOUT > 0))
    if (has_oneshot_layer_timed_out()) {
        clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
    }
    if (has_oneshot_mods_timed_out()) {
        clear_oneshot_mods();
    }
#        ifdef SWAP_HANDS_ENABLE
    if (has_oneshot_swaphands_timed_out()) {
        clear_oneshot_swaphands();
    }
#        endif
#    endif
#endif

#ifndef NO_ACTION_TAPPING
    if (IS_NOEVENT(record.event) || pre_process_record_quantum(&record)) {
        action_tapping_process(record);
    }
#else
    if (IS_NOEVENT(record.event) || pre_process_record_quantum(&record)) {
        process_record(&record);
    }
    if (!IS_NOEVENT(record.event)) {
        dprint("processed: ");
        debug_record(record);
        dprintln();
    }
#endif
}

#ifdef SWAP_HANDS_ENABLE
bool swap_hands = false;
bool swap_held  = false;

/** \brief Process Hand Swap
 *
 * FIXME: Needs documentation.
 */
void process_hand_swap(keyevent_t *event) {
    static swap_state_row_t swap_state[MATRIX_ROWS];

    keypos_t         pos     = event->key;
    swap_state_row_t col_bit = (swap_state_row_t)1 << pos.col;
    bool             do_swap = event->pressed ? swap_hands : swap_state[pos.row] & (col_bit);

    if (do_swap) {
        event->key.row = pgm_read_byte(&hand_swap_config[pos.row][pos.col].row);
        event->key.col = pgm_read_byte(&hand_swap_config[pos.row][pos.col].col);
        swap_state[pos.row] |= col_bit;
    } else {
        swap_state[pos.row] &= ~(col_bit);
    }
}
#endif

#if !defined(NO_ACTION_LAYER) && !defined(STRICT_LAYER_RELEASE)
bool disable_action_cache = false;

void process_record_nocache(keyrecord_t *record) {
    disable_action_cache = true;
    process_record(record);
    disable_action_cache = false;
}
#else
void process_record_nocache(keyrecord_t *record) { process_record(record); }
#endif

__attribute__((weak)) bool process_record_quantum(keyrecord_t *record) { return true; }

__attribute__((weak)) void post_process_record_quantum(keyrecord_t *record) {}

#ifndef NO_ACTION_TAPPING
/** \brief Allows for handling tap-hold actions immediately instead of waiting for TAPPING_TERM or another keypress.
 *
 * FIXME: Needs documentation.
 */
void process_record_tap_hint(keyrecord_t *record) {
    action_t action = layer_switch_get_action(record->event.key);

    switch (action.kind.id) {
#    ifdef SWAP_HANDS_ENABLE
        case ACT_SWAP_HANDS:
            switch (action.swap.code) {
                case OP_SH_ONESHOT:
                    break;
                case OP_SH_TAP_TOGGLE:
                default:
                    swap_hands = !swap_hands;
                    swap_held  = true;
            }
            break;
#    endif
    }
}
#endif

/** \brief Take a key event (key press or key release) and processes it.
 *
 * FIXME: Needs documentation.
 */
void process_record(keyrecord_t *record) {
    if (IS_NOEVENT(record->event)) {
        return;
    }

    if (!process_record_quantum(record)) {
#ifndef NO_ACTION_ONESHOT
        if (is_oneshot_layer_active() && record->event.pressed) {
            clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
        }
#endif
        return;
    }

    process_record_handler(record);
    post_process_record_quantum(record);
}

void process_record_handler(keyrecord_t *record) {
#ifdef COMBO_ENABLE
    action_t action;
    if (record->keycode) {
        action = action_for_keycode(record->keycode);
    } else {
        action = store_or_get_action(record->event.pressed, record->event.key);
    }
#else
    action_t action = store_or_get_action(record->event.pressed, record->event.key);
#endif
    dprint("ACTION: ");
    debug_action(action);
#ifndef NO_ACTION_LAYER
    dprint(" layer_state: ");
    layer_debug();
    dprint(" default_layer_state: ");
    default_layer_debug();
#endif
    dprintln();

    process_action(record, action);
}

#if defined(PS2_MOUSE_ENABLE) || defined(POINTING_DEVICE_ENABLE)
void register_button(bool pressed, enum mouse_buttons button) {
#    ifdef PS2_MOUSE_ENABLE
    tp_buttons = pressed ? tp_buttons | button : tp_buttons & ~button;
#    endif
#    ifdef POINTING_DEVICE_ENABLE
    report_mouse_t currentReport = pointing_device_get_report();
    currentReport.buttons        = pressed ? currentReport.buttons | button : currentReport.buttons & ~button;
    pointing_device_set_report(currentReport);
#    endif
}
#endif

/* Key and Mods */

void ap_act_mods(action_processor * ap) {
    if (ap->action->kind.id != ACT_LMODS && ap->action->kind.id != ACT_RMODS) return;
    if (ap->record->event.pressed) {
        if (ap.mods) {
            if (IS_MOD(ap->action->key.code) || ap->action->key.code == KC_NO) {
                // e.g. LSFT(KC_LGUI): we don't want the LSFT to be weak as it would make it useless.
                // This also makes LSFT(KC_LGUI) behave exactly the same as LGUI(KC_LSFT).
                // Same applies for some keys like KC_MEH which are declared as MEH(KC_NO).
                add_mods(ap.mods);
            }
            else {
                add_weak_mods(ap.mods);
            }
            send_keyboard_report();
        }
        register_code(ap->action->key.code);
    }
    else {
        unregister_code(ap->action->key.code);
        if (ap.mods) {
            if (IS_MOD(ap->action->key.code) || ap->action->key.code == KC_NO) {
                del_mods(ap.mods);
            }
            else {
                del_weak_mods(ap.mods);
            }
            send_keyboard_report();
        }
    }
}

void ap_act_mods_tap_oneshot(action_processor * ap) {
#ifdef NO_ACTION_TAPPING
    return;
#elifdef NO_ACTION_ONESHOT
    return;
#elifdef ONESHOT_TAP_TOGGLE
    if (ap->action->kind.id != ACT_LMODS_TAP && ap->action->kind.id != ACT_RMODS_TAP) return;
    if (ap->action->layer_tap.code != MODS_ONESHOT) return;
    // Oneshot modifier
    if (ap->record->event.pressed) {
        if (ap->record->tap.count == 0) {
            ap->debug_print("MODS_TAP: Oneshot: 0\n");
            register_mods(ap.mods | get_oneshot_mods());
        }
        else if (ap->record->tap.count == 1) {
            ap->debug_print("MODS_TAP: Oneshot: start\n");
            set_oneshot_mods(ap.mods | get_oneshot_mods());
        }
        else if (ap.oneshot_tap_toggle && ap->record->tap.count == ONESHOT_TAP_TOGGLE) {
            ap->debug_print("MODS_TAP: Toggling oneshot");
            clear_oneshot_mods();
            set_oneshot_locked_mods(ap.mods);
            register_mods(ap.mods);
        }
        else {
            register_mods(ap.mods | get_oneshot_mods());
        }
    }
    else {
        if (ap->record->tap.count == 0) {
            clear_oneshot_mods();
            unregister_mods(ap.mods);
        }
        else if (ap->record->tap.count == 1) {
            // Retain Oneshot mods
            if (ap.oneshot_tap_toggle && (ap.mods & get_mods())) {
                clear_oneshot_locked_mods();
                clear_oneshot_mods();
                unregister_mods(ap.mods);
            }
        }
        else if (ap.oneshot_tap_toggle && ap->record->tap.count == ONESHOT_TAP_TOGGLE) {
            // Toggle Oneshot Layer
        }
        else {
            clear_oneshot_mods();
            unregister_mods(ap.mods);
        }
    }
#else
    if (ap->action->kind.id != ACT_LMODS_TAP && ap->action->kind.id != ACT_RMODS_TAP) return;
    if (ap->action->layer_tap.code != MODS_ONESHOT) return;
    // Oneshot modifier
    if (ap->record->event.pressed) {
        if (ap->record->tap.count == 0) {
            ap->debug_print("MODS_TAP: Oneshot: 0\n");
            register_mods(ap.mods | get_oneshot_mods());
        }
        else if (ap->record->tap.count == 1) {
            ap->debug_print("MODS_TAP: Oneshot: start\n");
            set_oneshot_mods(ap.mods | get_oneshot_mods());
        }
        else {
            register_mods(ap.mods | get_oneshot_mods());
        }
    }
    else {
        if (ap->record->tap.count == 0) {
            clear_oneshot_mods();
            unregister_mods(ap.mods);
        }
        else if (ap->record->tap.count == 1) {
            // Retain Oneshot mods
        }
        else {
            clear_oneshot_mods();
            unregister_mods(ap.mods);
        }
    }
#endif
}

void ap_act_mods_tap_toggle(action_processor * ap) {
#ifdef NO_ACTION_TAPPING
    return;
#else
    if (ap->action->kind.id != ACT_LMODS_TAP && ap->action->kind.id != ACT_RMODS_TAP) return;
    if (ap->action->layer_tap.code != MODS_TAP_TOGGLE) return;
    if (ap->record->event.pressed) {
        if (ap->record->tap.count <= TAPPING_TOGGLE) {
            register_mods(ap.mods);
        }
    }
    else {
        if (ap->record->tap.count < TAPPING_TOGGLE) {
            unregister_mods(ap.mods);
        }
    }
#endif
}

void ap_act_mods_tap_default(action_processor * ap) {
#ifdef NO_ACTION_TAPPING
    return;
#else
    if (ap->action->kind.id != ACT_LMODS_TAP && ap->action->kind.id != ACT_RMODS_TAP) return;
#    ifndef NO_ACTION_ONESHOT
    if (ap->action->layer_tap.code == MODS_ONESHOT) return;
#    endif
    if (ap->action->layer_tap.code == MODS_TAP_TOGGLE) return;
    bool ignore_mod_tap_interrupt = false;
#    ifdef IGNORE_MOD_TAP_INTERRUPT_PER_KEY
    ignore_mod_tap_interrupt = !get_ignore_mod_tap_interrupt(get_event_keycode(ap->record->event, false), ap->record) && ap->record->tap.interrupted;
#    elifdef IGNORE_MOD_TAP_INTERRUPT
    ignore_mod_tap_interrupt = ap->record->tap.interrupted;
#    endif
    if (ap->record->event.pressed) {
        if (ap->record->tap.count > 0) {
            if (ignore_mod_tap_interrupt) {
                ap->debug_print("mods_tap: tap: cancel: add_mods\n");
                // ad hoc: set 0 to cancel tap
                ap->record->tap.count = 0;
                register_mods(ap.mods);
            }
            else {
                ap->debug_print("MODS_TAP: Tap: register_code\n");
                register_code(ap->action->key.code);
            }
        }
        else {
            ap->debug_print("MODS_TAP: No tap: add_mods\n");
            register_mods(ap.mods);
        }
    }
    else {
        if (ap->record->tap.count > 0) {
            ap->debug_print("MODS_TAP: Tap: unregister_code\n");
            if (ap->action->layer_tap.code == KC_CAPS) {
                ap->wait_ms(TAP_HOLD_CAPS_DELAY);
            }
            else {
                ap->wait_ms(TAP_CODE_DELAY);
            }
            unregister_code(ap->action->key.code);
        }
        else {
            ap->debug_print("MODS_TAP: No tap: add_mods\n");
            unregister_mods(ap.mods);
        }
    }
#endif
}

void ap_act_usage_system(action_processor * ap) {
#ifdef EXTRAKEY_ENABLE
    if (ap->action->kind.id != ACT_USAGE) return;
    /* other HID usage */
    if (ap->action->usage.page != PAGE_SYSTEM) return;
    if (ap->record->event.pressed) {
        host_system_send(ap->action->usage.code);
    }
    else {
        host_system_send(0);
    }
#else
    return;
#endif
}

void ap_act_usage_consumer(action_processor * ap) {
#ifdef EXTRAKEY_ENABLE
    if (ap->action->kind.id != ACT_USAGE) return;
    /* other HID usage */
    if (ap->action->usage.page != PAGE_CONSUMER) return;
    if (ap->record->event.pressed) {
        host_system_send(ap->action->usage.code);
    }
    else {
        host_system_send(0);
    }
#else
    return;
#endif
}

/* Mouse key */

void ap_act_mousekey(action_processor * ap) {
#ifdef MOUSEKEY_ENABLE
    if (ap->action->kind.id != ACT_MOUSEKEY) return;
    if (ap->record->event.pressed) {
        mousekey_on(ap->action->key.code);
    }
    else {
        mousekey_off(ap->action->key.code);
    }
    bool register_ps2_mouse_or_pointing_device = false;
#    ifdef PS2_MOUSE_ENABLE
    switch (ap->action->key.code) {
        case KC_MS_BTN1 ... KC_MS_BTN3:
            register_ps2_mouse_or_pointing_device = true;
    }
#    elifdef POINTING_DEVICE_ENABLE
    switch (ap->action->key.code) {
        case KC_MS_BTN1 ... KC_MS_BTN8:
            register_ps2_mouse_or_pointing_device = true;
    }
#    endif
    if (register_ps2_mouse_or_pointing_device) {
        register_button(ap->record->event.pressed, MOUSE_BTN_MASK(ap->action->key.code - KC_MS_BTN1));
    }
    else {
        mousekey_send();
    }
#else
    return;
#endif
}

void ap_act_layer_bitop_on(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
#else
    if (ap->action->kind.id != ACT_LAYER) return;
    if (ap->action->layer_bitop.on != 0) return;
    /* Default Layer Bitwise Operation */
    if (ap->record->event.pressed) return;
    uint8_t shift = ap->action->layer_bitop.part * 4;
    layer_state_t bits = ((layer_state_t) ap->action->layer_bitop.bits) << shift;
    layer_state_t mask = (ap->action->layer_bitop.xbit) ? ~(((layer_state_t) 0xf) << shift) : 0;
    switch (ap->action->layer_bitop.op) {
        case OP_BIT_AND:
            default_layer_and(bits | mask);
            break;
        case OP_BIT_OR:
            default_layer_or(bits | mask);
            break;
        case OP_BIT_XOR:
            default_layer_xor(bits | mask);
            break;
        case OP_BIT_SET:
            default_layer_set(bits | mask);
            break;
    }
#endif
}

void ap_act_layer_bitop_other(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
#else
    if (ap->action->kind.id != ACT_LAYER) return;
    if (ap->action->layer_bitop.on == 0) return;
    /* Layer Bitwise Operation */
    if (ap->record->event.pressed && (ap->action->layer_bitop.on & ON_PRESS)) return;
    if (!ap->record->event.pressed && (ap->action->layer_bitop.on & ON_RELEASE)) return;
    uint8_t shift = ap->action->layer_bitop.part * 4;
    layer_state_t bits = ((layer_state_t) ap->action->layer_bitop.bits) << shift;
    layer_state_t mask = (ap->action->layer_bitop.xbit) ? ~(((layer_state_t) 0xf) << shift) : 0;
    switch (ap->action->layer_bitop.op) {
        case OP_BIT_AND:
            default_layer_and(bits | mask);
            break;
        case OP_BIT_OR:
            default_layer_or(bits | mask);
            break;
        case OP_BIT_XOR:
            default_layer_xor(bits | mask);
            break;
        case OP_BIT_SET:
            default_layer_set(bits | mask);
            break;
    }
#endif
}

void ap_act_layer_mods(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
#else
    if (ap->action->kind.id != ACT_LAYER_MODS) return;
    if (ap->record->event.pressed) {
        layer_on(ap->action->layer_mods.layer);
        register_mods(ap->action->layer_mods.mods);
    }
    else {
        unregister_mods(ap->action->layer_mods.mods);
        layer_off(ap->action->layer_mods.layer);
    }
#endif
}

void ap_act_layer_tap_or_tap_ext_toggle(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
#elifdef NO_ACTION_TAPPING
    return;
#else
    if (ap->action->kind.id != ACT_LAYER_TAP && ap->action->kind.id != ACT_LAYER_TAP_EXT) return;
    if (ap->action->layer_tap.code != OP_TAP_TOGGLE) return;
    /* tap toggle */
    if (ap->record->event.pressed) {
        if (ap->record->tap.count < TAPPING_TOGGLE) {
            layer_invert(ap->action->layer_tap.val);
        }
    }
    else {
        if (ap->record->tap.count <= TAPPING_TOGGLE) {
            layer_invert(ap->action->layer_tap.val);
        }
    }
#endif
}

void ap_act_layer_tap_or_tap_ext_on_off(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
#elifdef NO_ACTION_TAPPING
    return;
#else
    if (ap->action->kind.id != ACT_LAYER_TAP && ap->action->kind.id != ACT_LAYER_TAP_EXT) return;
    if (ap->action->layer_tap.code != OP_ON_OFF) return;
    ap->record->event.pressed ? layer_on(ap->action->layer_tap.val) : layer_off(ap->action->layer_tap.val);
#endif
}

void ap_act_layer_tap_or_tap_ext_off_on(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
#elifdef NO_ACTION_TAPPING
    return;
#else
    if (ap->action->kind.id != ACT_LAYER_TAP && ap->action->kind.id != ACT_LAYER_TAP_EXT) return;
    if (ap->action->layer_tap.code != OP_OFF_ON) return;
    ap->record->event.pressed ? layer_off(ap->action->layer_tap.val) : layer_on(ap->action->layer_tap.val);
#endif
}

void ap_act_layer_tap_or_tap_ext_set_clear(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
#elifdef NO_ACTION_TAPPING
    return;
#else
    if (ap->action->kind.id != ACT_LAYER_TAP && ap->action->kind.id != ACT_LAYER_TAP_EXT) return;
    if (ap->action->layer_tap.code != OP_SET_CLEAR) return;
    ap->record->event.pressed ? layer_move(ap->action->layer_tap.val) : layer_clear();
#endif
}

void ap_act_layer_tap_or_tap_ext_oneshot(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
#elifdef NO_ACTION_TAPPING
    return;
#elifdef NO_ACTION_ONESHOT
    return;
#elifdef ONESHOT_TAP_TOGGLE
    if (ap->action->kind.id != ACT_LAYER_TAP && ap->action->kind.id != ACT_LAYER_TAP_EXT) return;
    if (ap->action->layer_tap.code != OP_ONESHOT) return;
    // Oneshot modifier
    if (!ap.oneshot_tap_toggle) {
        *(ap->do_release_oneshot) = false;
        if (ap->record->event.pressed) {
            del_mods(get_oneshot_locked_mods());
            if (get_oneshot_layer_state() == ONESHOT_TOGGLED) {
                reset_oneshot_layer();
                layer_off(ap->action->layer_tap.val);
            } else if (ap->record->tap.count < ONESHOT_TAP_TOGGLE) {
                layer_on(ap->action->layer_tap.val);
                set_oneshot_layer(ap->action->layer_tap.val, ONESHOT_START);
            }
        } else {
            add_mods(get_oneshot_locked_mods());
            if (ap->record->tap.count >= ONESHOT_TAP_TOGGLE) {
                reset_oneshot_layer();
                clear_oneshot_locked_mods();
                set_oneshot_layer(ap->action->layer_tap.val, ONESHOT_TOGGLED);
            } else {
                clear_oneshot_layer_state(ONESHOT_PRESSED);
            }
        }
    }
    else {
        if (ap->record->event.pressed) {
            layer_on(ap->action->layer_tap.val);
            set_oneshot_layer(ap->action->layer_tap.val, ONESHOT_START);
        } else {
            clear_oneshot_layer_state(ONESHOT_PRESSED);
            if (ap->record->tap.count > 1) {
                clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
            }
        }
    }
#else
    if (ap->record->event.pressed) {
        layer_on(ap->action->layer_tap.val);
        set_oneshot_layer(ap->action->layer_tap.val, ONESHOT_START);
    } else {
        clear_oneshot_layer_state(ONESHOT_PRESSED);
        if (ap->record->tap.count > 1) {
            clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
        }
    }
#endif
}

void ap_act_layer_tap_or_tap_ext_default(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
#elifdef NO_ACTION_TAPPING
    return;
#elifdef NO_ACTION_ONESHOT
    if (ap->action->kind.id != ACT_LAYER_TAP && ap->action->kind.id != ACT_LAYER_TAP_EXT) return;
    if (ap->action->layer_tap.code == OP_TAP_TOGGLE) return;
    if (ap->action->layer_tap.code == OP_ON_OFF) return;
    if (ap->action->layer_tap.code == OP_OFF_ON) return;
    if (ap->action->layer_tap.code == OP_SET_CLEAR) return;
    /* tap key */
    if (ap->record->event.pressed) {
        if (ap->record->tap.count > 0) {
            ap->debug_print("KEYMAP_TAP_KEY: Tap: register_code\n");
            register_code(ap->action->layer_tap.code);
        }
        else {
            ap->debug_print("KEYMAP_TAP_KEY: No tap: On on press\n");
            layer_on(ap->action->layer_tap.val);
        }
    }
    else {
        if (ap->record->tap.count > 0) {
            ap->debug_print("KEYMAP_TAP_KEY: Tap: unregister_code\n");
            if (ap->action->layer_tap.code == KC_CAPS) {
                ap->wait_ms(TAP_HOLD_CAPS_DELAY);
            }
            else {
                ap->wait_ms(TAP_CODE_DELAY);
            }
            unregister_code(ap->action->layer_tap.code);
        }
        else {
            ap->debug_print("KEYMAP_TAP_KEY: No tap: Off on release\n");
            layer_off(ap->action->layer_tap.val);
        }
    }
#else
    if (ap->action->kind.id != ACT_LAYER_TAP && ap->action->kind.id != ACT_LAYER_TAP_EXT) return;
    if (ap->action->layer_tap.code == OP_TAP_TOGGLE) return;
    if (ap->action->layer_tap.code == OP_ON_OFF) return;
    if (ap->action->layer_tap.code == OP_OFF_ON) return;
    if (ap->action->layer_tap.code == OP_SET_CLEAR) return;
    if (ap->action->layer_tap.code == OP_ONESHOT) return;
    /* tap key */
    if (ap->record->event.pressed) {
        if (ap->record->tap.count > 0) {
            ap->debug_print("KEYMAP_TAP_KEY: Tap: register_code\n");
            register_code(ap->action->layer_tap.code);
        }
        else {
            ap->debug_print("KEYMAP_TAP_KEY: No tap: On on press\n");
            layer_on(ap->action->layer_tap.val);
        }
    }
    else {
        if (ap->record->tap.count > 0) {
            ap->debug_print("KEYMAP_TAP_KEY: Tap: unregister_code\n");
            if (ap->action->layer_tap.code == KC_CAPS) {
                ap->wait_ms(TAP_HOLD_CAPS_DELAY);
            }
            else {
                ap->wait_ms(TAP_CODE_DELAY);
            }
            unregister_code(ap->action->layer_tap.code);
        }
        else {
            ap->debug_print("KEYMAP_TAP_KEY: No tap: Off on release\n");
            layer_off(ap->action->layer_tap.val);
        }
    }
#endif
}

/* Extentions */

void ap_act_macro(action_processor * ap) {
#ifdef NO_ACTION_MACRO
    return;
#else
    if (ap->action->kind.id != ACT_MACRO) return;
    action_macro_play(action_get_macro(ap->record, ap->action->func.id, ap->action->func.opt));
#endif
}

void ap_act_swap_hands_toggle(action_processor * ap) {
#ifdef SWAP_HANDS_ENABLE
    if (ap->action->kind.id != ACT_SWAP_HANDS) return;
    if (ap->action->swap.code != OP_SH_TOGGLE) return;
    if (ap->record->event.pressed) {
        *(ap->swap_hands) = !(*(ap->swap_hands));
    }
#else
    return;
#endif
}

void ap_act_swap_hands_on_off(action_processor * ap) {
#ifdef SWAP_HANDS_ENABLE
    if (ap->action->kind.id != ACT_SWAP_HANDS) return;
    if (ap->action->swap.code != OP_SH_ON_OFF) return;
    *(ap->swap_hands) = ap->record->event.pressed;
#else
    return;
#endif
}

void ap_act_swap_hands_off_on(action_processor * ap) {
#ifdef SWAP_HANDS_ENABLE
    if (ap->action->kind.id != ACT_SWAP_HANDS) return;
    if (ap->action->swap.code != OP_SH_OFF_ON) return;
    *(ap->swap_hands) = !ap->record->event.pressed;
#else
    return;
#endif
}

void ap_act_swap_hands_on(action_processor * ap) {
#ifdef SWAP_HANDS_ENABLE
    if (ap->action->kind.id != ACT_SWAP_HANDS) return;
    if (ap->action->swap.code != OP_SH_ON) return;
    if (!ap->record->event.pressed) {
        *(ap->swap_hands) = true;
    }
#else
    return;
#endif
}

void ap_act_swap_hands_off(action_processor * ap) {
#ifdef SWAP_HANDS_ENABLE
    if (ap->action->kind.id != ACT_SWAP_HANDS) return;
    if (ap->action->swap.code != OP_SH_OFF) return;
    if (!ap->record->event.pressed) {
        *(ap->swap_hands) = false;
    }
#else
    return;
#endif
}

void ap_act_swap_hands_oneshot(action_processor * ap) {
#ifdef NO_ACTION_ONESHOT
    return;
#elifdef SWAP_HANDS_ENABLE
    if (ap->action->kind.id != ACT_SWAP_HANDS) return;
    if (ap->action->swap.code != OP_SH_ONESHOT) return;
    if (ap->record->event.pressed) {
        set_oneshot_swaphands();
    }
    else {
        release_oneshot_swaphands();
    }
#else
    return;
#endif
}

void ap_act_swap_hands_tap_toggle(action_processor * ap) {
#ifdef NO_ACTION_TAPPING
    return;
#elifdef SWAP_HANDS_ENABLE
    if (ap->action->kind.id != ACT_SWAP_HANDS) return;
    if (ap->action->swap.code != OP_SH_TAP_TOGGLE) return;
    /* tap toggle */
    if (ap->record->event.pressed) {
        if (ap->swap_held) {
            *(ap->swap_held) = false;
        }
        else {
            *(ap->swap_hands) = !(*(ap->swap_hands));
        }
    }
    else {
        if (ap->record->tap.count < TAPPING_TOGGLE) {
            *(ap->swap_hands) = !(*(ap->swap_hands));
        }
    }
#else
    return;
#endif
}

void ap_act_swap_hands_default(action_processor * ap) {
#ifdef NO_ACTION_TAPPING
    return;
#elifdef SWAP_HANDS_ENABLE
    if (ap->action->kind.id != ACT_SWAP_HANDS) return;
    if (ap->action->swap.code == OP_SH_TOGGLE) return;
    if (ap->action->swap.code == OP_SH_ON_OFF) return;
    if (ap->action->swap.code == OP_SH_OFF_ON) return;
    if (ap->action->swap.code == OP_SH_ON) return;
    if (ap->action->swap.code == OP_SH_OFF) return;
#    ifndef NO_ACTION_ONESHOT
    if (ap->action->swap.code == OP_SH_ONESHOT) return;
#    endif
    if (ap->action->swap.code == OP_SH_TAP_TOGGLE) return;
    /* tap key */
    if (ap->record->tap.count > 0) {
        if (ap->swap_held) {
            *(ap->swap_hands) = !(*(ap->swap_hands));  // undo hold set up in _tap_hint
            *(ap->swap_held) = false;
        }
        if (ap->record->event.pressed) {
            register_code(ap->action->swap.code);
        }
        else {
            ap->wait_ms(TAP_CODE_DELAY);
            unregister_code(ap->action->swap.code);
            ap->record = (keyrecord_t){};  // hack: reset tap mode
        }
    }
    else {
        if (ap->swap_held && !ap->record->event.pressed) {
            *(ap->swap_hands) = !(*(ap->swap_hands));  // undo hold set up in _tap_hint
            *(ap->swap_held) = false;
        }
    }
#else
    return;
#endif
}

void ap_act_function(action_processor * ap) {
#ifdef NO_ACTION_FUNCTION
    return;
#else
    if (ap->action->kind.id != ACT_FUNCTION) return;
    action_function(ap->record, ap->action->func.id, ap->action->func.opt);
#endif
}

void ap_layer_led(action_processor * ap) {
#ifdef NO_ACTION_LAYER
    return;
    // if this event is a layer action, update the leds
#elifdef NO_ACTION_TAPPING
    if (ap->action->kind.id != ACT_LAYER && ap->action->kind.id != ACT_LAYER_MODS) return;
    led_set(host_keyboard_leds());
#else
    if (
        ap->action->kind.id != ACT_LAYER
        && ap->action->kind.id != ACT_LAYER_MODS
        && ap->action->kind.id != ACT_LAYER_TAP
        && ap->action->kind.id != ACT_LAYER_TAP_EXT
    ) {
        return;
    }
    led_set(host_keyboard_leds());
#endif
}

void ap_retro_tapping(action_processor * ap) {
#ifdef NO_ACTION_TAPPING
    return;
#elifdef RETRO_TAPPING_PER_KEY
    if (!is_tap_action(ap->action)) {
        *(ap->retro_tapping_counter) = 0;
    }
    else {
        if (ap->record->event.pressed) {
            if (ap->record->tap.count > 0) {
                *(ap->retro_tapping_counter) = 0;
            }
        }
        else {
            if (ap->record->tap.count > 0) {
                *(ap->retro_tapping_counter) = 0;
            }
            else {
                if (get_retro_tapping(get_event_keycode(ap->record->event, false), ap->record && *(ap->retro_tapping_counter) == 2)) {
                    tap_code(ap->action->layer_tap.code);
                }
                *(ap->retro_tapping_counter) = 0;
            }
        }
    }
#elifdef RETRO_TAPPING
    if (!is_tap_action(ap->action)) {
        *(ap->retro_tapping_counter) = 0;
    }
    else {
        if (ap->record->event.pressed) {
            if (ap->record->tap.count > 0) {
                *(ap->retro_tapping_counter) = 0;
            }
        }
        else {
            if (ap->record->tap.count > 0) {
                *(ap->retro_tapping_counter) = 0;
            }
            else {
                if (*(ap->retro_tapping_counter) == 2)) {
                        tap_code(ap->action->layer_tap.code);
                    }
                *(ap->retro_tapping_counter) = 0;
            }
        }
    }
#else
    return;
#endif
}

void ap_oneshot_swaphands(action_processor * ap) {
#ifdef NO_ACTION_ONESHOT
    return;
#elifdef SWAP_HANDS_ENABLE
    if (ap->record->event.pressed && !(ap->action->kind.id == ACT_SWAP_HANDS && ap->action->swap.code == OP_SH_ONESHOT)) {
        use_oneshot_swaphands();
    }
#else
    return;
#endif
}

void ap_release_oneshot(action_processor * ap) {
#ifdef NO_ACTION_ONESHOT
    return;
#else
    /* Because we switch layers after a oneshot event, we need to release the
     * key before we leave the layer or no key up event will be generated.
     */
    if (*(ap->do_release_oneshot) && !(get_oneshot_layer_state() & ONESHOT_PRESSED)) {
        ap->record->event.pressed = false;
        layer_on(get_oneshot_layer());
        process_record(ap->record);
        layer_off(get_oneshot_layer());
    }
#endif
}

void ap_debug_print(char * arg) {
    dprint(arg);
}

void ap_wait_ms(int arg) {
    wait_ms(arg);
}

void ap_run(action_processor * ap) {
    ap->act_mods(ap);
    ap->act_mods_tap_oneshot(ap);
    ap->act_mods_tap_toggle(ap);
    ap->act_mods_tap_default(ap);
    ap->act_usage_system(ap);
    ap->act_usage_consumer(ap);
    ap->act_mousekey(ap);
    ap->act_layer_bitop_on(ap);
    ap->act_layer_bitop_other(ap);
    ap->act_layer_mods(ap);
    ap->act_layer_tap_or_tap_ext_toggle(ap);
    ap->act_layer_tap_or_tap_ext_on_off(ap);
    ap->act_layer_tap_or_tap_ext_off_on(ap);
    ap->act_layer_tap_or_tap_ext_set_clear(ap);
    ap->act_layer_tap_or_tap_ext_oneshot(ap);
    ap->act_layer_tap_or_tap_ext_default(ap);
    ap->act_macro(ap);
    ap->act_swap_hands_toggle(ap);
    ap->act_swap_hands_on_off(ap);
    ap->act_swap_hands_off_on(ap);
    ap->act_swap_hands_on(ap);
    ap->act_swap_hands_off(ap);
    ap->act_swap_hands_oneshot(ap);
    ap->act_swap_hands_tap_toggle(ap);
    ap->act_swap_hands_default(ap);
    ap->act_function(ap);
    ap->layer_led(ap);
    ap->retro_tapping(ap);
    ap->oneshot_swaphands(ap);
}

action_processor action_processor_init(keyrecord_t * record, action_t * action) {
    action_processor ap;
    ap.record = record;
    ap.action = action;
    if (action->kind.id == ACT_LMODS || action->kind.id == ACT_RMODS) {
        ap.mods = (action->kind.id == ACT_LMODS) ? action->key.mods : action->key.mods << 4;
    }
#ifndef NO_ACTION_TAPPING
    else if (action->kind.id == ACT_LMODS_TAP || action->kind.id == ACT_RMODS_TAP) {
        ap.mods = (action->kind.id == ACT_LMODS_TAP) ? action->key.mods : action->key.mods << 4;
    }
#endif
#ifdef ONESHOT_TAP_TOGGLE
    if (ONESHOT_TAP_TOGGLE > 1) {
        ap.oneshot_tap_toggle = true;
    }
    else {
        ap.oneshot_tap_toggle = false;
    }
#endif
    ap.act_mods = ap_act_mods;
    ap.act_mods_tap_oneshot = ap_act_mods_tap_oneshot;
    ap.act_mods_tap_toggle = ap_act_mods_tap_toggle;
    ap.act_mods_tap_default = ap_act_mods_tap_default;
    ap.act_usage_system = ap_act_usage_system;
    ap.act_usage_consumer = ap_act_usage_consumer;
    ap.act_mousekey = ap_act_mousekey;
    ap.act_layer_bitop_on = ap_act_layer_bitop_on;
    ap.act_layer_bitop_other = ap_act_layer_bitop_other;
    ap.act_layer_mods = ap_act_layer_mods;
    ap.act_layer_tap_or_tap_ext_toggle = ap_act_layer_tap_or_tap_ext_toggle;
    ap.act_layer_tap_or_tap_ext_on_off = ap_act_layer_tap_or_tap_ext_on_off;
    ap.act_layer_tap_or_tap_ext_off_on = ap_act_layer_tap_or_tap_ext_off_on;
    ap.act_layer_tap_or_tap_ext_set_clear = ap_act_layer_tap_or_tap_ext_set_clear;
    ap.act_layer_tap_or_tap_ext_oneshot = ap_act_layer_tap_or_tap_ext_oneshot;
    ap.act_layer_tap_or_tap_ext_default = ap_act_layer_tap_or_tap_ext_default;
    ap.act_macro = ap_act_macro;
    ap.act_swap_hands_toggle = ap_act_swap_hands_toggle;
    ap.act_swap_hands_on_off = ap_act_swap_hands_on_off;
    ap.act_swap_hands_off_on = ap_act_swap_hands_off_on;
    ap.act_swap_hands_on = ap_act_swap_hands_on;
    ap.act_swap_hands_off = ap_act_swap_hands_off;
    ap.act_swap_hands_oneshot = ap_act_swap_hands_oneshot;
    ap.act_swap_hands_tap_toggle = ap_act_swap_hands_tap_toggle;
    ap.act_swap_hands_default = ap_act_swap_hands_default;
    ap.act_function = ap_act_function;
    ap.layer_led = ap_layer_led;
    ap.retro_tapping = ap_retro_tapping;
    ap.oneshot_swaphands = ap_oneshot_swaphands;
    ap.release_oneshot = ap_release_oneshot;
    ap.debug_print = ap_debug_print;
    ap.wait_ms = ap_wait_ms;
    ap.run = ap_run;
    return ap;
}

/** \brief Take an action and processes it.
 *
 * FIXME: Needs documentation.
 */
void process_action(keyrecord_t *record, action_t action) {
    keyevent_t event = record->event;
#ifndef NO_ACTION_TAPPING
    uint8_t tap_count = record->tap.count;
#endif

#ifndef NO_ACTION_ONESHOT
    bool do_release_oneshot = false;
    // notice we only clear the one shot layer if the pressed key is not a modifier.
    if (is_oneshot_layer_active() && event.pressed && (action.kind.id == ACT_USAGE || !IS_MOD(action.key.code))
#    ifdef SWAP_HANDS_ENABLE
        && !(action.kind.id == ACT_SWAP_HANDS && action.swap.code == OP_SH_ONESHOT)
#    endif
    ) {
        clear_oneshot_layer_state(ONESHOT_OTHER_KEY_PRESSED);
        do_release_oneshot = !is_oneshot_layer_active();
    }
#endif

    action_processor ap = action_processor_init(record, &action);
#ifndef NO_ACTION_ONESHOT
    *(ap.do_release_oneshot) = do_release_oneshot;
#endif
#ifdef SWAP_HANDS_ENABLE
    *(ap.swap_hands) = swap_hands;
#    ifndef NO_ACTION_TAPPING
    *(ap.swap_held) = swap_held;
#    endif
#endif
#ifdef RETRO_TAPPING
    *(ap.retro_tapping_counter) = retro_tapping_counter;
#elifdef RETRO_TAPPING_PER_KEY
    *(ap.retro_tapping_counter) = retro_tapping_counter;
#endif
    ap.run();
}

/** \brief Utilities for actions. (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
void register_code(uint8_t code) {
    if (code == KC_NO) {
        return;
    }
#ifdef LOCKING_SUPPORT_ENABLE
    else if (KC_LOCKING_CAPS == code) {
#    ifdef LOCKING_RESYNC_ENABLE
        // Resync: ignore if caps lock already is on
        if (host_keyboard_leds() & (1 << USB_LED_CAPS_LOCK)) return;
#    endif
        add_key(KC_CAPSLOCK);
        send_keyboard_report();
        wait_ms(100);
        del_key(KC_CAPSLOCK);
        send_keyboard_report();
    }

    else if (KC_LOCKING_NUM == code) {
#    ifdef LOCKING_RESYNC_ENABLE
        if (host_keyboard_leds() & (1 << USB_LED_NUM_LOCK)) return;
#    endif
        add_key(KC_NUMLOCK);
        send_keyboard_report();
        wait_ms(100);
        del_key(KC_NUMLOCK);
        send_keyboard_report();
    }

    else if (KC_LOCKING_SCROLL == code) {
#    ifdef LOCKING_RESYNC_ENABLE
        if (host_keyboard_leds() & (1 << USB_LED_SCROLL_LOCK)) return;
#    endif
        add_key(KC_SCROLLLOCK);
        send_keyboard_report();
        wait_ms(100);
        del_key(KC_SCROLLLOCK);
        send_keyboard_report();
    }
#endif

    else if IS_KEY (code) {
        // TODO: should push command_proc out of this block?
        if (command_proc(code)) return;

#ifndef NO_ACTION_ONESHOT
/* TODO: remove
        if (oneshot_state.mods && !oneshot_state.disabled) {
            uint8_t tmp_mods = get_mods();
            add_mods(oneshot_state.mods);

            add_key(code);
            send_keyboard_report();

            set_mods(tmp_mods);
            send_keyboard_report();
            oneshot_cancel();
        } else
*/
#endif
        {
            // Force a new key press if the key is already pressed
            // without this, keys with the same keycode, but different
            // modifiers will be reported incorrectly, see issue #1708
            if (is_key_pressed(keyboard_report, code)) {
                del_key(code);
                send_keyboard_report();
            }
            add_key(code);
            send_keyboard_report();
        }
    } else if IS_MOD (code) {
        add_mods(MOD_BIT(code));
        send_keyboard_report();
    }
#ifdef EXTRAKEY_ENABLE
    else if IS_SYSTEM (code) {
        host_system_send(KEYCODE2SYSTEM(code));
    } else if IS_CONSUMER (code) {
        host_consumer_send(KEYCODE2CONSUMER(code));
    }
#endif
#ifdef MOUSEKEY_ENABLE
    else if IS_MOUSEKEY (code) {
        mousekey_on(code);
        mousekey_send();
    }
#endif
}

/** \brief Utilities for actions. (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
void unregister_code(uint8_t code) {
    if (code == KC_NO) {
        return;
    }
#ifdef LOCKING_SUPPORT_ENABLE
    else if (KC_LOCKING_CAPS == code) {
#    ifdef LOCKING_RESYNC_ENABLE
        // Resync: ignore if caps lock already is off
        if (!(host_keyboard_leds() & (1 << USB_LED_CAPS_LOCK))) return;
#    endif
        add_key(KC_CAPSLOCK);
        send_keyboard_report();
        del_key(KC_CAPSLOCK);
        send_keyboard_report();
    }

    else if (KC_LOCKING_NUM == code) {
#    ifdef LOCKING_RESYNC_ENABLE
        if (!(host_keyboard_leds() & (1 << USB_LED_NUM_LOCK))) return;
#    endif
        add_key(KC_NUMLOCK);
        send_keyboard_report();
        del_key(KC_NUMLOCK);
        send_keyboard_report();
    }

    else if (KC_LOCKING_SCROLL == code) {
#    ifdef LOCKING_RESYNC_ENABLE
        if (!(host_keyboard_leds() & (1 << USB_LED_SCROLL_LOCK))) return;
#    endif
        add_key(KC_SCROLLLOCK);
        send_keyboard_report();
        del_key(KC_SCROLLLOCK);
        send_keyboard_report();
    }
#endif

    else if IS_KEY (code) {
        del_key(code);
        send_keyboard_report();
    } else if IS_MOD (code) {
        del_mods(MOD_BIT(code));
        send_keyboard_report();
    } else if IS_SYSTEM (code) {
        host_system_send(0);
    } else if IS_CONSUMER (code) {
        host_consumer_send(0);
    }
#ifdef MOUSEKEY_ENABLE
    else if IS_MOUSEKEY (code) {
        mousekey_off(code);
        mousekey_send();
    }
#endif
}

/** \brief Tap a keycode with a delay.
 *
 * \param code The basic keycode to tap.
 * \param delay The amount of time in milliseconds to leave the keycode registered, before unregistering it.
 */
void tap_code_delay(uint8_t code, uint16_t delay) {
    register_code(code);
    for (uint16_t i = delay; i > 0; i--) {
        wait_ms(1);
    }
    unregister_code(code);
}

/** \brief Tap a keycode with the default delay.
 *
 * \param code The basic keycode to tap. If `code` is `KC_CAPS`, the delay will be `TAP_HOLD_CAPS_DELAY`, otherwise `TAP_CODE_DELAY`, if defined.
 */
void tap_code(uint8_t code) { tap_code_delay(code, code == KC_CAPS ? TAP_HOLD_CAPS_DELAY : TAP_CODE_DELAY); }

/** \brief Adds the given physically pressed modifiers and sends a keyboard report immediately.
 *
 * \param mods A bitfield of modifiers to register.
 */
void register_mods(uint8_t mods) {
    if (mods) {
        add_mods(mods);
        send_keyboard_report();
    }
}

/** \brief Removes the given physically pressed modifiers and sends a keyboard report immediately.
 *
 * \param mods A bitfield of modifiers to unregister.
 */
void unregister_mods(uint8_t mods) {
    if (mods) {
        del_mods(mods);
        send_keyboard_report();
    }
}

/** \brief Adds the given weak modifiers and sends a keyboard report immediately.
 *
 * \param mods A bitfield of modifiers to register.
 */
void register_weak_mods(uint8_t mods) {
    if (mods) {
        add_weak_mods(mods);
        send_keyboard_report();
    }
}

/** \brief Removes the given weak modifiers and sends a keyboard report immediately.
 *
 * \param mods A bitfield of modifiers to unregister.
 */
void unregister_weak_mods(uint8_t mods) {
    if (mods) {
        del_weak_mods(mods);
        send_keyboard_report();
    }
}

/** \brief Utilities for actions. (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
void clear_keyboard(void) {
    clear_mods();
    clear_keyboard_but_mods();
}

/** \brief Utilities for actions. (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
void clear_keyboard_but_mods(void) {
    clear_keys();
    clear_keyboard_but_mods_and_keys();
}

/** \brief Utilities for actions. (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
void clear_keyboard_but_mods_and_keys() {
#ifdef EXTRAKEY_ENABLE
    host_system_send(0);
    host_consumer_send(0);
#endif
    clear_weak_mods();
    clear_macro_mods();
    send_keyboard_report();
#ifdef MOUSEKEY_ENABLE
    mousekey_clear();
    mousekey_send();
#endif
}

/** \brief Utilities for actions. (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
bool is_tap_key(keypos_t key) {
    action_t action = layer_switch_get_action(key);
    return is_tap_action(action);
}

/** \brief Utilities for actions. (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
bool is_tap_record(keyrecord_t *record) {
#ifdef COMBO_ENABLE
    action_t action;
    if (record->keycode) {
        action = action_for_keycode(record->keycode);
    } else {
        action = layer_switch_get_action(record->event.key);
    }
#else
    action_t action = layer_switch_get_action(record->event.key);
#endif
    return is_tap_action(action);
}

/** \brief Utilities for actions. (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
bool is_tap_action(action_t action) {
    switch (action.kind.id) {
        case ACT_LMODS_TAP:
        case ACT_RMODS_TAP:
        case ACT_LAYER_TAP:
        case ACT_LAYER_TAP_EXT:
            switch (action.layer_tap.code) {
                case KC_NO ... KC_RGUI:
                case OP_TAP_TOGGLE:
                case OP_ONESHOT:
                    return true;
            }
            return false;
        case ACT_SWAP_HANDS:
            switch (action.swap.code) {
                case KC_NO ... KC_RGUI:
                case OP_SH_TAP_TOGGLE:
                    return true;
            }
            return false;
        case ACT_MACRO:
        case ACT_FUNCTION:
            if (action.func.opt & FUNC_TAP) {
                return true;
            }
            return false;
    }
    return false;
}

/** \brief Debug print (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
void debug_event(keyevent_t event) { dprintf("%04X%c(%u)", (event.key.row << 8 | event.key.col), (event.pressed ? 'd' : 'u'), event.time); }
/** \brief Debug print (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
void debug_record(keyrecord_t record) {
    debug_event(record.event);
#ifndef NO_ACTION_TAPPING
    dprintf(":%u%c", record.tap.count, (record.tap.interrupted ? '-' : ' '));
#endif
}

/** \brief Debug print (FIXME: Needs better description)
 *
 * FIXME: Needs documentation.
 */
void debug_action(action_t action) {
    switch (action.kind.id) {
        case ACT_LMODS:
            dprint("ACT_LMODS");
            break;
        case ACT_RMODS:
            dprint("ACT_RMODS");
            break;
        case ACT_LMODS_TAP:
            dprint("ACT_LMODS_TAP");
            break;
        case ACT_RMODS_TAP:
            dprint("ACT_RMODS_TAP");
            break;
        case ACT_USAGE:
            dprint("ACT_USAGE");
            break;
        case ACT_MOUSEKEY:
            dprint("ACT_MOUSEKEY");
            break;
        case ACT_LAYER:
            dprint("ACT_LAYER");
            break;
        case ACT_LAYER_MODS:
            dprint("ACT_LAYER_MODS");
            break;
        case ACT_LAYER_TAP:
            dprint("ACT_LAYER_TAP");
            break;
        case ACT_LAYER_TAP_EXT:
            dprint("ACT_LAYER_TAP_EXT");
            break;
        case ACT_MACRO:
            dprint("ACT_MACRO");
            break;
        case ACT_FUNCTION:
            dprint("ACT_FUNCTION");
            break;
        case ACT_SWAP_HANDS:
            dprint("ACT_SWAP_HANDS");
            break;
        default:
            dprint("UNKNOWN");
            break;
    }
    dprintf("[%X:%02X]", action.kind.param >> 8, action.kind.param & 0xff);
}
