#ifndef GET_RESOURCE_HPP_INCLUDED
#define GET_RESOURCE_HPP_INCLUDED

#define MAX_LENGTH 256

class GetResource {
   // Used to control the loops for passing multiple strings.
   ULONG Stri;                                             
public:
   // *********************************************************************
   // NOTE: Everyone should be using the Resource.DLL for all of
   //       the strings.  You can obtain the handle by using
   //       LoadLibrary for Windows and DosLoadModule under OS/2.
   //       I designed this class to be expandable if need be at
   //       a later date without having to change older code.
   //       Anyone can add constructor functions as needed.
   // *********************************************************************
   // Default Constructor needed to use the ReturnString functions.
   GetResource();                            
   // Constructor that takes resource id and module handle.
   GetResource(ULONG, HMODULE);                            
   // Constructor that takes resource id, module handle, 
   // and character pointer.
   GetResource(ULONG, HMODULE, char *);
   // Constructor that takes resource id, module handle, 
   // and constant character pointer.
   GetResource(ULONG, HMODULE, const char *);
   // Constructor that takes resource id, module handle, 
   // integer, and character pointer.
   GetResource(ULONG, HMODULE, int, char *);
   // Constructor that takes resource id, module handle, 
   // and integer.
   GetResource(ULONG, HMODULE, int);
   // Constructor that takes resource id, module handle, 
   // integer, and integer.
   GetResource(ULONG, HMODULE, int, int);
   // Constructor that takes resource id, module handle, 
   // and two character pointers.
   GetResource(ULONG, HMODULE, char *, char *);
   // Constructor that takes module handle, unsigned message array,
   // and number of messages in array.
   GetResource(HMODULE, ULONG *, ULONG);
   // Constructor that takes module handle, unsigned message array, 
   // character pointer, and number of messages in array.
   GetResource(HMODULE, ULONG *, char *, ULONG);
   // Constructor that takes module handle, unsigned message array, 
   // integer, constant integer, and number of messages in array.
   GetResource(HMODULE, ULONG *, int, const int, ULONG);
   // ReturnString function that takes resource id and module handle
   // then returns the specified string.
   char *ReturnString(ULONG, HMODULE);                            
   // ReturnString function that takes resource id, module handle, and integer
   // puts the string together with arguments and then returns the specified string.
   char *ReturnString(ULONG, HMODULE, int);
   // ReturnString function that takes resource id, module handle, and character array
   // puts the string together with arguments and then returns the specified string.
   char *ReturnString(ULONG, HMODULE, char[]);
};
       
#endif

