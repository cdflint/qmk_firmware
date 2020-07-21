#include QMK_KEYBOARD_H

enum custom_keycodes {
  QWERTY = SAFE_RANGE,
  DYNAMIC_MACRO_RANGE,
};

#include "dynamic_macro.h"
// Define keys for recording dynamic macros to shorthand
#define KC_REC DYN_REC_START1
#define KC_DONE DYN_REC_STOP
#define KC_PLAY DYN_MACRO_PLAY1

enum layout_names {
    _BL=0,      // Base layer: standard typing layeer
    _FL,        // Function layer: Functions like toggle rgb etc
    _X,         // Keyboard Lock:
};

// bool has_layer_changed = false;
// static uint8_t current_layer;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

	[_BL] = LAYOUT_hotswap(
    KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_PSCR, KC_HOME, KC_END,  KC_PGUP, KC_PGDN, KC_DEL,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,          KC_NLCK, KC_PSLS, KC_PAST, KC_PMNS,
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,          KC_P7,   KC_P8,   KC_P9,
    KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,                    KC_P4,   KC_P5,   KC_P6,   KC_PPLS,
    KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_UP,            KC_P1,   KC_P2,   KC_P3,
    KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, MO(_FL), KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT, KC_P0,   KC_PDOT, KC_PENT
    ),

	[_FL] = LAYOUT_hotswap(
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, TG(_X) , _______, _______, KC_VOLU, KC_VOLD, KC_MUTE,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          KC_CALC, _______, _______, _______,
    RGB_TOG, _______, RGB_MOD, RGB_HUI, RGB_SAI, RGB_VAI, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, _______,
    BL_TOGG, _______, _______, RGB_HUD, RGB_SAD, RGB_VAD, _______, _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______,
    BL_INC,           _______, _______, _______, _______, RESET,   _______, _______, _______, _______, _______, _______, _______,          _______, _______, _______,
    BL_DEC,  _______, _______,                            KC_PLAY,                            KC_REC,  _______, KC_DONE, _______, _______, _______, _______, _______, _______
    ),

    [_X] = LAYOUT_hotswap(
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX,          XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX,                            XXXXXXX,                            XXXXXXX, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    ),
};

const macro_t *action_get_macro(keyrecord_t *record, uint8_t id, uint8_t opt) {
	return MACRO_NONE;
};

void matrix_init_user(void) {
};

void matrix_scan_user(void) {
    // if (alt_tab_enabled && (timer_elapsed(alt_tab_timer) > 100)) {
    //     tap_code(KC_TAB);
    //     alt_tab_timer = timer_read();
    // }
};

#ifdef RGBLIGHT_ENABLE
void keyboard_post_init_user(void) {
    rgblight_enable_noeeprom();  // Enables RGB, without saving settings
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    // rgblight_sethsv_noeeprom(HSV_ORANGE);
}
#endif

uint32_t layer_state_set_user(uint32_t state) {
    switch (biton32(state)) {
    case _BL:
        rgblight_sethsv_noeeprom(28, 234, 190);
        // rgblight_setrgb()
        break;
    case _FL:
        rgblight_sethsv_noeeprom(110, 255, 108);
        break;
    case _X:
        rgblight_sethsv_noeeprom(251, 255, 108);
        break;
    default:
        rgblight_sethsv_noeeprom(219, 146, 35);
        break;
    }
    return state;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_dynamic_macro(keycode, record)) {
        return false;
    }
    return true;

    // bool     alt_tab_enabled;
    // uint32_t alt_tab_timer;
    // switch (keycode) {
    //     case ALT_TAB:
    //         alt_tab_enabled = record->event.pressed;
    //         if (alt_tab_enabled) {
    //             alt_tab_timer = timer_read();
    //             register_code(KC_LALT);
    //             tap_code(KC_TAB);
    //         } else {
    //             unregister_code(KC_LALT);
    //         }
    //         break;
    // }
    // return true;
}
