#ifndef OSARCHTYPE_H
#define OSARCHTYPE_H

/*
 * Operating System Type as defined by the update xml ("System x Update XML System Design Specification Documentation" v1.04 (Peter Donovan) Section 9.7.3 "Operating System Enumeration Values")
 */
typedef enum OSTypeEnum
  {
    eNone            = 0,
    eVMWare          = 1,
    eAIX             = 2,

    eWindowsNT       = 101,
    eWindows2000     = 102,
    eWindows2003     = 103,
    eWindowsPE       = 104,
    eWindowsXP       = 105,
    eWinDC2000       = 106,
    eWinDC2003       = 107,
    eWinVista        = 108,
    eWinServer2008   = 109,
    eWindows7        = 110,

    eRHEL3           = 201,
    eRHEL4           = 202,
    eRHEL5           = 203,
    eSLES8           = 204,
    eSLES9           = 205,
    eSLES10          = 206,
    eSLES11          = 207,
    eRHEL6           = 208,
	    
    eVMWareESX2      = 301,
    eVMWareESX30     = 302,
    eVMWareESX35     = 303,
    eVMWareESX40     = 304,
    eALL             = 999
  };

/*
 * Architecture Type as defined by the update xml ("System x Update XML System Design Specification Documentation" v1.04 (Peter Donovan) Section 9.5 Additional Fields supported by Update Schema 3.0 Row "orderedOSArchitectureList")
 */
typedef enum ArchTypeEnum
  {
    eNoArch = 0,
    e32bit = 1,
    e64bit = 2,
    eia64  = 3,
    ez32bit= 6,
    ez64bit= 7
  };
  
#endif
