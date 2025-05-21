/*
 * File:   main.c
 * Author: Jeevan Moger
 * PROJECT : CAR BLACK BOX
 * Created on 11 November, 2024, 9:10 AM
 */


#include <xc.h>
#include "adc.h"
#include "matrix_keypad.h"
#include "clcd.h"
#include "eeprom.h"
#include "ds1307.h"
#include "i2c.h"
#include "my_string.h"
#include "ext_eeprom.h"
#include "uart.h"

// Gear states and menu options
char gr[][3] = {"ON","GN","GR","G1","G2","G3","G4","G5","C "};        
char *menu[] = {"VIEW_LOG","SET_TIME","DOWNLOAD LOG","CLEAR_LOG"};

// Global variables for flags, counters, and states
unsigned int gear_inc,collision_flag,overwrite_flag,menu_flag,key_count,key1_count,key1_flag,clear_flag;
int wh_menu,go_in,menu_count;
unsigned int sp;                    // speed variable
unsigned int adc_val,event_count = 0,temp,download_flag = 1;
unsigned char addr = 0x00;   // address value to write into eeprom
int index,num;
unsigned int print_flag,var,var2;
unsigned long int delay;
char event_data[10][15];                // to store the data 


// RTC VARIABLES
unsigned char clock_reg[3];
unsigned char calender_reg[4];
unsigned char time[9];
unsigned char date[11];

// SET TIME VARIABLES 
unsigned int field,time_flag = 1;
int hr,min,sec;

static void get_time(void)
{
	clock_reg[0] = read_ds1307(HOUR_ADDR);
	clock_reg[1] = read_ds1307(MIN_ADDR);
	clock_reg[2] = read_ds1307(SEC_ADDR);

	if (clock_reg[0] & 0x40)
	{                                                    // 12 hr format
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);    // read the hr's tens digit stored in bit 3    
		time[1] = '0' + (clock_reg[0] & 0x0F);           // read 0 to 3 bits for units place of the hr
	}
	else
	{                                                       // 24 hrs format
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);      // read the hr's tens digit stored in bit 4 and 5
		time[1] = '0' + (clock_reg[0] & 0x0F);              // read 0 to 3 bits for units place of the hr
	}
	time[2] = ':';
	time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
	time[4] = '0' + (clock_reg[1] & 0x0F);
	time[5] = ':';
	time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
	time[7] = '0' + (clock_reg[2] & 0x0F);
	time[8] = '\0';
}

void read_log(char data[10][15])
{
    unsigned char address = 0x00;                           // read from 0x00 address
    for(int i = 0; i < 10 ; i++)
    {
        for(int j = 0; j  < 14 ; j++)
        {
            if(j == 2 || j == 5)
            {
                data[i][j] = ':';                           //add respective special characters 
            }
            else if (j == 8 || j == 11)
            {
                data[i][j] = ' ';
            }
            else
                data[i][j] =read_ext_eeprom(address++);  // read from the external eeprom
        }
        data[i][14] = '\0';
    }
    
}
 

//VIEW LOG FUNCTION TO VIEW THE LOGS STORED
void view_log(unsigned char key)
{
    if(event_count == 0)                               // IF NO EVENTS ARE STORED DISPLAY NO LOGS
    {
        if(delay++ < 500)
        {
            clcd_print("NO LOGS ",LINE1(0));
            clcd_print("TO DISPLAY :(",LINE2(3));
            return;
        }
        go_in=1;
        delay = 0;
        wh_menu = 0;
        menu_count = 0;
        key_count = 0;
        return;
    }
 
    read_log(event_data);
        
    clcd_print("# VIEW_LOGS  : ",LINE1(0));
    
    // NAVIGATE THROUGH THE LOGS 
    if(key == 1)
    {
        if(index > 0)
            index --;
        else
            index = 0;
    }
    if(key == 2)
    {
        if(index >= event_count - 1)
            index = event_count-1;
        else
            index++;
        
        if(index == 10)
            index = 9;
    }

    if(event_count > 10)                // TO DISPLAY THE OLDEST EVENTS FIRST 
    {
        var = event_count % 10;
    }
    else
        var = 0;

    clcd_putch(index + '0' ,LINE2(0));
    clcd_print(event_data[(index+var) % 10],LINE2(2)); // DISPLAY THE OLDEST FOLLOWED BY THE NEWEST EVENT
}

// TO SET THE TIME 
void set_time(unsigned char key)
{
    clcd_print("HH:MM:SS",LINE1(0));
    
    // CALCULATE HOUR,MINUTES AND SECONDS FROM RTC
    if(time_flag == 1)
    {
        hr = (time[0] - 48) * 10;
        hr = hr + (time[1] - 48);
    
        min = (time[3] - 48) * 10;
        min = min + (time[4] - 48);
    
        sec = (time[6] - 48) * 10;
        sec = sec + (time[7] - 48);
        time_flag = 0;
    }
    
    //CHANGE THE FIELDS FROM HR to MIN , MIN to SEC and then SEC to HR WHEN 
   
    if(key == 2)
    {
        if(field >= 2)
            field = 0;
        else
            field++;
    }
    
    //INCREMENT THE RESPECTIVE VALUE BASED ON THE FIELD
    
    if(key == 1)
    {
        if(field == 0)
        {
            hr++;
            if(hr > 23)
                hr = 0;
            
        }
        else if(field == 1)
        {
            min++;
            if(min > 59)
                min  = 0;
        }
        else if(field == 2)
        {
            sec++;
            if(sec > 59)
                sec = 0;
        }
            
    }
    
    // WHEN KEY 11 IS PRESSED WRITE THE DATA INTO THE RTC
    
    if(key == 11)
    {
        write_ds1307(HOUR_ADDR,(((hr / 10) << 4)| (hr % 10)));
        write_ds1307(MIN_ADDR,(((min / 10) << 4)| (min % 10)));
        write_ds1307(SEC_ADDR,(((sec / 10) << 4)| (sec % 10)));
        go_in = 0;
        wh_menu = 0;
        menu_count  = 0;
        key_count = 0;
        
    }
    // BLINKING THE RESPECTIVE FIELD WITH SOME DELAY
    
    if(delay++ < 200)
    {
        clcd_putch((hr/10) + '0',LINE2(0));
        clcd_putch((hr % 10) + '0',LINE2(1));
        clcd_putch(':',LINE2(2));
        clcd_putch((min/10) + '0',LINE2(3));
        clcd_putch((min % 10) + '0',LINE2(4));
        clcd_putch(':',LINE2(5));
        clcd_putch((sec/10) + '0',LINE2(6));
        clcd_putch((sec % 10) + '0',LINE2(7));
        
    }
    else if(delay > 200 && delay < 400)
    {
        if(field == 0)
            clcd_print("  ",LINE2(0));
        if(field == 1)
            clcd_print("  ",LINE2(3));
        if(field == 2)
            clcd_print("  ",LINE2(6));
    }
    else
        delay = 0;
    
}

// TO DOWNLOAD THE LOGS THROUGH UART

void download_logs()
{
    // IF NO LOGS ARE THERE PRINT NO LOGS ON CLCD AND TERA TERM
    if(event_count == 0)
    {
        clcd_print("NO LOGS ",LINE1(0));
        clcd_print("TO DOWNLOAD :(",LINE2(2));
        if(delay++ < 500)
            return;
        puts("SORRY NO LOGS TO DOWNLOAD :(\n\r");
        go_in=1;
        delay = 0;
        wh_menu = 0;
        menu_count = 0;
        key_count = 0;
        return;
    }
    if(delay++ < 500)
    {
        clcd_print("DOWNLOADING",LINE1(0));
        clcd_print("LOGS.....",LINE2(10));
        return;
    }
    else
    {
        read_log(event_data);
    
        int lim;
        if(event_count > 10)
        {
            var2 = event_count % 10;
            lim = 10;
        }
        else
        {
            lim = event_count; 
            var2 = 0;
        }
        // PRINT THE LOGS IN TERA TERM USING TERA TERM
        if(download_flag == 1)
        {
            puts("#   TIME   EV SP\n\r");
        
            char var[3];
            var[1] = ' ';
            var[2] = '\0';
            for(int i = 0 ; i < lim ; i++)
            {
                var[0] = i + '0';
                puts(var);
                puts(event_data[(i + var2) % 10]);
                puts("\n\r");
            }
            download_flag = 0;
        }
        go_in =1;
        delay = 0;
        wh_menu = 0;
        menu_count = 0;
        key_count = 0;
    }
}

// FUNCTION TO CLEAR THE LOGS 

void clear_log()
{
    clcd_print("CLEARING LOGS...",LINE1(0));
    clcd_print("JUST A MINUTE",LINE2(0));
    if(delay++ < 500)
    {
        clcd_print("CLEARING LOGS...",LINE1(0));
        clcd_print("JUST A MINUTE",LINE2(0));
        return;
    }
    go_in = 1;
    event_count = 0;
    addr = 0x00;
    wh_menu = 0;
    menu_count = 0;
    key_count = 0;
}

// FUNCTION TO STORE THE LOGS INTO EXTERNAL EEPROM

void  store_log(void)
{
    if(addr == 0x64)         // IF 10 EVENTS ARE STORED , THEN OVERWRITE FROM THE FIRST ADDRESS
    {
        addr = 0x00;
        overwrite_flag = 1;
    }
    
        // STORING DATA
    CLEAR_DISP_SCREEN;
    write_ext_eeprom(addr++,time[0]);
    write_ext_eeprom(addr++,time[1]);
    write_ext_eeprom(addr++,time[3]);
    write_ext_eeprom(addr++,time[4]);
    write_ext_eeprom(addr++,time[6]);
    write_ext_eeprom(addr++,time[7]);
    write_ext_eeprom(addr++,gr[gear_inc][0]);
    write_ext_eeprom(addr++,gr[gear_inc][1]);
    write_ext_eeprom(addr++,((sp/10) + '0'));
    write_ext_eeprom(addr++,((sp % 10)+'0'));
}

// FUNCTION TO DISPLAY THE DASHBOARD
void dashboard_display(unsigned char key)
{
        if(key == 1)                    //KEY 1 PRESSED INCREMENT THE GEAR  
        {
            
            temp = event_count;
            gear_inc++;
            if(gear_inc > 7)
                gear_inc = 7;
            else
            {
                store_log();
                event_count++;
            }
            if(collision_flag)
            {
                collision_flag = 0;
                gear_inc = 1;
            }
        }
        if(key == 2)            // IF KEY 2 IS PRESSED DECRE<ENT THE GEAR
        {
            temp = event_count;
            if(gear_inc > 1)
            {
                gear_inc--;
                event_count++;
                store_log();
            }
            if(gear_inc < 1 && gear_inc != 0)
                gear_inc = 1;
            
            if(collision_flag)
            {
                collision_flag = 0;
                gear_inc = 1;
            }
            
        }
        if(key == 3)            // IF KEY3 IS PRESSED SIMULATE A COLLISION
        {
            collision_flag = 1;
            event_count++;
            gear_inc = 8;
            store_log();
        }
        adc_val = read_adc(CHANNEL4);
        sp = adc_val / 10;
        if(sp > 99)
            sp = 99;
        // DISPLAY THE VALUE ON THE CLCD
        clcd_print("  TIME    EV  SP",LINE1(0));
        clcd_print(time,LINE2(0));

        clcd_print(gr[gear_inc],LINE2(10));
        clcd_putch((sp/10) + '0',LINE2(14));
        clcd_putch((sp % 10) + '0',LINE2(15));
}


// FUNCTION TO DISPLAY THE MAIN MENU
void menu_display(unsigned char key)
{

    // BASED ON KEY PRESS NAVIGATE THROUGH  
    if(key == 1)
    {
        
        CLEAR_DISP_SCREEN;
        if(go_in ==1)
        {
            wh_menu--;
            if(wh_menu <= 0)
                wh_menu  = 0;
            key_count  = 0;
            key1_flag = 1; 
            key1_count++;
            clear_flag = 1;
            if(key1_count > 1)
            {
                menu_count--;
                if(menu_count <= 0)
                    menu_count = 0;
            }
        }
    }
    if(key == 2)
    {
       CLEAR_DISP_SCREEN;
       if(go_in == 1)
       {
            wh_menu++;
            if(wh_menu >= 3)
                wh_menu = 3;
            key1_flag = 0;
            key_count++;
            key1_count = 0;
            if(key_count > 1)
            {
                menu_count++;
                if(menu_count > 2)
                    menu_count = 2;
            }     
        }
    }
    
    //DISPLAY * ON THE CLCD
    
    if((key_count == 0 || key1_flag) && go_in == 1)
    {
        clcd_putch('*',LINE1(0));
    }
    else if (go_in == 1)
        clcd_putch('*',LINE2(0));                                                 
     
    // DISPLAY THE MENU ON THE CLCD
    
    clcd_print(menu[menu_count],LINE1(1));
    clcd_print(menu[menu_count+1],LINE2(1));   
    download_flag = 1;
    time_flag = 1;
    delay = 0;
    field = 0;
}

// INITAL CONFIGURATIONS FOR THE PERIPHERALS 
void init_config()
{
    init_clcd();
    init_matrix_keypad();
    init_adc();
    init_i2c();
	init_ds1307();
    init_uart();
    CLEAR_DISP_SCREEN;
}
void main(void) {
    init_config();
    unsigned char key,prekey;         // VARIABLES FOR KEYS 
    while(1)
    {
        get_time();                             // TO GET THE TIME FROM RTC
        key = read_switches(STATE_CHANGE);
        prekey = key;
        if(key == 11)
        {
            CLEAR_DISP_SCREEN;
            go_in++;                            // GO IN FLAG TO NAVIGATE THROUGH DASHBOARD MAIN MENU AND SUB MENU
            if(go_in > 2)
                go_in  = 2;
            clear_flag = 1;
        }
        if(key == 12)
        {
            CLEAR_DISP_SCREEN;
            go_in--;
            if(go_in < 0)
                go_in  = 0;
            key_count = 0;
            menu_count= 0;
            clear_flag = 1;
            wh_menu  = 0;
            index = 0;
        }
        if(go_in < 0)
            go_in = 0;
        
        if(go_in == 0)      // IF GO_IN = 0 GO TO DASHBOARD
        {
            if(clear_flag)
            {
                CLEAR_DISP_SCREEN;
                clear_flag = 0;
            } 
            dashboard_display(key);
        }
        if(go_in == 1)              //IF GO_IN == 1 GO TO MAIN MENU
        {
            if(clear_flag)
            {
                CLEAR_DISP_SCREEN;
                clear_flag = 0;
            }
            menu_display(key);
        }
        
        // BASED ON wh_menu and go_in == 2 GO_TO THE SUB MENU
        
        if(go_in == 2 && wh_menu == 0)  
        {
            view_log(key);
        }
        if(go_in == 2 && wh_menu == 1)
        {
            if(prekey == 11 && time_flag == 1)
                key = 0xFF;
            set_time(key);
        }
        if(go_in == 2 && wh_menu == 2)
        {
            download_logs();
        }
        if(go_in == 2 && wh_menu == 3)
        {
            clear_log(); 
        }
        
    }    
    return;
}
