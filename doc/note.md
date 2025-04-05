
Flash Esp32c3
python -m esptool --chip esp32c3 --before default_reset --after hard_reset write_flash 0x10000 .\firmware.bin

Flash Esp32s3
python -m esptool --chip esp32s3 --before default_reset --after hard_reset write_flash 0x10000 .\firmware.bin