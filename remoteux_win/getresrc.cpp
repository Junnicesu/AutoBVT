/**************************************************************************************
*
*  Program name                 : GetResrc.obj
*                                 Gets linked into the strings library (strings.lib).
*
*  Dependencies                 : GetResrc.cpp
*                                 GetResrc.hpp
*                                 strings.hpp
*                    
*  Objective                    : Get and print out resources from the .res files you
*                                 link into your program via the strings library.
*                                 Will also return the specified string with any
*                                 parameters you pass.
*                    
*  Parameters                   : Many overloaded constructor functions can be used.
*                                 Many overloaded ReturnString functions can be used.
*
*  Written by                   : Mike Nolterieke
*                                 mikenol@us.ibm.com
*                                 919-526-0175 - T/L 526-0175
*                     
*  Change History               : who -   when   - what
*                                 MHN - 02/27/00 - Initial revision.
*                                 MHN - 06/30/00 - Added ReturnString Functions.
*                                 MHN - 08/26/00 - Added check for NULL strings.
*                                                  Changed NOTE, we are now using a
*                                                  resource DLL.
*                     
**************************************************************************************/

// This program will compile and work on all platforms by using the #ifdefs.
#ifdef _OS2
  #include <os2.h>
#elif defined(FDR_OS_WINDOWS)
  #include <windows.h>
#endif
  
#include <windows.h> //WW
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#ifndef GET_RESOURCE_HPP_INCLUDED
  #include "GetResrc.hpp"  // Class declaration.
#endif
#ifndef STRINGS_HPP_INCLUDED
  #include "strings.hpp"   // Str class declaration used for Str objects.
#endif

char *strtoreturn = new char[MAX_LENGTH];

// *********************************************************************
// NOTE: Everyone should be using the Resource.DLL for all of
//       the strings.  You can obtain the handle by using
//       LoadLibrary for Windows and DosLoadModule under OS/2.
//       I designed this class to be expandable if need be at
//       a later date without having to change older code.
//       Anyone can add constructor functions as needed.
// *********************************************************************
   
// Default Constructor needed to use the ReturnString functions.
GetResource::GetResource()
{
}

// Constructor that takes resource id and module handle.
GetResource::GetResource(ULONG id, HMODULE mod)
{
   Str message(id, mod);
   if (message) 
     {
       fprintf(stdout, (PCHAR)message);
       fprintf(stdout, "\n");
       fflush(stdout);
     }
}

// Constructor that takes resource id, module handle, 
// and character pointer.
GetResource::GetResource(ULONG id, HMODULE mod, char *var1)
{
   Str message(id, mod);
   if (message) 
     {
       fprintf(stdout, (PCHAR)message, var1);
       fprintf(stdout, "\n");
       fflush(stdout);
     }
}

// Constructor that takes resource id, module handle, 
// and constant character pointer.
GetResource::GetResource(ULONG id, HMODULE mod, const char *var1)
{
   Str message(id, mod);
   if (message) 
     {
       fprintf(stdout, (PCHAR)message, var1);
       fprintf(stdout, "\n");
       fflush(stdout);
     }
}

// Constructor that takes resource id, module handle, 
// integer, and character pointer.
GetResource::GetResource(ULONG id, HMODULE mod, int num, char *var1)
{
   Str message(id, mod);
   if (message) 
     {
       fprintf(stdout, (PCHAR)message, num, var1);
       fprintf(stdout, "\n");
       fflush(stdout);
     }
}

// Constructor that takes resource id, module handle, 
// and integer.
GetResource::GetResource(ULONG id, HMODULE mod, int num)
{
   Str message(id, mod);
   if (message) 
     {
       fprintf(stdout, (PCHAR)message, num);
       fprintf(stdout, "\n");
       fflush(stdout);
     }
}

// Constructor that takes resource id, module handle, 
// integer, and integer.
GetResource::GetResource(ULONG id, HMODULE mod, int num, int var1)
{
   Str message(id, mod);
   if (message) 
     {
       fprintf(stdout, (PCHAR)message, num, var1);
       fprintf(stdout, "\n");
       fflush(stdout);
     }
}

// Constructor that takes resource id, module handle, 
// and two character pointers.
GetResource::GetResource(ULONG id, HMODULE mod, char *var1, char *var2)
{
   Str message(id, mod);
   if (message) 
     {
       fprintf(stdout, (PCHAR)message, var1, var2);
       fprintf(stdout, "\n");
       fflush(stdout);
     }
}

// Constructor that takes module handle, unsigned message array,
// and number of messages in array.
GetResource::GetResource(HMODULE mod, ULONG *mesarray, ULONG howmanystrs)
{

   fprintf(stdout, "\n");
   for(Stri = 0; Stri < howmanystrs; Stri++) 
     {
       Str message(mesarray[Stri], mod);
       if (message) 
         {
           fprintf(stdout, (PCHAR)message);
           fprintf(stdout, "\n");
           fflush(stdout);
         }
     }
}

// Constructor that takes module handle, unsigned message array, 
// character pointer, and number of messages in array.
GetResource::GetResource(HMODULE mod, ULONG *mesarray, char *parameter, ULONG howmanystrs)
{

   fprintf(stdout, "\n");
   for(Stri = 0; Stri < howmanystrs; Stri++) 
     {
       Str message(mesarray[Stri], mod);
       if (message) 
         {
           fprintf(stdout, (PCHAR)message, parameter);
           fprintf(stdout, "\n");
           fflush(stdout);
         }
     }
}

// Constructor that takes module handle, unsigned message array, 
// integer, constant integer, and number of messages in array.
GetResource::GetResource(HMODULE mod, ULONG *mesarray, int num, const int num2, ULONG howmanystrs)
{

   fprintf(stdout, "\n");
   for(Stri = 0; Stri < howmanystrs; Stri++) 
     {
        Str message(mesarray[Stri], mod);
        if (message) 
          {
            fprintf(stdout, (PCHAR)message, num, num2);
            fprintf(stdout, "\n");
            fflush(stdout);
          }
     }
}

// ReturnString function that takes resource id and module handle
// then returns the specified string.
char *GetResource::ReturnString(ULONG id, HMODULE mod)
{
   memset(strtoreturn, 0, MAX_LENGTH);

   Str message(id, mod);
   if (message)
     {
	   sprintf_s(strtoreturn, MAX_LENGTH, (PCHAR)message);
       return strtoreturn;
     }
   return NULL;
}

// ReturnString function that takes resource id, module handle, and integer
// puts the string together with arguments and then returns the specified string.
char *GetResource::ReturnString(ULONG id, HMODULE mod, int num)
{
   memset(strtoreturn, 0, MAX_LENGTH);

   Str message(id, mod);
   if (message)
     {
       sprintf_s(strtoreturn, MAX_LENGTH,(PCHAR)message, num);
       return strtoreturn;
     }
   return NULL;
}

// ReturnString function that takes resource id, module handle, and character array
// puts the string together with arguments and then returns the specified string.
char *GetResource::ReturnString(ULONG id, HMODULE mod, char parameter[])
{
   memset(strtoreturn, 0, MAX_LENGTH);

   Str message(id, mod);
   if (message)
     {
       sprintf_s(strtoreturn, MAX_LENGTH, (PCHAR)message, parameter);
       return strtoreturn;
     }
   return NULL;
}

