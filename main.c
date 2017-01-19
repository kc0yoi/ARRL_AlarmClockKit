/*
 * File:   main.c
 * Author: KC0YOI
 *
 * Copyright 2016,2017 Adrian Hill
 *
 * Created on November 10, 2016, 8:16 AM
 */

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ= 32 MHz and CDIV = 1:1)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = OFF      // Clock Switch Enable bit (The NOSC and NDIV bits cannot be changed by user software)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (FSCM timer disabled)

// CONFIG2
#pragma config MCLRE = OFF      // Master Clear Enable bit (MCLR pin function is port defined function)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config LPBOREN = ON     // Low-Power BOR enable bit (ULPBOR enabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config WRT = OFF        // UserNVM self-write protection bits (Write protection off)
#pragma config SCANE = not_available// Scanner Enable bit (Scanner module is not available for use)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (High Voltage on MCLR/Vpp must be used for programming)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (Program Memory code protection disabled)
#pragma config CPD = OFF        // DataNVM code protection bit (Data EEPROM code protection disabled)

#include <xc.h>

#define TICKSECONDS     488L
#define DEBOUNCETIME   (TICKSECONDS / 5)
#define REPEATTIME     (TICKSECONDS / 3)
#define FASTREPEATTIME (TICKSECONDS / 12)
#define BLINKTIME      (TICKSECONDS / 4)
#define FASTADJUST     (4 * TICKSECONDS)
#define DISPLAYTIME    (10 * TICKSECONDS)
#define ALARMTIME      (2 * 60 * TICKSECONDS)
#define SNOOZETIME     (9 * 60 * TICKSECONDS)

#define JMP_24          0x01
#define JMP_CA          0x02
#define JMP_TIMER       0x04
#define JMP_SPARE       0x08

#define BTN_UP          0x10
#define BTN_DOWN        0x20
#define BTN_SNOOZE      0x80

#define bALARM_SW       RA6

#define bBUZZER         RC2
#define bMAIN_DETECT    RC3

#define bDIGIT0         RC4
#define bDIGIT1         RC5
#define bDIGIT2         RC6
#define bDIGIT3         RC7

#define SEG_A           0x80
#define SEG_B           0x01
#define SEG_C           0x08
#define SEG_D           0x20
#define SEG_E           0x40
#define SEG_F           0x02
#define SEG_G           0x04
#define SEG_DP          0x10

#define CHAR0   ( SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F )           // 0
#define CHAR1   ( SEG_B | SEG_C )                                           // 1
#define CHAR2   ( SEG_A | SEG_B | SEG_G | SEG_E | SEG_D )                   // 2
#define CHAR3   ( SEG_A | SEG_B | SEG_G | SEG_C | SEG_D )                   // 3
#define CHAR4   ( SEG_F | SEG_G | SEG_B | SEG_C )                           // 4
#define CHAR5   ( SEG_A | SEG_F | SEG_G | SEG_C | SEG_D )                   // 5
#define CHAR6   ( SEG_A | SEG_F | SEG_E | SEG_D | SEG_C | SEG_G )           // 6
#define CHAR7   ( SEG_A | SEG_B | SEG_C )                                   // 7
#define CHAR8   ( SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G )   // 8
#define CHAR9   ( SEG_G | SEG_F | SEG_A | SEG_B | SEG_C | SEG_D )           // 9
#define CHARA   ( SEG_E | SEG_F | SEG_A | SEG_B | SEG_G | SEG_C )           // A
#define CHARB   ( SEG_F | SEG_E | SEG_D | SEG_C | SEG_G )                   // b
#define CHARC   ( SEG_A | SEG_F | SEG_E | SEG_D )                           // C
#define CHARD   ( SEG_G | SEG_E | SEG_D | SEG_C | SEG_B )                   // d
#define CHARE   ( SEG_A | SEG_F | SEG_G | SEG_E | SEG_D )                   // E
#define CHARF   ( SEG_A | SEG_F | SEG_G | SEG_E )                           // F

// timekeeping variables
unsigned char tick ;
unsigned char newsecond ;
unsigned char seconds ;
unsigned char minutes ;
unsigned char hours ;
unsigned char alarm_minutes ;
unsigned char alarm_hours ;

// misc variables
unsigned char alarmsounding ;

// display variables
unsigned char blink ;
unsigned char blinkstate ;
unsigned char digit ;
unsigned char digitbuffer[4] ;

// input variables
unsigned char strap ;
unsigned char button ;
unsigned short buttoncount ;

// software timer variables
unsigned long alarmtimer ;
unsigned long snoozetimer ;
unsigned short repeattimer ;
unsigned short blinktimer ;
unsigned short displaytimer ;
unsigned short beeptimer ;

unsigned char bcd_adjust_add( unsigned char ) ;
unsigned char bcd_adjust_sub( unsigned char ) ;
unsigned char twelve_hour( unsigned char ) ;

const unsigned char hexlookup[] = 
{
    CHAR0, CHAR1, CHAR2, CHAR3,
    CHAR4, CHAR5, CHAR6, CHAR7,
    CHAR8, CHAR9, CHARA, CHARB,
    CHARC, CHARD, CHARE, CHARF
} ;

void main( void )
{
    unsigned char btemp ;

    // configure ports
    ANSELA = 0 ;
    ANSELB = 0 ;
    ANSELC = 0 ;

    LATA = 0 ;
    LATB = 0 ;
    LATC = 0 ;

    WPUA = 0xFF ;
    WPUB = 0 ;
    WPUC = 0 ;
    WPUE = 0x08 ;

    TRISA = 0xFF ;
    TRISB = 0 ;
    TRISC = 0x08 ;

    // configure primary oscillator, 8 MHz
    OSCFRQ = 0x3 ;

    // configure 8-bit timer0 Fosc/4, 1:16 prescale
    T0CON0 = 0x80 ;
    T0CON1 = 0x44 ;

    // configure secondary oscillator
    SOSCEN = 1 ;

    // configure timer1 (2.0 second) using secondary oscillatory @ 32.768 kHz
    T1CON = 0x07 ;
    T1CLK = 0x06 ;

    // read configuration straps
    strap = PORTA ;
    strap &= (JMP_24 | JMP_CA | JMP_TIMER | JMP_SPARE) ;

    // init globals
    tick = 0 ;
    newsecond = 0 ;
    seconds = 0 ;
    minutes = 0 ;
    hours = 0x12 ;
    alarm_minutes = 0 ;
    alarm_hours = 7 ;

    blink = 0 ;
    blinkstate = 0 ;

    buttoncount = 0 ;

    alarmsounding = 0 ;

    alarmtimer = 0 ;
    snoozetimer = 0 ;
    repeattimer = 0 ;
    blinktimer = 0 ;
    displaytimer = 0 ;
    beeptimer = 0 ;

    // enable interrupts
    TMR0IE = 1 ;
    TMR1IE = 1 ;
    INTCON = 0xC0 ;

    while( 1 )
    {
        if( tick )
        {
            tick = 0 ;

            // update timers
            if( alarmtimer )
                alarmtimer-- ;
            if( snoozetimer )
                snoozetimer-- ;
            if( repeattimer )
                repeattimer-- ;
            if( blinktimer )
                blinktimer-- ;
            if( displaytimer )
                displaytimer-- ;
            if( beeptimer )
                beeptimer-- ;



            // do time-keeping
            if( newsecond )
            {
                seconds += newsecond ;
                newsecond = 0 ;
                seconds = bcd_adjust_add(seconds) ;
            }

            if( seconds == 0x60 )
            {
                seconds = 0 ;
                minutes++ ;
                minutes = bcd_adjust_add(minutes) ;

                if( minutes == 0x60 )
                {
                    minutes = 0 ;
                    hours++ ;
                    hours = bcd_adjust_add(hours) ;

                    if( hours == 0x24 )
                    {
                        hours = 0 ;
                    }
                }
            }



            // poll buttons
            btemp = PORTA ;
            btemp &= ( BTN_UP | BTN_DOWN | BTN_SNOOZE ) ;
            if( btemp == button )
            {
                if( buttoncount < 0xFFFF )
                {
                    buttoncount++ ;
                }
            }
            else
            {
                button = btemp ;
                buttoncount = 0 ;
            }

            

            // update display
            digit++ ;
            digit &= 0x3 ;

            if( strap & JMP_CA )
            {
                // common cathode
                LATC |= 0xF0 ;
                LATB = digitbuffer[digit] ;
            }
            else
            {
                // common anode
                LATC &= 0x0F ;
                LATB = ~digitbuffer[digit] ;
            }

            switch( digit )
            {
                case 0 :
                    btemp = 0x80 ;
                    break ;
                case 1 :
                    btemp = 0x20 ;
                    break ;
                case 2 :
                    btemp = 0x40 ;
                    break ;
                case 3 :
                    btemp = 0x10 ;
                    break ;
                default :
                    break ;
            }

            if( bMAIN_DETECT && blink )
            {
                // only enable display if main power is active
                if( strap & JMP_CA )
                {
                    // common cathode
                    btemp = ~btemp ;
                    LATC &= btemp ;
                }
                else
                {
                    // common anode
                    LATC |= btemp ;
                }
            }
            


            // do alarm function
            if( (bALARM_SW == 0) && (bMAIN_DETECT) )
            {
                if( (alarm_minutes == minutes) && (alarm_hours == hours) && (seconds == 0) )
                {
                    alarmsounding = 1 ;
                    snoozetimer = 0 ;
                    alarmtimer = ALARMTIME ;
                }
                else if( alarmsounding )
                {
                    if( alarmtimer == 0 )
                    {
                        alarmsounding = 0 ;
                    }
                    if( (buttoncount > DEBOUNCETIME) && ((button & BTN_SNOOZE) == 0) )
                    {
                        snoozetimer = 9 * 60 * TICKSECONDS ;
                        alarmsounding = 0 ;
                    }
                }
                else if( snoozetimer == 1 )
                {
                    alarmsounding = 1 ;
                    alarmtimer = ALARMTIME ;
                }
                if( alarmsounding )
                {
                    if( beeptimer == 0 )
                    {
                        bBUZZER = ~ bBUZZER ;
                        beeptimer = TICKSECONDS / 3 ;
                    }
                }
                else
                {
                    bBUZZER = 0 ;
                }
            }
            else
            {
                bBUZZER = 0 ;
                alarmsounding = 0 ;
                alarmtimer = 0 ;
                snoozetimer = 0 ;
            }



            // handle user interface input
            if( (buttoncount > DEBOUNCETIME) && (repeattimer == 0) )
            {
                if( bALARM_SW == 0 )
                {
                    switch( ~button & (BTN_UP | BTN_DOWN) )
                    {
                        case BTN_UP :
                            displaytimer = DISPLAYTIME ;
                            if( buttoncount < FASTADJUST )
                                repeattimer = REPEATTIME ;
                            else
                                repeattimer = FASTREPEATTIME ;
                            alarm_minutes++ ;
                            alarm_minutes = bcd_adjust_add(alarm_minutes) ;

                            if( alarm_minutes == 0x60 )
                            {
                                alarm_minutes = 0 ;
                                alarm_hours++ ;
                                alarm_hours = bcd_adjust_add(alarm_hours) ;

                                if( alarm_hours == 0x24 )
                                {
                                    alarm_hours = 0 ;
                                }
                            }
                            
                            break ;
                        case BTN_DOWN :
                            displaytimer = DISPLAYTIME ;
                            if( buttoncount < FASTADJUST )
                                repeattimer = REPEATTIME ;
                            else
                                repeattimer = FASTREPEATTIME ;
                            alarm_minutes-- ;
                            alarm_minutes = bcd_adjust_sub(alarm_minutes) ;

                            if( alarm_minutes > 0x60 )
                            {
                                alarm_minutes = 0x59 ;
                                alarm_hours-- ;
                                alarm_hours = bcd_adjust_sub(alarm_hours) ;

                                if( alarm_hours > 0x24 )
                                {
                                    alarm_hours = 0x23 ;
                                }
                            }

                            break ;

                        default :
                            break ;
                    }
                }
                else
                {
                    switch( ~button & (BTN_UP | BTN_DOWN | BTN_SNOOZE) )
                    {
                        case BTN_UP :
                            if( buttoncount < FASTADJUST )
                                repeattimer = REPEATTIME ;
                            else
                                repeattimer = FASTREPEATTIME ;
                            minutes++ ;
                            minutes = bcd_adjust_add(minutes) ;

                            if( minutes == 0x60 )
                            {
                                minutes = 0 ;
                                hours++ ;
                                hours = bcd_adjust_add(hours) ;

                                if( hours == 0x24 )
                                {
                                    hours = 0 ;
                                }
                            }

                            break ;

                        case BTN_DOWN :
                            if( buttoncount < FASTADJUST )
                                repeattimer = REPEATTIME ;
                            else
                                repeattimer = FASTREPEATTIME ;
                            minutes-- ;
                            minutes = bcd_adjust_sub(minutes) ;

                            if( minutes > 0x60 )
                            {
                                minutes = 0x59 ;
                                hours-- ;
                                hours = bcd_adjust_sub(hours) ;

                                if( hours > 0x24 )
                                {
                                    hours = 0x23 ;
                                }
                            }

                            break ;
                        case (BTN_UP | BTN_DOWN | BTN_SNOOZE) :
                            seconds = 0 ;
                            TMR1ON = 0 ;
                            TMR1H = 0 ;
                            TMR1L = 0 ;
                            TMR1ON = 1 ;
                            break ;

                        default :
                            break; 
                    }
                }
            }

            // drive display buffer
            if( (bALARM_SW == 0) && (displaytimer) )
            {
                // show alarm time
                digitbuffer[0] = hexlookup[ (alarm_minutes & 0xF) ] ;
                digitbuffer[1] = hexlookup[ (alarm_minutes & 0xF0) >> 4 ] ;

                if( strap & JMP_24 )
                {
                    if( alarm_hours )
                    {
                        digitbuffer[2] = hexlookup[ (twelve_hour(alarm_hours) & 0xF) ] ;
                        if( twelve_hour(alarm_hours) & 0xF0 )
                            digitbuffer[3] = hexlookup[ (twelve_hour(alarm_hours) & 0xF0) >> 4 ] ;
                        else
                            digitbuffer[3] = 0 ;
                    }
                    else
                    {
                        digitbuffer[3] = CHAR1 ;
                        digitbuffer[2] = CHAR2 ;
                    }
                    if( alarm_hours >= 0x12 )
                    {
                        // show PM indicator
                        digitbuffer[3] |= SEG_DP ;
                    }
                }
                else
                {
                    digitbuffer[2] = hexlookup[ (alarm_hours & 0xF) ] ;
                    if( alarm_hours & 0xF0 )
                        digitbuffer[3] = hexlookup[ (alarm_hours & 0xF0) >> 4 ] ;
                    else
                        digitbuffer[3] = 0 ;
                }


                if( blinktimer == 0 )
                {
                    if( blink )
                    {
                        blinktimer = BLINKTIME ;
                        blink = 0 ;
                    }
                    else
                    {
                        blinktimer = TICKSECONDS ;
                        blink = 1 ;
                    }
                }
                
            }
            else
            {
                blink = 1 ;
                blinktimer = 0 ;
                displaytimer = 0 ;

                if( (button & (BTN_UP|BTN_DOWN)) == 0 )
                {
                    // show MM:SS when both UP and DOWN are pressed
                    digitbuffer[0] = hexlookup[ (seconds & 0xF) ] ;
                    digitbuffer[1] = hexlookup[ (seconds & 0xF0) >> 4 ] ;
                    digitbuffer[2] = hexlookup[ (minutes & 0xF) ] ;
                    digitbuffer[3] = hexlookup[ (minutes & 0xF0) >> 4 ] ;
                }
                else
                {
                    // show time HH:MM
                    digitbuffer[0] = hexlookup[ (minutes & 0xF) ] ;
                    digitbuffer[1] = hexlookup[ (minutes & 0xF0) >> 4 ] ;

                    if( strap & JMP_24 )
                    {
                        if( hours )
                        {
                            digitbuffer[2] = hexlookup[ (twelve_hour(hours) & 0xF) ] ;
                            if( twelve_hour(hours) & 0xF0 )
                                digitbuffer[3] = hexlookup[ (twelve_hour(hours) & 0xF0) >> 4 ] ;
                            else
                                digitbuffer[3] = 0 ;
                        }
                        else
                        {
                            digitbuffer[3] = CHAR1 ;
                            digitbuffer[2] = CHAR2 ;
                        }
                        if( hours >= 0x12 )
                        {
                            // show PM indicator
                            digitbuffer[3] |= SEG_DP ;
                        }
                    }
                    else
                    {
                        digitbuffer[2] = hexlookup[ (hours & 0xF) ] ;
                        if( hours & 0xF0 )
                            digitbuffer[3] = hexlookup[ (hours & 0xF0) >> 4 ] ;
                        else
                            digitbuffer[3] = 0 ;
                    }
                }
            }

            // show colon between hours and minutes
            digitbuffer[2] |= SEG_DP ;

            if( bMAIN_DETECT == 0 )
            {
                SLEEP() ;
            }
        }
    }
}

unsigned char twelve_hour( unsigned char c )
{
    // TODO: More elegant implementation
    if( c >= 0x22 )
    {
        c -= 0x12 ;
    }
    else if( c == 0x21 )
    {
        c = 0x09 ;
    }
    else if( c == 0x20 )
    {
        c = 0x08 ;
    }
    else if( c > 0x12 )
    {
        c -= 0x12 ;
    }

    return(c) ;
}

unsigned char bcd_adjust_add( unsigned char c )
{
    if( (c & 0x0F) == 0xA )
    {
        return( c+6 ) ;
    }

    return(c) ;
}

unsigned char bcd_adjust_sub( unsigned char c )
{
    if( (c & 0xF) == 0xF )
    {
        return( c-6 ) ;
    }
    
    return(c) ;
}

void interrupt isr( void )
{
    if( TMR0IF )
    {
        TMR0IF = 0 ;
        tick++;
    }

    if( TMR1IF )
    {
        TMR1IF = 0 ;
        newsecond += 2 ;
    }
}
