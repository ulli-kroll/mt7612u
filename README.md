<u>**MT7612U for Linux**</u>

Tested with latest -stable, v4.15.6

Driver for 802.11ac USB Adapter with  
MT7612U/MT7632U/MT7662U chipset  
STA, AP are possible  
  
Current setting for this driver is APSTA mode as default.  
STA mode is working fine, (your need to set STA in Makefile)
AP mode **can** crash your kernel if you call `iwconfig` after module loading

Codepaths for Monitor Mode are missing, detected while fix memory leak in mt7610u  

<u>At least v4.2 is needed to compile this module</u>  
sorry people with older kernels, the code is removed.

Tested on X86_64 platform(s) **only**,  
cross compile possible

For compiling type  
`make`  
in source dir  

For install the needed firmware files  
`sudo make installfw`

For install the driver use  
`sudo insmod mt7612u.ko`  

To Unload driver you may need to disconnect the device

If the driver fails building consult your distro how to  
install the kernel sources and build an external module.
  
Questions about this will **silently** ignored !  
They are <u>plenty</u> information around the web.  

**NOTES**  

The original driver is **PITA**  
Both mt7610u and mt7612u <u>can</u> work with the same driver.  
**but currently dont't**  

Code which is missing in one driver, may found in the other driver.  
i.e STA, AP, Monitor, RSSI, LED handling stuff  

**STATUS**  
Driver works fine (some sort of)  
Most of the work is done is cleaning the driver and make this mess **readable**   for conversion.  
Updates for wireless-ext/cfg80211 are not accepted.  
The only solution is uptream and this is mac80211 support.  

**BUGS**  
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




