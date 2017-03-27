/****************************************************************************

    Module Name:
	rt_usb_util.c

    Abstract:
	Any utility is used in UTIL module for USB function.

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------

***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rtmp_comm.h"
#include "rt_os_util.h"


#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND


void rausb_autopm_put_interface( void *intf)
{
	usb_autopm_put_interface((struct usb_interface *)intf);
}

int  rausb_autopm_get_interface( void *intf)
{
	return usb_autopm_get_interface((struct usb_interface *)intf);
}



/*
========================================================================
Routine Description:
	RTMP_Usb_AutoPM_Put_Interface

Arguments:


Return Value:


Note:
========================================================================
*/

int RTMP_Usb_AutoPM_Put_Interface (
	IN	VOID			*pUsb_Devsrc,
	IN	VOID			*intfsrc)
{

	INT	 pm_usage_cnt;
	struct usb_interface	*intf =(struct usb_interface *)intfsrc;

		pm_usage_cnt = atomic_read(&intf->pm_usage_cnt);

		if (pm_usage_cnt == 1)
		{
			rausb_autopm_put_interface(intf);

              }

			return 0;
}

EXPORT_SYMBOL(RTMP_Usb_AutoPM_Put_Interface);

/*
========================================================================
Routine Description:
	RTMP_Usb_AutoPM_Get_Interface

Arguments:


Return Value: (-1)  error (resume fail )    1 success ( resume success)  2  (do  nothing)


Note:
========================================================================
*/

int RTMP_Usb_AutoPM_Get_Interface (
	IN	VOID			*pUsb_Devsrc,
	IN	VOID			*intfsrc)
{

	INT	 pm_usage_cnt;
	struct usb_interface	*intf =(struct usb_interface *)intfsrc;

	pm_usage_cnt = (INT)atomic_read(&intf->pm_usage_cnt);

	if (pm_usage_cnt == 0)
	{
		int res=1;

		res = rausb_autopm_get_interface(intf);
		if (res)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel autopm_resume fail ------\n"));
			return (-1);
		}

	}
	return 2;


}

EXPORT_SYMBOL(RTMP_Usb_AutoPM_Get_Interface);

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */

