#include QMK_KEYBOARD_H

// Include for debugging.
#include <print.h>

#define MILLISECONDS_IN_SECOND 1000

//========================================================== CONFIGURABLE DEFAULTS ==========================================================
#define RGB_DEFAULT_TIME_OUT 30
#define RGB_FAST_MODE_TIME_OUT 3
#define RGB_TIME_OUT_MAX 600
#define RGB_TIME_OUT_MIN 10
#define RGB_TIME_OUT_STEP 10

static uint16_t idle_timer;             // Idle LED timeout timer
static uint8_t idle_second_counter;     // Idle LED seconds counter, counts seconds not milliseconds

bool rgb_enabled_flag;                  // Current LED state flag. If false then LED is off.
bool rgb_time_out_enable;               // Idle LED toggle enable. If false then LED will not turn off after idle timeout.
bool rgb_time_out_fast_mode_enabled;
bool rgb_time_out_user_value;           // This holds the toggle value set by user with ROUT_TG. It's necessary as RGB_TOG changes timeout enable.
uint16_t rgb_time_out_seconds;          // Idle LED timeout value, in seconds not milliseconds
uint16_t rgb_time_out_saved_seconds;
led_flags_t rgb_time_out_saved_flag;    // Store LED flag before timeout so it can be restored when LED is turned on again.

enum tapdance_keycodes {
    TD_GUI_ML = 0,     // Tap dance key to switch to mouse layer _ML
    TD_LCTRL_TERM,
    TD_RCTRL_TERM,
    TD_APP_LCSA,
};

enum ctrl_keycodes {
    U_T_AUTO = SAFE_RANGE, //USB Extra Port Toggle Auto Detect / Always Active
    U_T_AGCR,              //USB Toggle Automatic GCR control
    DBG_TOG,               //DEBUG Toggle On / Off
    DBG_MTRX,              //DEBUG Toggle Matrix Prints
    DBG_KBD,               //DEBUG Toggle Keyboard Prints
    DBG_MOU,               //DEBUG Toggle Mouse Prints
    MD_BOOT,               //Restart into bootloader after hold timeout
    SEL_CPY,               //Select Copy. Select the word cursor is pointed at and copy, using double mouse click and ctrl+c
    ROUT_TG,               //Timeout Toggle. Toggle idle LED time out on or off
    ROUT_VI,               //Timeout Value Increase. Increase idle time out before LED disabled
    ROUT_VD,               //Timeout Value Decrease. Decrease idle time out before LED disabled
    ROUT_FM,               //RGB timeout fast mode toggle
    COPY_ALL,              //Copy all text using ctrl(a+c)
    TERMINAL,
};

enum layout_names {
    _KL=0,       // Keys Layout: The main keyboard layout that has all the characters
    _FL,         // Function Layout: The function key activated layout with default functions and some added ones
    _ML,         // Mouse Layout: Mouse Keys and mouse movement
    _LCSA,       // Application Launcher: Layer for AHK script that will wrap Left Control + Shift + Alt around keys
    _GL,         // Game Layout: Disable gui key
    _X,          // Lock Keyboard: Testing functionality to lock keyboard, disabling all keys but FN and ScrLK
    _YL,         // Yakuake Layout: Yakuake drop-down terminal shortcuts and macros
    _EL,         // KDE Layout: Shortcuts for KDE desktop using default KDE shortcuts settings
};

//Associate our tap dance key with its functionality
qk_tap_dance_action_t tap_dance_actions[] = {
    [TD_GUI_ML]     = ACTION_TAP_DANCE_LAYER_TOGGLE(KC_LGUI, _ML),
    [TD_LCTRL_TERM] = ACTION_TAP_DANCE_DOUBLE(KC_LCTRL, LCA(KC_T)),
    [TD_RCTRL_TERM] = ACTION_TAP_DANCE_DOUBLE(KC_RCTRL, LCA(KC_T)),
    [TD_APP_LCSA] = ACTION_TAP_DANCE_LAYER_TOGGLE(KC_APP, _LCSA),
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
    [DEFAULT] = LAYOUT(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,             KC_PSCR, KC_SLCK, KC_PAUS,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,   KC_INS,  KC_HOME, KC_PGUP,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,   KC_DEL,  KC_END,  KC_PGDN,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,                              KC_UP,
        KC_LCTL, KC_LGUI, KC_LALT,                   KC_SPC,                             KC_RALT, MO(1),   KC_APP,  KC_RCTL,            KC_LEFT, KC_DOWN, KC_RGHT
    ),
    */
    [_KL] = LAYOUT(
        KC_ESC,            KC_F1,         KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,    KC_F12,             KC_PSCR, KC_SLCK, KC_PAUS,
        KC_GRV,            KC_1,          KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,   KC_EQL,  KC_BSPC,   KC_INS,  KC_HOME, KC_PGUP,
        KC_TAB,            KC_Q,          KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC,   KC_RBRC, KC_BSLS,   KC_DEL,  KC_END,  KC_PGDN,
        KC_CAPS,           KC_A,          KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,   KC_ENT,
        KC_LSFT,           KC_Z,          KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_SFTENT,                             KC_UP,
        KC_LCTRL, TD(TD_GUI_ML), KC_LALT,                   KC_SPC,                             KC_RALT, MO(_FL), TD(TD_APP_LCSA),   KC_RCTL,            KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [_FL] = LAYOUT(
        TG(_GL), DM_PLY1, DM_PLY2, _______,  _______, DM_REC1, DM_REC2, _______,  _______,  DM_RSTP, _______, KC_WAKE, KC_SLEP,            KC_MUTE, TG(_X), _______,
        _______, RGB_M_P, _______, _______,  _______, _______, _______, _______,  _______,  ROUT_FM, ROUT_TG, ROUT_VD, ROUT_VI, _______,   KC_MSTP, KC_MPLY, KC_VOLU,
        _______, RGB_SPD, RGB_VAI, RGB_SPI,  RGB_HUI, RGB_SAI, _______, U_T_AUTO, U_T_AGCR, _______, _______, _______, _______, _______,   KC_MPRV, KC_MNXT, KC_VOLD,
        _______, RGB_RMOD,RGB_VAD, RGB_MOD,  RGB_HUD, RGB_SAD, _______, _______,  _______,  _______, _______, _______, _______,
        _______, RGB_TOG, _______, COPY_ALL, _______, MD_BOOT, TG_NKRO, _______,  _______,  _______, _______, _______,                              KC_BRIU,
        _______, _______, _______,                    _______,                              _______, _______, _______, _______,            _______, KC_BRID, _______
    ),
    [_ML] = LAYOUT(
        _______, KC_ACL0,       KC_ACL1, KC_ACL2, _______, _______, _______, _______, _______, _______, _______, _______, _______,            _______, _______, _______,
        _______, KC_BTN4,       KC_BTN3, KC_BTN5, _______, _______, _______, _______, _______, _______, KC_PSLS, KC_PAST, KC_PMNS, _______,   _______, _______, _______,
        _______, KC_BTN1,       KC_MS_U, KC_BTN2, KC_WH_U, _______, _______, _______, _______, KC_KP_7, KC_KP_8, KC_KP_9, KC_PPLS, _______,   _______, _______, _______,
        _______, KC_MS_L,       KC_MS_D, KC_MS_R, KC_WH_D, _______, _______, _______, _______, KC_KP_4, KC_KP_5, KC_KP_6, KC_PENT,
        _______, _______,       _______, SEL_CPY, _______, _______, TG_NKRO, _______, _______, KC_KP_1, KC_KP_2, KC_KP_3,                              _______,
        _______, TD(TD_GUI_ML), _______,                   KC_KP_0,                            _______, _______, _______, _______,            _______, _______, _______
    ),
    [_GL] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,            _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                              _______,
        _______, XXXXXXX, _______,                   _______,                            _______, _______, _______, _______,            _______, _______, _______
    ),
    [_LCSA] = LAYOUT(
        _______, MEH(KC_F1), MEH(KC_F2), MEH(KC_F3), MEH(KC_F4), MEH(KC_F5), MEH(KC_F6), MEH(KC_F7), MEH(KC_F8),   MEH(KC_F9),   MEH(KC_F10),  MEH(KC_F11),    MEH(KC_F12),                  MEH(KC_PSCR), MEH(KC_SLCK), MEH(KC_PAUS),
        _______, MEH(KC_1),  MEH(KC_2),  MEH(KC_3),  MEH(KC_4),  MEH(KC_5),  MEH(KC_6),  MEH(KC_7),  MEH(KC_8),    MEH(KC_9),    MEH(KC_0),    MEH(KC_MINS),   MEH(KC_EQL),  MEH(KC_BSPC),   MEH(KC_INS),  MEH(KC_HOME), MEH(KC_PGUP),
        _______, MEH(KC_Q),  MEH(KC_W),  MEH(KC_E),  MEH(KC_R),  MEH(KC_T),  MEH(KC_Y),  MEH(KC_U),  MEH(KC_I),    MEH(KC_O),    MEH(KC_P),    MEH(KC_LBRC),   MEH(KC_RBRC), MEH(KC_BSLS),   MEH(KC_DEL),  MEH(KC_END),  MEH(KC_PGDN),
        _______, MEH(KC_A),  MEH(KC_S),  MEH(KC_D),  MEH(KC_F),  MEH(KC_G),  MEH(KC_H),  MEH(KC_J),  MEH(KC_K),    MEH(KC_L),    MEH(KC_SCLN), MEH(KC_QUOT),   MEH(KC_ENT),
        _______, MEH(KC_Z),  MEH(KC_X),  MEH(KC_C),  MEH(KC_V),  MEH(KC_B),  MEH(KC_N),  MEH(KC_M),  MEH(KC_COMM), MEH(KC_DOT),  MEH(KC_SLSH), MEH(KC_SFTENT),                                             MEH(KC_UP),
        _______, _______,    _______,                MEH(KC_SPC),                                    _______,      _______,      TD(TD_APP_LCSA),      _______,                              MEH(KC_LEFT), MEH(KC_DOWN), MEH(KC_RGHT)
    ),
    [_X] = LAYOUT(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,            XXXXXXX, _______, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                              XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX,                            XXXXXXX, _______, XXXXXXX, XXXXXXX,            XXXXXXX, XXXXXXX, XXXXXXX
    ),
    /*
    [X] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,            _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, TG_NKRO, _______, _______, _______, _______, _______,                              _______,
        _______, _______, _______,                   _______,                            _______, _______, _______, _______,            _______, _______, _______
    ),
    [RGB] = LAYOUT(
        ESC: 0,   F1: 1,    F2: 2,    F3: 3,    F4: 4,    F5: 5,    F6: 6,    F7: 7,    F8: 8,    F9: 9,    F10: 10,  F11: 11,  F12: 12,            PSCR: 13, SLCK: 14, PAUS: 15,
        GRV: 16,  1: 17,    2: 18,    3: 19,    4: 20,    5: 21,    6: 22,    7: 23,    8: 24,    9: 25,    0: 26,    MINS: 27, EQL: 28,  BSPC: 29, INS: 30,  HOME: 31, PGUP: 32,
        TAB: 33,  Q: 34,    W: 35,    E: 36,    R: 37,    T: 38,    Y: 39,    U: 40,    I: 41,    O: 42,    P: 43,    LBRC: 44, RBRC: 45, BSLS: 46, DEL: 47,  END: 48,  PGDN: 49,
        CAPS: 50, A: 51,    S: 52,    D: 53,    F: 54,    G: 55,    H: 56,    J: 57,    K: 58,    L: 59,    SCLN: 60, QUOT: 61, ENT: 62,
        LSFT: 63, Z: 64,    X: 65,    C: 66,    V: 67,    B: 68,    N: 69,    M: 70,    COMM: 71, DOT: 72,  SLSH: 73, RSFT: 74,                               UP: 75,
        LCTL: 76, LGUI: 77, LALT: 78,                   SPC: 79,                                  RALT: 80, Fn: 81,   APP: 82,  RCTL: 83,           LEFT: 84, DOWN: 85, RGHT: 86
    ),
	[MATRIX] = LAYOUT(
	    0,       1,       2,       3,       4,       5,       6,       7,       8,       9,       10,      11,      12,                 13,      14,      15,
	    16,      17,      18,      19,      20,      21,      22,      23,      24,      25,      26,      27,      28,      29,        30,      31,      32,
	    33,      34,      35,      36,      37,      38,      39,      40,      41,      42,      43,      44,      45,      46,        47,      48,      49,
	    50,      51,      52,      53,      54,      55,      56,      57,      58,      59,      60,      61,      62,
	    63,      64,      65,      66,      67,      68,      69,      70,      71,      72,      73,      74,                                   75,
	    76,      77,      78,                        79,                                 80,      81,      82,      83,                 84,      85,      86
	),
    */
};

// Runs just one time when the keyboard initializes.
void matrix_init_user(void) {
    // Enable or disable debugging
    debug_enable=true;
    debug_matrix=true;
    debug_keyboard=true;
    debug_mouse=true;

    idle_second_counter = 0;
    rgb_time_out_seconds = RGB_DEFAULT_TIME_OUT;
    rgb_time_out_enable = true;
    rgb_enabled_flag = true;
    rgb_time_out_user_value = true;
    rgb_time_out_fast_mode_enabled = false;
    rgb_time_out_saved_flag = rgb_matrix_get_flags();
};

// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {
    if(rgb_time_out_enable && rgb_enabled_flag) {
        if (timer_elapsed(idle_timer) > MILLISECONDS_IN_SECOND) {
            idle_second_counter++;
            idle_timer = timer_read();
        }

        if (idle_second_counter >= rgb_time_out_seconds) {
            rgb_time_out_saved_flag = rgb_matrix_get_flags();
            rgb_matrix_set_flags(LED_FLAG_NONE);
            rgb_matrix_disable_noeeprom();
            rgb_enabled_flag = false;
            idle_second_counter = 0;
        }
    }
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint32_t key_timer;

    if (rgb_time_out_enable) {
        idle_timer = timer_read();
        // Reset the seconds counter. Without this, something like press> leave x seconds> press, would be x seconds on the effective counter not 0 as it should.
        idle_second_counter = 0;
        if (!rgb_enabled_flag) {
            rgb_matrix_enable_noeeprom();
            rgb_matrix_set_flags(rgb_time_out_saved_flag);
            rgb_enabled_flag = true;
        }
    }

    switch (keycode) {
        case U_T_AUTO:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_extra_manual, "USB extra port manual mode");
            }
            return false;
        case U_T_AGCR:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_gcr_auto, "USB GCR auto mode");
            }
            return false;
        case DBG_TOG:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_enable, "Debug mode");
            }
            return false;
        case DBG_MTRX:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_matrix, "Debug matrix");
            }
            return false;
        case DBG_KBD:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_keyboard, "Debug keyboard");
            }
            return false;
        case DBG_MOU:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_mouse, "Debug mouse");
            }
            return false;
        case MD_BOOT:
            if (record->event.pressed) {
                key_timer = timer_read32();
            } else {
                if (timer_elapsed32(key_timer) >= 500) {
                    reset_keyboard();
                }
            }
            return false;
        case RGB_TOG:
            if (record->event.pressed) {
                rgb_time_out_enable = rgb_time_out_user_value;
                switch (rgb_matrix_get_flags()) {
                    case LED_FLAG_ALL: {
                        rgb_matrix_set_flags(LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER);
                        rgb_matrix_set_color_all(0, 0, 0);
                    }
                    break;
                    case LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER: {
                        rgb_matrix_set_flags(LED_FLAG_UNDERGLOW);
                        rgb_matrix_set_color_all(0, 0, 0);
                    }
                    break;
                    case LED_FLAG_UNDERGLOW: {
                        // This line is for LED idle timer. It disables the toggle so you can turn off LED completely if you like
                        rgb_time_out_enable = false;
                        rgb_matrix_set_flags(LED_FLAG_NONE);
                        rgb_matrix_disable_noeeprom();
                    }
                    break;
                    default: {
                        rgb_matrix_set_flags(LED_FLAG_ALL);
                        rgb_matrix_enable_noeeprom();
                    }
                    break;
                }
            }
            return false;
        // ======================================================== CUSTOM KEYCOADS BELOW ========================================================
        case COPY_ALL:
            if (record->event.pressed) {
                // Selects all and text and copy
                SEND_STRING(SS_LCTRL("ac"));
            }
            return false;
        case SEL_CPY:
            // Select word under cursor and copy. Double mouse click then ctrl+c
            if (record->event.pressed) {
                tap_code16(KC_BTN1);
                tap_code16(KC_BTN1);
                tap_code16(C(KC_C));
            }
            return false;
        case ROUT_TG:
            // Toggle idle LED timeout on or off
            if (record->event.pressed) {
                rgb_time_out_enable = !rgb_time_out_enable;
                rgb_time_out_user_value = rgb_time_out_enable;
            }
            return false;
        case ROUT_VI:
            // Increase idle LED timeout value in seconds
            if (record->event.pressed) {
                // Only increase if current value is lower than RGB_TIME_OUT_MAX. Don't care what value the result will be
                // Modity RGB_TIME_OUT_STEP for bigger or smaller increments
                if (!rgb_time_out_fast_mode_enabled && rgb_time_out_seconds <= RGB_TIME_OUT_MAX) {
                    rgb_time_out_seconds += RGB_TIME_OUT_STEP;
                }
            }
            return false;
        case ROUT_VD:
                // Decrease idle LED timeout value in seconds
            if (record->event.pressed) {
                // Only decrease if current value is higher than minimum value and the result is larger than zero
                // Modity RGB_TIME_OUT_STEP for bigger or smaller decrements
                if (!rgb_time_out_fast_mode_enabled && rgb_time_out_seconds > RGB_TIME_OUT_MIN) {
                    rgb_time_out_seconds -= RGB_TIME_OUT_STEP;
                }
            }
            return false;
        case ROUT_FM:
            if (record->event.pressed) {
                if (rgb_time_out_fast_mode_enabled) {
                    rgb_time_out_seconds = rgb_time_out_saved_seconds;
                } else {
                    rgb_time_out_saved_seconds = rgb_time_out_seconds;
                    rgb_time_out_seconds = RGB_FAST_MODE_TIME_OUT;
                }
                rgb_time_out_fast_mode_enabled = !rgb_time_out_fast_mode_enabled;
            }
            return false;
        default:
            return true; //Process all other keycodes normally
    }
}

void rgb_matrix_indicators_user(void) {
    /*
    [RGB] = LAYOUT(
        ESC: 0,   F1: 1,    F2: 2,    F3: 3,    F4: 4,    F5: 5,    F6: 6,    F7: 7,    F8: 8,    F9: 9,    F10: 10,  F11: 11,  F12: 12,            PSCR: 13, SLCK: 14, PAUS: 15,
        GRV: 16,  1: 17,    2: 18,    3: 19,    4: 20,    5: 21,    6: 22,    7: 23,    8: 24,    9: 25,    0: 26,    MINS: 27, EQL: 28,  BSPC: 29, INS: 30,  HOME: 31, PGUP: 32,
        TAB: 33,  Q: 34,    W: 35,    E: 36,    R: 37,    T: 38,    Y: 39,    U: 40,    I: 41,    O: 42,    P: 43,    LBRC: 44, RBRC: 45, BSLS: 46, DEL: 47,  END: 48,  PGDN: 49,
        CAPS: 50, A: 51,    S: 52,    D: 53,    F: 54,    G: 55,    H: 56,    J: 57,    K: 58,    L: 59,    SCLN: 60, QUOT: 61, ENT: 62,
        LSFT: 63, Z: 64,    X: 65,    C: 66,    V: 67,    B: 68,    N: 69,    M: 70,    COMM: 71, DOT: 72,  SLSH: 73, RSFT: 74,                               UP: 75,
        LCTL: 76, LGUI: 77, LALT: 78,                   SPC: 79,                                  RALT: 80, Fn: 81,   APP: 82,  RCTL: 83,           LEFT: 84, DOWN: 85, RGHT: 86
    )
    */
	if (!g_suspend_state && rgb_matrix_config.enable) {
		switch (biton32(layer_state)) {
            // case _KL: {
            //     /*
            //     [_KL] = LAYOUT(
            //         KC_ESC,            KC_F1,         KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,    KC_F12,             KC_PSCR, KC_SLCK, KC_PAUS,
            //         KC_GRV,            KC_1,          KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,   KC_EQL,  KC_BSPC,   KC_INS,  KC_HOME, KC_PGUP,
            //         KC_TAB,            KC_Q,          KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC,   KC_RBRC, KC_BSLS,   KC_DEL,  KC_END,  KC_PGDN,
            //         KC_CAPS,           KC_A,          KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,   KC_ENT,
            //         KC_LSFT,           KC_Z,          KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_SFTENT,                             KC_UP,
            //         TD(TD_LCTRL_TERM), TD(TD_GUI_ML), KC_LALT,                   KC_SPC,                             KC_RALT, MO(_FL), KC_APP,   KC_RCTL,            KC_LEFT, KC_DOWN, KC_RGHT
            //     )
            //     */

            //     // Function key. Color code #ffc100
            //     rgb_matrix_set_color(81, 0xff, 0xc1, 0x00);
            //     // Basic key cluster set #c07612
            //     rgb_matrix_set_color(0, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(13, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(14, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(15, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(16, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(27, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(28, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(29, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(30, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(31, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(32, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(33, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(56, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(57, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(58, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(59, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(60, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(61, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(62, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(68, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(69, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(70, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(71, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(72, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(73, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(74, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(77, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(80, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(82, 0xc0, 0x76, 0x12);
            //     rgb_matrix_set_color(83, 0xc0, 0x76, 0x12);
            //     // RBG Control Keys. Color code #2790c2
            //     rgb_matrix_set_color(1, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(2, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(3, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(4, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(5, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(6, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(7, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(8, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(9, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(10, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(11, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(12, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(17, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(18, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(19, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(20, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(21, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(22, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(23, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(24, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(25, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(26, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(34, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(35, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(36, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(37, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(38, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(39, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(40, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(41, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(42, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(43, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(44, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(45, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(46, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(47, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(48, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(49, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(50, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(51, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(52, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(53, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(54, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(55, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(63, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(64, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(65, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(66, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(67, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(75, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(76, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(78, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(79, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(84, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(85, 0x27, 0x90, 0xc2);
            //     rgb_matrix_set_color(86, 0x27, 0x90, 0xc2);
            //     // rim LED's blue
            //     // rim LED's orange
            //     // rim LED's white
            //     rgb_matrix_set_color(87, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(88, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(89, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(90, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(91, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(92, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(93, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(94, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(95, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(96, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(97, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(98, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(99, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(100, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(101, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(102, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(103, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(104, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(105, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(106, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(107, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(108, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(109, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(110, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(111, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(112, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(113, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(114, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(115, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(116, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(117, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(118, 0xff, 0xff, 0xff);
            //     rgb_matrix_set_color(119, 0xff, 0xff, 0xff);
            // } break;
            case _FL: {
                /*
                [_FL] = LAYOUT(
                _______, DM_PLY1, DM_PLY2, _______,  _______, DM_REC1, DM_REC2, _______,  _______,  DM_RSTP, _______, KC_WAKE, KC_SLEP,            KC_MUTE, _______, _______,
                _______, RGB_M_P, _______, _______,  _______, _______, _______, _______,  _______,  ROUT_FM, ROUT_TG, ROUT_VD, ROUT_VI, _______,   KC_MSTP, KC_MPLY, KC_VOLU,
                _______, RGB_SPD, RGB_VAI, RGB_SPI,  RGB_HUI, RGB_SAI, _______, U_T_AUTO, U_T_AGCR, _______, _______, _______, _______, _______,   KC_MPRV, KC_MNXT, KC_VOLD,
                _______, RGB_RMOD,RGB_VAD, RGB_MOD,  RGB_HUD, RGB_SAD, _______, _______,  _______,  _______, _______, _______, _______,
                _______, RGB_TOG, _______, COPY_ALL, _______, MD_BOOT, TG_NKRO, _______,  _______,  _______, _______, _______,                              KC_BRIU,
                _______, _______, _______,                    _______,                              _______, _______, _______, _______,            _______, KC_BRID, _______
                )
                */

                // Function key. Color code #ffc100
                rgb_matrix_set_color(81, 0xff, 0xc1, 0x00);
                // Sleep and Wake. Color code #aeb0b0
                rgb_matrix_set_color(11, 0xae, 0xb0, 0xb0);
                rgb_matrix_set_color(12, 0xae, 0xb0, 0xb0);
                // Power Control Keys. Color code #cb2f2a
                rgb_matrix_set_color(40, 0xcb, 0x2f, 0x2a);
                rgb_matrix_set_color(41, 0xcb, 0x2f, 0x2a);
                rgb_matrix_set_color(74, 0xcb, 0x2f, 0x2a);
                rgb_matrix_set_color(83, 0xcb, 0x2f, 0x2a);
                // Keyboard Control Keys. Color code #f4791e
                rgb_matrix_set_color(68, 0xf4, 0x79, 0x1e);
                rgb_matrix_set_color(69, 0xf4, 0x79, 0x1e);
                // Copy All Key. Color Code #45b866
                rgb_matrix_set_color(66, 0x45, 0xb8, 0x66);
                // Brightness Keys. Color code #aeb0b0
                rgb_matrix_set_color(75, 0xae, 0xb0, 0xb0);
                rgb_matrix_set_color(85, 0xae, 0xb0, 0xb0);
                // RGB Timeout Keys. Color code #34a4b9
                rgb_matrix_set_color(25, 0x34, 0xa3, 0xb9);
                rgb_matrix_set_color(26, 0x34, 0xa3, 0xb9);
                rgb_matrix_set_color(27, 0x34, 0xa3, 0xb9);
                rgb_matrix_set_color(28, 0x34, 0xa3, 0xb9);
                // Media Keys. Color code #51c0dd
                rgb_matrix_set_color(13, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(30, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(31, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(32, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(47, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(48, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(49, 0x51, 0xc0, 0xdd);
                // RBG Control Keys. Color code #2790c2
                rgb_matrix_set_color(17, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(34, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(35, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(36, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(37, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(38, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(51, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(52, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(53, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(54, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(55, 0x27, 0x90, 0xc2);
                rgb_matrix_set_color(64, 0x27, 0x90, 0xc2);
                // Dynamic Macro Keys. Color code #45b866
                rgb_matrix_set_color(1, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(2, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(5, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(6, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(9, 0x45, 0xb8, 0x66);
                // Underglow key. Color code #2f07a6
                rgb_matrix_set_color(87, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(88, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(89, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(90, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(91, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(92, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(93, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(94, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(95, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(96, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(97, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(98, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(99, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(100, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(101, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(102, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(103, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(104, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(105, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(106, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(107, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(108, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(109, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(110, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(111, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(112, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(113, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(114, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(115, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(116, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(117, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(118, 0x2f, 0x07, 0xa6);
                rgb_matrix_set_color(119, 0x2f, 0x07, 0xa6);
            }
            break;
            case _ML: {
                /*
                [_ML] = LAYOUT(
                    _______, KC_ACL0,       KC_ACL1, KC_ACL2, _______, _______, _______, _______, _______, _______, _______, _______, _______,            _______, _______, _______,
                    _______, KC_BTN4,       KC_BTN3, KC_BTN5, _______, _______, _______, _______, _______, _______, KC_PSLS, KC_PAST, KC_PMNS, _______,   _______, _______, _______,
                    _______, KC_BTN1,       KC_MS_U, KC_BTN2, KC_WH_U, _______, _______, _______, _______, KC_KP_7, KC_KP_8, KC_KP_9, KC_PPLS, _______,   _______, _______, _______,
                    _______, KC_MS_L,       KC_MS_D, KC_MS_R, KC_WH_D, _______, _______, _______, _______, KC_KP_4, KC_KP_5, KC_KP_6, KC_PENT,
                    _______, _______,       _______, SEL_CPY, _______, _______, TG_NKRO, _______, _______, KC_KP_1, KC_KP_2, KC_KP_3,                              _______,
                    _______, TD(TD_GUI_ML), _______,                   KC_KP_0,                            _______, _______, TG(_ML), _______,            _______, _______, _______
                ),
                */

                // Mouse movement keys. Color code #51c0dd
                rgb_matrix_set_color(35, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(51, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(52, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(53, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(54, 0x51, 0xc0, 0xdd);
                rgb_matrix_set_color(37, 0x51, 0xc0, 0xdd);
                // Mouse Button keys. Color code #009bca
                rgb_matrix_set_color(17, 0x00, 0x9b, 0xca);
                rgb_matrix_set_color(18, 0x00, 0x9b, 0xca);
                rgb_matrix_set_color(19, 0x00, 0x9b, 0xca);
                rgb_matrix_set_color(34, 0x00, 0x9b, 0xca);
                rgb_matrix_set_color(36, 0x00, 0x9b, 0xca);
                // Mouse acceleration control. Color code #5dcde3
                rgb_matrix_set_color(1, 0x5d, 0xcd, 0xe3);
                rgb_matrix_set_color(2, 0x5d, 0xcd, 0xe3);
                rgb_matrix_set_color(3, 0x5d, 0xcd, 0xe3);
                // Layer toggle keys. Color code #ffc100
                rgb_matrix_set_color(77, 0xff, 0xc1, 0x00);
                rgb_matrix_set_color(82, 0xff, 0xc1, 0x00);
                // Select copy key. Color code #45b866
                rgb_matrix_set_color(66, 0x45, 0xb8, 0x66);
                // Numpad number keys #45b866
                rgb_matrix_set_color(42, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(43, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(44, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(59, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(60, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(61, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(72, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(73, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(74, 0x45, 0xb8, 0x66);
                rgb_matrix_set_color(79, 0x45, 0xb8, 0x66);
                // Numpad arithmatic keys #249e46
                rgb_matrix_set_color(26, 0x24, 0x9e, 0x46);
                rgb_matrix_set_color(27, 0x24, 0x9e, 0x46);
                rgb_matrix_set_color(28, 0x24, 0x9e, 0x46);
                rgb_matrix_set_color(45, 0x24, 0x9e, 0x46);
                rgb_matrix_set_color(62, 0x24, 0x9e, 0x46);
                // NKRO key. Color code #f4791e
                rgb_matrix_set_color(69, 0xf4, 0x79, 0x1e);
                // Underglow key. Color code #40f2b1
                rgb_matrix_set_color(87, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(88, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(89, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(90, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(91, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(92, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(93, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(94, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(95, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(96, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(97, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(98, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(99, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(100, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(101, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(102, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(103, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(104, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(105, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(106, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(107, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(108, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(109, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(110, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(111, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(112, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(113, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(114, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(115, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(116, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(117, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(118, 0x40, 0xf2, 0xb1);
                rgb_matrix_set_color(119, 0x40, 0xf2, 0xb1);
            }
            break;
            case _GL: {
                /*
                [_GL] = LAYOUT(
                    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,            _______, _______, _______,
                    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______,
                    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______,
                    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
                    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                              _______,
                    _______, XXXXXXX, _______,                   _______,                            _______, _______, _______, _______,            _______, _______, _______
                ),
                */
                // GUI off. Color code #000000
                rgb_matrix_set_color(77, 0x00, 0x00, 0x00);
                // WASD key. Color code #ff2600
                rgb_matrix_set_color(35, 0xff, 0x26, 0x00);
                rgb_matrix_set_color(51, 0xff, 0x26, 0x00);
                rgb_matrix_set_color(52, 0xff, 0x26, 0x00);
                rgb_matrix_set_color(53, 0xff, 0x26, 0x00);
                // QERFG. Color code #2b60ab
                rgb_matrix_set_color(34, 0x2b, 0x60, 0xab);
                rgb_matrix_set_color(36, 0x2b, 0x60, 0xab);
                rgb_matrix_set_color(37, 0x2b, 0x60, 0xab);
                rgb_matrix_set_color(54, 0x2b, 0x60, 0xab);
                rgb_matrix_set_color(55, 0x2b, 0x60, 0xab);
                // ZXCV. Color code #602bab
                rgb_matrix_set_color(64, 0x60, 0x2b, 0xab);
                rgb_matrix_set_color(65, 0x60, 0x2b, 0xab);
                rgb_matrix_set_color(66, 0x60, 0x2b, 0xab);
                rgb_matrix_set_color(67, 0x60, 0x2b, 0xab);
                // UIOJKLM. Color code #a7ddd9
                rgb_matrix_set_color(40, 0xa7, 0xdd, 0xd9);
                rgb_matrix_set_color(41, 0xa7, 0xdd, 0xd9);
                rgb_matrix_set_color(42, 0xa7, 0xdd, 0xd9);
                rgb_matrix_set_color(57, 0xa7, 0xdd, 0xd9);
                rgb_matrix_set_color(58, 0xa7, 0xdd, 0xd9);
                rgb_matrix_set_color(59, 0xa7, 0xdd, 0xd9);
                rgb_matrix_set_color(70, 0xa7, 0xdd, 0xd9);
                // 123456. Color code #eeff2e
                rgb_matrix_set_color(17, 0xee, 0xff, 0x2e);
                rgb_matrix_set_color(18, 0xee, 0xff, 0x2e);
                rgb_matrix_set_color(19, 0xee, 0xff, 0x2e);
                rgb_matrix_set_color(20, 0xee, 0xff, 0x2e);
                rgb_matrix_set_color(21, 0xee, 0xff, 0x2e);
                rgb_matrix_set_color(22, 0xee, 0xff, 0x2e);
                // Tab, Shift, Ctrl, Alt, Space #ffab2e
                rgb_matrix_set_color(69, 0xff, 0xab, 0x2e);
                rgb_matrix_set_color(69, 0xff, 0xab, 0x2e);
                rgb_matrix_set_color(69, 0xff, 0xab, 0x2e);
                rgb_matrix_set_color(69, 0xff, 0xab, 0x2e);
                rgb_matrix_set_color(69, 0xff, 0xab, 0x2e);
                // Underglow key. Color code #07d51f
                rgb_matrix_set_color(87, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(88, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(89, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(90, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(91, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(92, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(93, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(94, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(95, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(96, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(97, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(98, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(99, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(100, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(101, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(102, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(103, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(104, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(105, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(106, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(107, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(108, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(109, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(110, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(111, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(112, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(113, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(114, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(115, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(116, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(117, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(118, 0x07, 0xd5, 0x1f);
                rgb_matrix_set_color(119, 0x07, 0xd5, 0x1f);
            } break;
            case _LCSA: {
                /*
                [_ML] = LAYOUT(
                    _______, KC_ACL0,       KC_ACL1, KC_ACL2, _______, _______, _______, _______, _______, _______, _______, _______, _______,            _______, _______, _______,
                    _______, KC_BTN4,       KC_BTN3, KC_BTN5, _______, _______, _______, _______, _______, _______, KC_PSLS, KC_PAST, KC_PMNS, _______,   _______, _______, _______,
                    _______, KC_BTN1,       KC_MS_U, KC_BTN2, KC_WH_U, _______, _______, _______, _______, KC_KP_7, KC_KP_8, KC_KP_9, KC_PPLS, _______,   _______, _______, _______,
                    _______, KC_MS_L,       KC_MS_D, KC_MS_R, KC_WH_D, _______, _______, _______, _______, KC_KP_4, KC_KP_5, KC_KP_6, KC_PENT,
                    _______, _______,       _______, SEL_CPY, _______, _______, TG_NKRO, _______, _______, KC_KP_1, KC_KP_2, KC_KP_3,                              _______,
                    _______, TD(TD_GUI_ML), _______,                   KC_KP_0,                            _______, _______, TG(_ML), _______,            _______, _______, _______
                ),
                */

                // Underglow key. Color code #e1be23
                rgb_matrix_set_color(87, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(88, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(89, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(90, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(91, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(92, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(93, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(94, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(95, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(96, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(97, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(98, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(99, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(100, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(101, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(102, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(103, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(104, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(105, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(106, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(107, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(108, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(109, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(110, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(111, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(112, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(113, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(114, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(115, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(116, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(117, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(118, 0xe1, 0xbe, 0x23);
                rgb_matrix_set_color(119, 0xe1, 0xbe, 0x23);
            } break;
            case _X: {
                /*
                [_ML] = LAYOUT(
                    _______, KC_ACL0,       KC_ACL1, KC_ACL2, _______, _______, _______, _______, _______, _______, _______, _______, _______,            _______, _______, _______,
                    _______, KC_BTN4,       KC_BTN3, KC_BTN5, _______, _______, _______, _______, _______, _______, KC_PSLS, KC_PAST, KC_PMNS, _______,   _______, _______, _______,
                    _______, KC_BTN1,       KC_MS_U, KC_BTN2, KC_WH_U, _______, _______, _______, _______, KC_KP_7, KC_KP_8, KC_KP_9, KC_PPLS, _______,   _______, _______, _______,
                    _______, KC_MS_L,       KC_MS_D, KC_MS_R, KC_WH_D, _______, _______, _______, _______, KC_KP_4, KC_KP_5, KC_KP_6, KC_PENT,
                    _______, _______,       _______, SEL_CPY, _______, _______, TG_NKRO, _______, _______, KC_KP_1, KC_KP_2, KC_KP_3,                              _______,
                    _______, TD(TD_GUI_ML), _______,                   KC_KP_0,                            _______, _______, TG(_ML), _______,            _______, _______, _______
                ),
                */

                // Underglow key. Color code #383838
                rgb_matrix_set_color(87, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(88, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(89, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(90, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(91, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(92, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(93, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(94, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(95, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(96, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(97, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(98, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(99, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(100, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(101, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(102, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(103, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(104, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(105, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(106, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(107, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(108, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(109, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(110, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(111, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(112, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(113, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(114, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(115, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(116, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(117, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(118, 0x38, 0x38, 0x38);
                rgb_matrix_set_color(119, 0x38, 0x38, 0x38);
            } break;
        }
	}
}
