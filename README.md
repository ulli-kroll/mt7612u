<u>**MT7612U for Linux**</u>

Driver for 802.11ac USB Adapter with MT7610U chipset  
STA, AP and Monitor Modes are possible  
Current setting for this driver is APSTA mode as default.  
STA mode is working fine  
AP mode **can** crash your kernel if you call `iwconfig` after module loading

<u>At least v4.2 is needed to compile this module</u>  
sorry people with older kernels, the code is removed.

This driver is **currently** under **heavy** development.  

Tested on X86_64 platform(s) **only**,  
cross compile possible, but not fully tested.  

For compiling type  
`make`  
in source dir  

For install the driver use  
`sudo insmod mt7612u.ko`  

To Unload driver you need to disconnect the device

If the driver fails building consult your distro how to  
install the kernel sources and build an external module.
  
Questions about this will **silently** ignored !  
They are <u>plenty</u> information around the web.  

**BUGS**  
- can't unload driver, must disconnect device(s) first  
- use of wireless-ext
  

**TODO**, in order of no appearance ;-)  
- enable cfg80211  
- remove stupid AP/STA switch  
- fix unloading driver  
- do more function typesafe  
- cross compile check with real hw on $target  
- strip fw files and use kernel firmware load  
- check for wrong typecasts  
- remove/strip hardcoded `RT2870STA.dat`  
- check for wrong variable sizes (driver was for 32bit)  
- update to more USB-IDs  
- check if monitor mode is working  
- more cleanup and other stuff  
- fix compile warnings  
- misc. other stuff  


Hans Ulli Kroll <ulli.kroll@googlemail.com>




