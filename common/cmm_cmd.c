/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

 	Module Name:
	cmm_cmd.c

	Abstract:
	All command related API.

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
	Paul Lin    06-25-2004  created
*/

#include "rt_config.h"




/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
VOID	RTInitializeCmdQ(
	IN	PCmdQ	cmdq)
{
	cmdq->head = NULL;
	cmdq->tail = NULL;
	cmdq->size = 0;
	cmdq->CmdQState = RTMP_TASK_STAT_INITED;
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
VOID	RTThreadDequeueCmd(
	IN	PCmdQ		cmdq,
	OUT	PCmdQElmt	*pcmdqelmt)
{
	*pcmdqelmt = cmdq->head;

	if (*pcmdqelmt != NULL)
	{
		cmdq->head = cmdq->head->next;
		cmdq->size--;
		if (cmdq->size == 0)
			cmdq->tail = NULL;
	}
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
int RTEnqueueInternalCmd(
	IN struct rtmp_adapter *pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN uint32_t 		InformationBufferLength)
{
	int status;
	PCmdQElmt	cmdqelmt = NULL;


	cmdqelmt = kmalloc(sizeof(CmdQElmt), GFP_ATOMIC);
	if (cmdqelmt == NULL)
		return (NDIS_STATUS_RESOURCES);
	memset(cmdqelmt, 0, sizeof(CmdQElmt));

	if(InformationBufferLength > 0)
	{
		cmdqelmt->buffer = kmalloc(InformationBufferLength, GFP_ATOMIC);
		if (cmdqelmt->buffer == NULL) {
			kfree(cmdqelmt);
			return (NDIS_STATUS_RESOURCES);
		}
		else
		{
			memmove(cmdqelmt->buffer, pInformationBuffer, InformationBufferLength);
			cmdqelmt->bufferlength = InformationBufferLength;
		}
	}
	else
	{
		cmdqelmt->buffer = NULL;
		cmdqelmt->bufferlength = 0;
	}

	cmdqelmt->command = Oid;
	cmdqelmt->CmdFromNdis = false;

	if (cmdqelmt != NULL)
	{
		spin_lock_bh(&pAd->CmdQLock);
		if (pAd->CmdQ.CmdQState & RTMP_TASK_CAN_DO_INSERT)
		{
			EnqueueCmd((&pAd->CmdQ), cmdqelmt);
			status = NDIS_STATUS_SUCCESS;
		}
		else
		{
			status = NDIS_STATUS_FAILURE;
		}
		RTMP_SEM_UNLOCK(&pAd->CmdQLock);

		if (status == NDIS_STATUS_FAILURE)
		{
			if (cmdqelmt->buffer)
				kfree(cmdqelmt->buffer);
			kfree(cmdqelmt);
		}
		else
			RTCMDUp(&pAd->cmdQTask);
	}
	return(NDIS_STATUS_SUCCESS);
}
