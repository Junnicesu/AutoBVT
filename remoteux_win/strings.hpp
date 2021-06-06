#ifndef STRINGS_HPP_INCLUDED
#define STRINGS_HPP_INCLUDED

#include <windows.h>

//typedef unsigned long ULONG ;
//typedef char * PCHAR;
//typedef HINSTANCE HMODULE;


class Str {
protected:
   ULONG   length;             // Number of bytes allocated in the string
   PCHAR   ptr;                // Pointer to ASCIIZ string
public:
   // *********************************************************************
   // NOTE: Everyone should be using the Resource.DLL for all of
   //       the strings.  You can obtain the handle by using
   //       LoadLibrary for Windows and DosLoadModule under OS/2.
   //       I designed this class to be expandable if need be at
   //       a later date without having to change older code.
   //       Anyone can add constructor functions as needed.
   // *********************************************************************

   // Constructor that takes resource id and module handle.
   Str(ULONG id, HMODULE mod); 
   // Destructor
   ~Str();                     
   // Type conversion to an ASCIIZ string
   operator const PCHAR();     
   // Load string directly into buffer
   static BOOL LoadString(ULONG id, HMODULE hmod, PCHAR buf, ULONG len);
};
#endif
