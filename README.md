# PM2040 EEPROM Manager
An EEPROM backup and restore program for the PM2040 flash cart.

## Overview
This program is intended only to be used in combination with the [PM2040 flash cart](https://github.com/zwenergy/PM2040) and a suitable PM2040 FW.
A ready to flash FW for the PM2040 flash cart containing this EEPROM manager is available at the [PM2040 repo](https://github.com/zwenergy/PM2040).

**Note that the simple "ROM-less base FW" for regular ROMs does NOT work, as the EEPROM manager expects certain PM2040-specific behaviours, which is not baked in to the regular FW.**

## Functionality
The core idea of this PM2040 EEPROM manager is to offer a way to back up or restore the EEPROM of the Pokemon Mini handheld.
Save files are not stored on the carts, but rather on the EEPROM of the handheld.
The Pokemon Mini can store up to 6 save files, but with 10 games overall published + some homebrew may also use the EEPROM, you can run out of save game memory.

The PM2040 EEPROM manager offers 3 EEPROM backup slots which are stored on the PM2040 cart's Flash memory.
You can backup the entire current EEPROM content to one of the 3 slots on the cart, and also restore from one of these slots to the handheld's local EEPROM.

Backing up or restoring the EEPROM takes roughly somewhere between 40 secound and 1 minute.
The display will show "...done" once the operation completed.

## Further managing the EEPROMs via USB
An alternative "EEPROM USB drive" firmware for the PM2040 flash cart is available at the [repo here](https://github.com/zwenergy/PM2040-EEPROM-USB-Drive).
When flashing this firmware to the PM2040, it will function as a USB drive and display the 3 EEPROM slots from the cart.
This way, you can easily backup all save files via USB to a computer.
