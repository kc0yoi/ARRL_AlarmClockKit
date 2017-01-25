# Alarm Clock Usage Instructions

## Setting the Time
1. Move the ALARM switch to the off position (down)
2. Adjust time using the UP and DOWN buttons
    * The decimal point in the middle of the display will be illuminated for PM times in 12 hour mode
3. Press both UP and DOWN buttons to display minutes and seconds
    * Due to a limitation with the PIC's hardware timer the seconds display counts by twos
4. Hold UP, DOWN, and SNOOZE buttons to reset seconds to zero

## Setting the Alarm
1. Move the ALARM switch to the on position (up)
2. Adjust the alarm time using the UP and DOWN buttons
    * The display will blink slowly to indicate that the alarm time is being shown
    * After a short time the display will switch back to time of day and stop blinking
    * The decimal point in the middle of the display will be illuminated for PM alarm times in 12 hour mode

## Using the Alarm
* The alarm function is enabled and disabled with the ALARM switch
    * When the alarm is enabled it is not possible to adjust system time
* When the time of day matches the alarm time the beeper will sound
    * The beeper will sound for two minutes and then silence itself
    * The beeper may be temporarly silenced for 9 minutes by pressing the SNOOZE button
    * A snoozed alarm can be cancelled by moving the ALARM switch to the off position

## Backup Capacitor
The alarm clock utilizes a "super capacitor" to keep time and alarm setting through a DC power interruption. When
DC power is lost and the barrel jack remains inserted the clock will keep time in a lower power mode (no display)
until DC power is restored or until the capacitor discharge reaches the PIC's brown-out reset voltage threshold.

## 12/24 Hour Option
The alarm clock can be configured to display time and alarm time in 24 hour notation.
To enable 24 hour display the jumper on the back side of the circuit board may be shorted with a wire or solder blob.
The DC power jack should be removed during jumper (re)configuration.

