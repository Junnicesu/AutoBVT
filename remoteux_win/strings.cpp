/**************************************************************************************
*
*  Program name                 : strings.lib
*
*  Dependencies                 : strings.cpp
*                                 strings.hpp
*                                 GetResrc.obj
*                                 makefile (Windows and OS/2 versions)
*                    
*  Objective                    : Get resources from the .res files you
*                                 link into your program.
*                    
*  Parameters                   : Many overloaded constructor functions can be used.
*
*  Written by                   : Mike Nolterieke
*                                 mikenol@us.ibm.com
*                                 919-526-0175 - T/L 526-0175
*                     
*  Change History               : who -   when   - what
*                                 MHN - 02/27/00 - Initial revision.
*                                 MHN - 08/26/00 - Changed NOTE, we are now using a
*                                                  resource DLL.
*                     
**************************************************************************************/

// This program will compile and work on all platforms by using the #ifdefs.
#ifdef _OS2
  #define INCL_DOS
  #include <os2.h>
  #include <iostream.h>
#elif defined(FDR_OS_WINDOWS)
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <iostream>
  #include <windows.h>
#endif

// Str class declaration.
#ifndef STRINGS_HPP_INCLUDED
  #include "strings.hpp"
#endif

// *********************************************************************
// NOTE: Everyone should be using the Resource.DLL for all of
//       the strings.  You can obtain the handle by using
//       LoadLibrary for Windows and DosLoadModule under OS/2.
//       I designed this class to be expandable if need be at
//       a later date without having to change older code.
//       Anyone can add constructor functions as needed.
// *********************************************************************

// Constructor that takes resource id and module handle.
Str::Str(ULONG id, HMODULE mod)
{
#ifdef _OS2
   PCHAR  tmp, tmp1;
   ULONG  i;

   if ( ! DosGetResource( mod, RT_STRING, id / 16 + 1, (PPVOID) &tmp1 ) ) 
      {
        tmp = tmp1;
        // If found, search for offset of needed string
        tmp = tmp + 2;                      // Start at length of first string
        for ( i = 0; i < id % 16; i++ ) 
           {
             ULONG  off = (ULONG) tmp[0] & 0xFF;
             // Move offset to just after current string
             tmp = tmp + off + 1;
           }
        length = tmp[ 0 ];               // Get length of space needed
        ptr = new char[ length ];        // Get space for string
        strcpy_s( ptr ,length + 1,tmp + 1 );          // Copy string into buffer
        DosFreeResource( tmp1 );         // Release the resource
      } 
   else 
      {
        length = 0;                      // Get no space
        ptr = NULL;
      } 
#elif defined(FDR_OS_WINDOWS)
   CHAR   tmp[256];                             

   // Load string from resource
   if(!mod){
       mod= GetModuleHandle("remoteux.dll");
   }
   if(!mod){
       exit(100);
   }

   length = ::LoadString(mod, id, tmp, 256);    

   if (length != 0)
     {
       ptr = new char[ length + 1 ];  // Get space for string
       strcpy_s(ptr,length + 1, tmp );            // Copy string into buffer
     }
#endif
}

// Type conversion to an ASCIIZ string                                              
Str::operator const PCHAR()
{
   return ptr;                       // Return address of the buffer             
}

Str::~Str()
{
   if ( ptr != NULL )                // If space was allocated
      delete ptr;                    // Free it
}

// Load string directly into a buffer                                                      
BOOL Str::LoadString(ULONG id, HMODULE hmod, PCHAR buf, ULONG len)
{
#ifdef FDR_OS_WINDOWS
   return ::LoadString(hmod, id, buf, len);
#endif
   // As far as I can tell, this will never be used but need it to get rid of 
   // OS/2 compile error.
   return FALSE;
}


