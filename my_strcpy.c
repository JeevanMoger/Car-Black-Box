/*
 * File:   my_strcpy.c
 * Author: Jeevan Moger
 *
 * Created on 16 November, 2024, 8:32 AM
 */

//
#include <xc.h>
#include "my_string.h"

void my_strcpy(char src[],char dest[])
{
    for(int i = 0; i < 15 ; i++)
    {
        dest[i] = src[i];
    }
}