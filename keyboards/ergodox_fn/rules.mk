# MCU name
MCU = atmega32u4

# Bootloader selection
BOOTLOADER = halfkay

# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = no   # Enable Bootmagic Lite
MOUSEKEY_ENABLE  = yes  # Mouse keys
EXTRAKEY_ENABLE  = yes  # Audio control and System control
CONSOLE_ENABLE   = no   # Console for debug
COMMAND_ENABLE   = no   # Commands for debug and configuration
CUSTOM_MATRIX    = lite # Custom matrix file for the ErgoDox EZ
NKRO_ENABLE      = yes           # Enable N-Key Rollover
UNICODE_ENABLE   = no  # Unicode
SWAP_HANDS_ENABLE= no   # Allow swapping hands of keyboard

RGB_MATRIX_ENABLE = no # enable later
RGB_MATRIX_DRIVER = IS31FL3731
DEBOUNCE_TYPE = sym_eager_pr

# project specific files
SRC += matrix.c
QUANTUM_LIB_SRC += i2c_master.c

LAYOUTS = ergodox

# Disable unsupported hardware
AUDIO_SUPPORTED = no
BACKLIGHT_SUPPORTED = no

MOUSE_SHARED_EP = no
