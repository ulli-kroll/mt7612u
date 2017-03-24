/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	cmm_profile.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"


#define ETH_MAC_ADDR_STR_LEN 17  /* in format of xx:xx:xx:xx:xx:xx*/

/* We assume the s1 is a sting, s2 is a memory space with 6 bytes. and content of s1 will be changed.*/
bool rtstrmactohex(char *s1, char *s2)
{
	int i = 0;
	char *ptokS = s1, *ptokE = s1;

	if (strlen(s1) != ETH_MAC_ADDR_STR_LEN)
		return FALSE;

	while((*ptokS) != '\0')
	{
		if((ptokE = strchr(ptokS, ':')) != NULL)
			*ptokE++ = '\0';
		if ((strlen(ptokS) != 2) || (!isxdigit(*ptokS)) || (!isxdigit(*(ptokS+1))))
			break; /* fail*/
		AtoH(ptokS, (u8 *)&s2[i++], 1);
		ptokS = ptokE;
		if (ptokS == NULL)
			break;
		if (i == 6)
			break; /* parsing finished*/
	}

	return ( i == 6 ? TRUE : FALSE);

}


#define ASC_LOWER(_x)	((((_x) >= 0x41) && ((_x) <= 0x5a)) ? (_x) + 0x20 : (_x))
/* we assume the s1 and s2 both are strings.*/
bool rtstrcasecmp(char *s1, char *s2)
{
	char *p1 = s1, *p2 = s2;
	CHAR c1, c2;

	if (strlen(s1) != strlen(s2))
		return FALSE;

	while(*p1 != '\0')
	{
		c1 = ASC_LOWER(*p1);
		c2 = ASC_LOWER(*p2);
		if(c1 != c2)
			return FALSE;
		p1++;
		p2++;
	}

	return TRUE;
}


/* we assume the s1 (buffer) and s2 (key) both are strings.*/
char *rtstrstruncasecmp(char *s1, char *s2)
{
	INT l1, l2, i;
	char temp1, temp2;

	l2 = strlen(s2);
	if (!l2)
		return (char *) s1;

	l1 = strlen(s1);

	while (l1 >= l2)
	{
		l1--;

		for(i=0; i<l2; i++)
		{
			temp1 = *(s1+i);
			temp2 = *(s2+i);

			if (('a' <= temp1) && (temp1 <= 'z'))
				temp1 = 'A'+(temp1-'a');
			if (('a' <= temp2) && (temp2 <= 'z'))
				temp2 = 'A'+(temp2-'a');

			if (temp1 != temp2)
				break;
		}

		if (i == l2)
			return (char *) s1;

		s1++;
	}

	return NULL; /* not found*/
}


 /**
  * strstr - Find the first substring in a %NUL terminated string
  * @s1: The string to be searched
  * @s2: The string to search for
  */
char *rtstrstr(char *s1, char *s2)
{
	INT l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return s1;

	l1 = strlen(s1);

	while (l1 >= l2)
	{
		l1--;
		if (!memcmp(s1,s2,l2))
			return s1;
		s1++;
	}

	return NULL;
}

/**
 * rstrtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 * * WARNING: strtok is deprecated, use strsep instead. However strsep is not compatible with old architecture.
 */
char *__rstrtok;
char *rstrtok(char *s,const char *ct)
{
	char *sbegin, *send;

	sbegin  = s ? s : __rstrtok;
	if (!sbegin)
	{
		return NULL;
	}

	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0')
	{
		__rstrtok = NULL;
		return( NULL );
	}

	send = strpbrk( sbegin, ct);
	if (send && *send != '\0')
		*send++ = '\0';

	__rstrtok = send;

	return (sbegin);
}

/**
 * delimitcnt - return the count of a given delimiter in a given string.
 * @s: The string to be searched.
 * @ct: The delimiter to search for.
 * Notice : We suppose the delimiter is a single-char string(for example : ";").
 */
INT delimitcnt(char *s,char *ct)
{
	INT count = 0;
	/* point to the beginning of the line */
	char *token = s;

	for ( ;; )
	{
		token = strpbrk(token, ct); /* search for delimiters */

        if ( token == NULL )
		{
			/* advanced to the terminating null character */
			break;
		}
		/* skip the delimiter */
	    ++token;

		/*
		 * Print the found text: use len with %.*s to specify field width.
		 */

		/* accumulate delimiter count */
	    ++count;
	}
    return count;
}

/*
  * converts the Internet host address from the standard numbers-and-dots notation
  * into binary data.
  * returns nonzero if the address is valid, zero if not.
  */
int rtinet_aton(const char *cp, unsigned int *addr)
{
	unsigned int 	val;
	int         	base, n;
	STRING        	c;
	unsigned int    parts[4];
	unsigned int    *pp = parts;

	for (;;)
    {
         /*
          * Collect number up to ``.''.
          * Values are specified as for C:
          *	0x=hex, 0=octal, other=decimal.
          */
         val = 0;
         base = 10;
         if (*cp == '0')
         {
             if (*++cp == 'x' || *cp == 'X')
                 base = 16, cp++;
             else
                 base = 8;
         }
         while ((c = *cp) != '\0')
         {
             if (isdigit((unsigned char) c))
             {
                 val = (val * base) + (c - '0');
                 cp++;
                 continue;
             }
             if (base == 16 && isxdigit((unsigned char) c))
             {
                 val = (val << 4) +
                     (c + 10 - (islower((unsigned char) c) ? 'a' : 'A'));
                 cp++;
                 continue;
             }
             break;
         }
         if (*cp == '.')
         {
             /*
              * Internet format: a.b.c.d a.b.c   (with c treated as 16-bits)
              * a.b     (with b treated as 24 bits)
              */
             if (pp >= parts + 3 || val > 0xff)
                 return 0;
             *pp++ = val, cp++;
         }
         else
             break;
     }

     /*
      * Check for trailing junk.
      */
     while (*cp)
         if (!isspace((unsigned char) *cp++))
             return 0;

     /*
      * Concoct the address according to the number of parts specified.
      */
     n = pp - parts + 1;
     switch (n)
     {

         case 1:         /* a -- 32 bits */
             break;

         case 2:         /* a.b -- 8.24 bits */
             if (val > 0xffffff)
                 return 0;
             val |= parts[0] << 24;
             break;

         case 3:         /* a.b.c -- 8.8.16 bits */
             if (val > 0xffff)
                 return 0;
             val |= (parts[0] << 24) | (parts[1] << 16);
             break;

         case 4:         /* a.b.c.d -- 8.8.8.8 bits */
             if (val > 0xff)
                 return 0;
             val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
             break;
     }

     *addr = OS_HTONL(val);
     return 1;

}

/*
    ========================================================================

    Routine Description:
        Find key section for Get key parameter.

    Arguments:
        buffer                      Pointer to the buffer to start find the key section
        section                     the key of the secion to be find

    Return Value:
        NULL                        Fail
        Others                      Success
    ========================================================================
*/
char *RTMPFindSection(
    IN  char *  buffer)
{
    STRING temp_buf[32];
    char * ptr;

    strcpy(temp_buf, "Default");

    if((ptr = rtstrstr(buffer, temp_buf)) != NULL)
            return (ptr+strlen("\n"));
        else
            return NULL;
}

/*
    ========================================================================

    Routine Description:
        Get key parameter.

    Arguments:
	key			Pointer to key string
	dest			Pointer to destination
	destsize		The datasize of the destination
	buffer		Pointer to the buffer to start find the key
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
        TRUE                        Success
        FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPGetKeyParameter(
    IN char *key,
    OUT char *dest,
    IN INT destsize,
    IN char *buffer,
    IN bool bTrimSpace)
{
	char *pMemBuf, *temp_buf1 = NULL, *temp_buf2 = NULL;
	char *start_ptr, *end_ptr;
	char *ptr;
	char *offset = NULL;
	INT  len, keyLen;


	keyLen = strlen(key);
	pMemBuf = kmalloc(MAX_PARAM_BUFFER_SIZE  * 2, GFP_ATOMIC);
	if (pMemBuf == NULL)
		return (FALSE);

	memset(pMemBuf, 0, MAX_PARAM_BUFFER_SIZE * 2);
	temp_buf1 = pMemBuf;
	temp_buf2 = (char *)(pMemBuf + MAX_PARAM_BUFFER_SIZE);


	/*find section*/
	if((offset = RTMPFindSection(buffer)) == NULL)
	{
		kfree((u8 *)pMemBuf);
		return (FALSE);
	}

	strcpy(temp_buf1, "\n");
	strcat(temp_buf1, key);
	strcat(temp_buf1, "=");

	/*search key*/
	if((start_ptr=rtstrstr(offset, temp_buf1)) == NULL)
	{
		kfree((u8 *)pMemBuf);
		return (FALSE);
	}

	start_ptr += strlen("\n");
	if((end_ptr = rtstrstr(start_ptr, "\n"))==NULL)
		end_ptr = start_ptr+strlen(start_ptr);

	if (end_ptr<start_ptr)
	{
		kfree((u8 *)pMemBuf);
		return (FALSE);
	}

	memmove(temp_buf2, start_ptr, end_ptr-start_ptr);
	temp_buf2[end_ptr-start_ptr]='\0';

	if((start_ptr=rtstrstr(temp_buf2, "=")) == NULL)
	{
		kfree((u8 *)pMemBuf);
		return (FALSE);
	}
	ptr = (start_ptr +1);
	/*trim special characters, i.e.,  TAB or space*/
	while(*start_ptr != 0x00)
	{
		if( ((*ptr == ' ') && bTrimSpace) || (*ptr == '\t') )
			ptr++;
		else
			break;
	}
	len = strlen(start_ptr);

	memset(dest, 0x00, destsize);
	strncpy(dest, ptr, ((len >= destsize) ? destsize: len));

	kfree((u8 *)pMemBuf);

	return TRUE;
}


/*
    ========================================================================

    Routine Description:
        Get multiple key parameter.

    Arguments:
        key                         Pointer to key string
        dest                        Pointer to destination
        destsize                    The datasize of the destination
        buffer                      Pointer to the buffer to start find the key

    Return Value:
        TRUE                        Success
        FALSE                       Fail

    Note:
        This routine get the value with the matched key (case case-sensitive)
    ========================================================================
*/
INT RTMPGetKeyParameterWithOffset(
    IN  char *  key,
    OUT char *  dest,
    OUT	USHORT	*end_offset,
    IN  INT     destsize,
    IN  char *  buffer,
    IN	bool	bTrimSpace)
{
    char *temp_buf1 = NULL;
    char *temp_buf2 = NULL;
    char *start_ptr;
    char *end_ptr;
    char *ptr;
    char *offset = 0;
    INT  len;

	if (*end_offset >= MAX_INI_BUFFER_SIZE)
		return (FALSE);

	temp_buf1 = kmalloc(MAX_PARAM_BUFFER_SIZE, GFP_ATOMIC);

	if(temp_buf1 == NULL)
		return (FALSE);

	temp_buf2 = kmalloc(MAX_PARAM_BUFFER_SIZE, GFP_ATOMIC);
	if(temp_buf2 == NULL) {
		kfree((u8 *)temp_buf1);
        return (FALSE);
	}

    /*find section		*/
	if(*end_offset == 0)
    {
		if ((offset = RTMPFindSection(buffer)) == NULL)
		{
			kfree((u8 *)temp_buf1);
	    	kfree((u8 *)temp_buf2);
    	    return (FALSE);
		}
    }
	else
		offset = buffer + (*end_offset);

    strcpy(temp_buf1, "\n");
    strcat(temp_buf1, key);
    strcat(temp_buf1, "=");

    /*search key*/
    if((start_ptr=rtstrstr(offset, temp_buf1))==NULL)
    {
		kfree((u8 *)temp_buf1);
    	kfree((u8 *)temp_buf2);
        return (FALSE);
    }

    start_ptr+=strlen("\n");
    if((end_ptr=rtstrstr(start_ptr, "\n"))==NULL)
       end_ptr=start_ptr+strlen(start_ptr);

    if (end_ptr<start_ptr)
    {
		kfree((u8 *)temp_buf1);
    	kfree((u8 *)temp_buf2);
        return (FALSE);
    }

	*end_offset = end_ptr - buffer;

    memmove(temp_buf2, start_ptr, end_ptr-start_ptr);
    temp_buf2[end_ptr-start_ptr]='\0';
    len = strlen(temp_buf2);
    strcpy(temp_buf1, temp_buf2);
    if((start_ptr=rtstrstr(temp_buf1, "=")) == NULL)
    {
		kfree((u8 *)temp_buf1);
    	kfree((u8 *)temp_buf2);
        return (FALSE);
    }

    strcpy(temp_buf2, start_ptr+1);
    ptr = temp_buf2;
    /*trim space or tab*/
    while(*ptr != 0x00)
    {
        if((bTrimSpace && (*ptr == ' ')) || (*ptr == '\t') )
            ptr++;
        else
           break;
    }

    len = strlen(ptr);
    memset(dest, 0x00, destsize);
    strncpy(dest, ptr, len >= destsize ?  destsize: len);

	kfree((u8 *)temp_buf1);
    kfree((u8 *)temp_buf2);
    return TRUE;
}


#ifdef CONFIG_STA_SUPPORT
inline void RTMPSetSTADefKeyId(struct rtmp_adapter *pAd, ULONG KeyIdx)
{
	if((KeyIdx >= 1 ) && (KeyIdx <= 4))
		pAd->StaCfg.wdev.DefaultKeyId = (UCHAR) (KeyIdx - 1);
	else
		pAd->StaCfg.wdev.DefaultKeyId = 0;
}
#endif /* CONFIG_STA_SUPPORT */


static int rtmp_parse_key_buffer_from_file(IN  struct rtmp_adapter *pAd,IN  char *buffer,IN  ULONG KeyType,IN  INT BSSIdx,IN  INT KeyIdx)
{
	char *	keybuff;
	/*INT			i = BSSIdx, idx = KeyIdx, retVal;*/
	ULONG		KeyLen;
	/*UCHAR		CipherAlg = CIPHER_WEP64;*/
	CIPHER_KEY	*pSharedKey;

	keybuff = buffer;
	KeyLen = strlen(keybuff);
	pSharedKey = &pAd->SharedKey[BSSIdx][KeyIdx];

	if(((KeyType != 0) && (KeyType != 1)) ||
	    ((KeyType == 0) && (KeyLen != 10) && (KeyLen != 26)) ||
	    ((KeyType== 1) && (KeyLen != 5) && (KeyLen != 13)))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Key%dStr is Invalid key length(%ld) or Type(%ld)\n",
								KeyIdx+1, KeyLen, KeyType));
		return FALSE;
	}
	else
	{
		return RT_CfgSetWepKey(pAd, buffer, pSharedKey, KeyIdx);
	}

}


static void rtmp_read_key_parms_from_file(IN  struct rtmp_adapter *pAd, char *tmpbuf, char *buffer)
{
	STRING		tok_str[16];
	char *	macptr;
	INT			i = 0, idx;
	ULONG		KeyType[HW_BEACON_MAX_NUM];
	ULONG		KeyIdx;

	memset(KeyType, 0, sizeof(KeyType));

	/*DefaultKeyID*/
	if(RTMPGetKeyParameter("DefaultKeyID", tmpbuf, 25, buffer, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			{
				if (i >= pAd->ApCfg.BssidNum)
					break;

				KeyIdx = simple_strtol(macptr, 0, 10);
				if((KeyIdx >= 1 ) && (KeyIdx <= 4))
					pAd->ApCfg.MBSSID[i].wdev.DefaultKeyId = (UCHAR) (KeyIdx - 1 );
				else
					pAd->ApCfg.MBSSID[i].wdev.DefaultKeyId = 0;

				DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) DefaultKeyID(0~3)=%d\n", i, pAd->ApCfg.MBSSID[i].wdev.DefaultKeyId));
			}
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			KeyIdx = simple_strtol(tmpbuf, 0, 10);
			RTMPSetSTADefKeyId(pAd, KeyIdx);

			DBGPRINT(RT_DEBUG_TRACE, ("DefaultKeyID(0~3)=%d\n", pAd->StaCfg.wdev.DefaultKeyId));
		}
#endif /* CONFIG_STA_SUPPORT */
	}


	for (idx = 0; idx < 4; idx++)
	{
		snprintf(tok_str, sizeof(tok_str), "Key%dType", idx + 1);
		/*Key1Type*/
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				/*
					do sanity check for KeyType length;
					or in station mode, the KeyType length > 1,
					the code will overwrite the stack of caller
					(RTMPSetProfileParameters) and cause srcbuf = NULL
				*/
				if (i < MAX_MBSSID_NUM(pAd))
					KeyType[i] = simple_strtol(macptr, 0, 10);
		    }
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				if (TRUE)
				{
					bool bKeyxStryIsUsed = FALSE;

					//GPRINT(RT_DEBUG_TRACE, ("pAd->ApCfg.BssidNum=%d\n", pAd->ApCfg.BssidNum));
					for (i = 0; i < pAd->ApCfg.BssidNum; i++)
			        	{
						snprintf(tok_str, sizeof(tok_str), "Key%dStr%d", idx + 1, i + 1);
					if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, FALSE))
						{
							rtmp_parse_key_buffer_from_file(pAd, tmpbuf, KeyType[i], i, idx);

							if (bKeyxStryIsUsed == FALSE)
							{
								bKeyxStryIsUsed = TRUE;
							}
						}
					}

					if (bKeyxStryIsUsed == FALSE)
					{
						snprintf(tok_str, sizeof(tok_str), "Key%dStr", idx + 1);
					if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, FALSE))
						{
							if (pAd->ApCfg.BssidNum == 1)
							{
								rtmp_parse_key_buffer_from_file(pAd, tmpbuf, KeyType[BSS0], BSS0, idx);
							}
							else
							{
								/* Anyway, we still do the legacy dissection of the whole KeyxStr string.*/
							    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
							    {
									rtmp_parse_key_buffer_from_file(pAd, macptr, KeyType[i], i, idx);
							    }
							}
						}
					}
				}
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				snprintf(tok_str, sizeof(tok_str), "Key%dStr", idx + 1);
				if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, FALSE))
				{
					rtmp_parse_key_buffer_from_file(pAd, tmpbuf, KeyType[BSS0], BSS0, idx);
				}
			}
#endif /* CONFIG_STA_SUPPORT */
		}
	}
}

#ifdef CONFIG_AP_SUPPORT

#ifdef APCLI_SUPPORT
static void rtmp_read_ap_client_from_file(
	IN struct rtmp_adapter *pAd,
	IN char *tmpbuf,
	IN char *buffer)
{
	char *	macptr = NULL;
	INT			i=0, j=0, idx;
	UCHAR		macAddress[MAC_ADDR_LEN];
	PAPCLI_STRUCT   pApCliEntry = NULL;
	ULONG		KeyIdx;
	STRING		tok_str[16];
	ULONG		KeyType[MAX_APCLI_NUM];
	ULONG		KeyLen;
	struct rtmp_wifi_dev *wdev;

	memset(KeyType, 0, sizeof(KeyType));

	/*ApCliEnable*/
	if(RTMPGetKeyParameter("ApCliEnable", tmpbuf, 128, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			if ((strncmp(macptr, "0", 1) == 0))
				pApCliEntry->Enable = FALSE;
			else if ((strncmp(macptr, "1", 1) == 0))
				pApCliEntry->Enable = TRUE;
	        else
				pApCliEntry->Enable = FALSE;

			if (pApCliEntry->Enable)
			{
				/*pApCliEntry->WpaState = SS_NOTUSE;*/
				/*pApCliEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;*/
				/*memset(pApCliEntry->ReplayCounter, 0, LEN_KEY_DESC_REPLAY); */
			}
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliEntry[%d].Enable=%d\n", i, pApCliEntry->Enable));
	    }
	}

#ifdef APCLI_CONNECTION_TRIAL
	if(RTMPGetKeyParameter("ApCliTrialCh", tmpbuf, 128, buffer, TRUE))
	{
		for (i = 0; i < MAX_APCLI_NUM; i++)
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			pApCliEntry->TrialCh = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			DBGPRINT(RT_DEBUG_TRACE, ("TrialChannel=%d\n", pApCliEntry->TrialCh));
		}

	}
#endif /* APCLI_CONNECTION_TRIAL */

	/*ApCliSsid*/
	if(RTMPGetKeyParameter("ApCliSsid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE))
	{
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			/*Ssid acceptable strlen must be less than 32 and bigger than 0.*/
			pApCliEntry->CfgSsidLen = (UCHAR)strlen(macptr);
			if (pApCliEntry->CfgSsidLen > 32)
			{
				pApCliEntry->CfgSsidLen = 0;
				continue;
			}
			if(pApCliEntry->CfgSsidLen > 0)
			{
				memcpy(&pApCliEntry->CfgSsid, macptr, pApCliEntry->CfgSsidLen);
				pApCliEntry->Valid = FALSE;/* it should be set when successfuley association*/
			} else
			{
				memset(&(pApCliEntry->CfgSsid), 0, MAX_LEN_OF_SSID);
				continue;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliEntry[%d].CfgSsidLen=%d, CfgSsid=%s\n", i, pApCliEntry->CfgSsidLen, pApCliEntry->CfgSsid));
		}
	}

	/*ApCliBssid*/
	if(RTMPGetKeyParameter("ApCliBssid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE))
	{
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			if(strlen(macptr) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17*/
				continue;
			if(strcmp(macptr,"00:00:00:00:00:00") == 0)
				continue;
			for (j=0; j<MAC_ADDR_LEN; j++)
			{
				AtoH(macptr, &macAddress[j], 1);
				macptr=macptr+3;
			}
			memcpy(pApCliEntry->CfgApCliBssid, &macAddress, MAC_ADDR_LEN);
			pApCliEntry->Valid = FALSE;/* it should be set when successfuley association*/
		}
	}

	/*ApCliAuthMode*/
	if (RTMPGetKeyParameter("ApCliAuthMode", tmpbuf, 255, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			wdev = &pApCliEntry->wdev;

			if ((strncmp(macptr, "WEPAUTO", 7) == 0) || (strncmp(macptr, "wepauto", 7) == 0))
				wdev->AuthMode = Ndis802_11AuthModeAutoSwitch;
			else if ((strncmp(macptr, "SHARED", 6) == 0) || (strncmp(macptr, "shared", 6) == 0))
				wdev->AuthMode = Ndis802_11AuthModeShared;
			else if ((strncmp(macptr, "WPAPSK", 6) == 0) || (strncmp(macptr, "wpapsk", 6) == 0))
				wdev->AuthMode = Ndis802_11AuthModeWPAPSK;
			else if ((strncmp(macptr, "WPA2PSK", 7) == 0) || (strncmp(macptr, "wpa2psk", 7) == 0))
				wdev->AuthMode = Ndis802_11AuthModeWPA2PSK;
			else
				wdev->AuthMode = Ndis802_11AuthModeOpen;

			/*pApCliEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;*/

			DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) ApCli_AuthMode=%d \n", i, wdev->AuthMode));
			RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, (i + MIN_NET_DEVICE_FOR_APCLI));
		}

	}

	/*ApCliEncrypType*/
	if (RTMPGetKeyParameter("ApCliEncrypType", tmpbuf, 255, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];
			wdev = &pApCliEntry->wdev;

			wdev->WepStatus = Ndis802_11WEPDisabled;
			if ((strncmp(macptr, "WEP", 3) == 0) || (strncmp(macptr, "wep", 3) == 0))
            {
				if (wdev->AuthMode < Ndis802_11AuthModeWPA)
					wdev->WepStatus = Ndis802_11WEPEnabled;
			}
			else if ((strncmp(macptr, "TKIP", 4) == 0) || (strncmp(macptr, "tkip", 4) == 0))
			{
				if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
					wdev->WepStatus = Ndis802_11Encryption2Enabled;
            }
			else if ((strncmp(macptr, "AES", 3) == 0) || (strncmp(macptr, "aes", 3) == 0))
			{
				if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
					wdev->WepStatus = Ndis802_11Encryption3Enabled;
			}
			else
			{
				wdev->WepStatus      = Ndis802_11WEPDisabled;
			}

			pApCliEntry->PairCipher     = wdev->WepStatus;
			pApCliEntry->GroupCipher    = wdev->WepStatus;
			pApCliEntry->bMixCipher		= FALSE;

			/*pApCliEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;*/

			DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) APCli_EncrypType = %d \n", i, wdev->WepStatus));
			RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, (i + MIN_NET_DEVICE_FOR_APCLI));
		}

	}

	/*ApCliWPAPSK*/
	if (RTMPGetKeyParameter("ApCliWPAPSK", tmpbuf, 255, buffer, FALSE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			int retval = TRUE;

			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			if((strlen(macptr) < 8) || (strlen(macptr) > 64))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("APCli_WPAPSK_KEY, key string required 8 ~ 64 characters!!!\n"));
				continue;
			}

			memmove(pApCliEntry->PSK, macptr, strlen(macptr));
			pApCliEntry->PSKLen = strlen(macptr);
			DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) APCli_WPAPSK_KEY=%s, Len=%d\n", i, pApCliEntry->PSK, pApCliEntry->PSKLen));

			if ((pApCliEntry->wdev.AuthMode != Ndis802_11AuthModeWPAPSK) &&
				(pApCliEntry->wdev.AuthMode != Ndis802_11AuthModeWPA2PSK))
			{
				retval = FALSE;
			}

			{
				retval = RT_CfgSetWPAPSKKey(pAd, macptr, strlen(macptr), (u8 *)pApCliEntry->CfgSsid, (INT)pApCliEntry->CfgSsidLen, pApCliEntry->PMK);
			}
			if (retval == TRUE)
			{
				/* Start STA supplicant WPA state machine*/
				DBGPRINT(RT_DEBUG_TRACE, ("Start AP-client WPAPSK state machine \n"));
				/*pApCliEntry->WpaState = SS_START;				*/
			}

			/*RTMPMakeRSNIE(pAd, pApCliEntry->AuthMode, pApCliEntry->WepStatus, (i + MIN_NET_DEVICE_FOR_APCLI));			*/
#ifdef DBG
			DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) PMK Material => \n", i));

			for (j = 0; j < 32; j++)
			{
				DBGPRINT(RT_DEBUG_OFF, ("%02x:", pApCliEntry->PMK[j]));
				if ((j%16) == 15)
					DBGPRINT(RT_DEBUG_OFF, ("\n"));
			}
			DBGPRINT(RT_DEBUG_OFF,("\n"));
#endif
		}
	}

	/*ApCliDefaultKeyID*/
	if (RTMPGetKeyParameter("ApCliDefaultKeyID", tmpbuf, 255, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			wdev = &pAd->ApCfg.ApCliTab[i].wdev;

			KeyIdx = simple_strtol(macptr, 0, 10);
			if((KeyIdx >= 1 ) && (KeyIdx <= 4))
				wdev->DefaultKeyId = (UCHAR) (KeyIdx - 1);
			else
				wdev->DefaultKeyId = 0;

			DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) DefaultKeyID(0~3)=%d\n", i, wdev->DefaultKeyId));
		}
	}

	/*ApCliKeyXType, ApCliKeyXStr*/
	for (idx=0; idx<4; idx++)
	{
		snprintf(tok_str, sizeof(tok_str),  "ApCliKey%dType", idx+1);
		/*ApCliKey1Type*/
		if(RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE))
		{
			for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
			{
			    KeyType[i] = simple_strtol(macptr, 0, 10);
			}

			snprintf(tok_str, sizeof(tok_str), "ApCliKey%dStr", idx+1);
			/*ApCliKey1Str*/
			if(RTMPGetKeyParameter(tok_str, tmpbuf, 512, buffer, FALSE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
				{
					pApCliEntry = &pAd->ApCfg.ApCliTab[i];
					KeyLen = strlen(macptr);
					if(((KeyType[i] == 0) && (KeyLen != 10) && (KeyLen != 26)) ||
					    ((KeyType[i] != 0) && (KeyLen != 5) && (KeyLen != 13)))
					{
						DBGPRINT(RT_DEBUG_ERROR, ("I/F(apcli%d) Key%dStr is Invalid key length!\n", i, idx+1));
					}
					else
					{
						if (RT_CfgSetWepKey(pAd, macptr, &pApCliEntry->SharedKey[idx], idx) != TRUE)
							DBGPRINT(RT_DEBUG_ERROR, ("RT_CfgSetWepKey fail!\n"));
					}
				}
			}
		}
	}

	/* ApCliTxMode*/
	if (RTMPGetKeyParameter("ApCliTxMode", tmpbuf, 25, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			wdev = &pAd->ApCfg.ApCliTab[i].wdev;

			wdev->DesiredTransmitSetting.field.FixedTxMode =
										RT_CfgSetFixedTxPhyMode(macptr);
			DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) Tx Mode = %d\n", i,
											wdev->DesiredTransmitSetting.field.FixedTxMode));
		}
	}

	/* ApCliTxMcs*/
	if (RTMPGetKeyParameter("ApCliTxMcs", tmpbuf, 50, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL,";"), i++)
		{
			wdev = &pAd->ApCfg.ApCliTab[i].wdev;

			wdev->DesiredTransmitSetting.field.MCS =
					RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);

			DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) Tx MCS = %s(%d)\n", i,
						(wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : ""),
						wdev->DesiredTransmitSetting.field.MCS));
		}
	}




#ifdef UAPSD_SUPPORT
	/*APSDCapable*/
	if(RTMPGetKeyParameter("ApCliAPSDCapable", tmpbuf, 10, buffer, TRUE))
	{
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = TRUE;

		for (i = 0, macptr = rstrtok(tmpbuf,";");
			(macptr && i < MAX_APCLI_NUM);
			macptr = rstrtok(NULL,";"), i++)
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[i];

			pApCliEntry->UapsdInfo.bAPSDCapable = \
									(UCHAR) simple_strtol(macptr, 0, 10);
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliAPSDCapable[%d]=%d\n", i,
					pApCliEntry->UapsdInfo.bAPSDCapable));
	    }
	}
#endif /* UAPSD_SUPPORT */

	/* ApCliNum */
	if(RTMPGetKeyParameter("ApCliNum", tmpbuf, 10, buffer, TRUE))
	{
		if (simple_strtol(tmpbuf, 0, 10) <= MAX_APCLI_NUM)
		{
			pAd->ApCfg.ApCliNum = simple_strtol(tmpbuf, 0, 10);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli) ApCliNum=%d\n", pAd->ApCfg.ApCliNum));
	}
}
#endif /* APCLI_SUPPORT */


static void rtmp_read_acl_parms_from_file(IN  struct rtmp_adapter *pAd, char *tmpbuf, char *buffer)
{
	STRING		tok_str[32];
	char *	macptr;
	INT			i=0, j=0, idx;
	UCHAR		macAddress[MAC_ADDR_LEN];


	memset(macAddress, 0, MAC_ADDR_LEN);
	for (idx=0; idx<MAX_MBSSID_NUM(pAd); idx++)
	{
		memset(&pAd->ApCfg.MBSSID[idx].AccessControlList, 0, sizeof(RT_802_11_ACL));
		/* AccessPolicyX*/
		snprintf(tok_str, sizeof(tok_str), "AccessPolicy%d", idx);
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 10, buffer, TRUE))
		{
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case 1: /* Allow All, and the AccessControlList is positive now.*/
					pAd->ApCfg.MBSSID[idx].AccessControlList.Policy = 1;
					break;
				case 2: /* Reject All, and the AccessControlList is negative now.*/
					pAd->ApCfg.MBSSID[idx].AccessControlList.Policy = 2;
					break;
				case 0: /* Disable, don't care the AccessControlList.*/
				default:
					pAd->ApCfg.MBSSID[idx].AccessControlList.Policy = 0;
					break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("%s=%ld\n", tok_str, pAd->ApCfg.MBSSID[idx].AccessControlList.Policy));
		}
		/* AccessControlListX*/
		snprintf(tok_str, sizeof(tok_str), "AccessControlList%d", idx);
		if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE))
		{
			for (i=0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			{
				if (strlen(macptr) != 17)  /* Mac address acceptable format 01:02:03:04:05:06 length 17*/
					continue;

				ASSERT(pAd->ApCfg.MBSSID[idx].AccessControlList.Num <= MAX_NUM_OF_ACL_LIST);

				for (j=0; j<MAC_ADDR_LEN; j++)
				{
					AtoH(macptr, &macAddress[j], 1);
					macptr=macptr+3;
				}

				if (pAd->ApCfg.MBSSID[idx].AccessControlList.Num == MAX_NUM_OF_ACL_LIST)
				{
					DBGPRINT(RT_DEBUG_WARN, ("The AccessControlList is full, and no more entry can join the list!\n"));
        			DBGPRINT(RT_DEBUG_WARN, ("The last entry of ACL is %02x:%02x:%02x:%02x:%02x:%02x\n",
        				macAddress[0],macAddress[1],macAddress[2],macAddress[3],macAddress[4],macAddress[5]));

				    break;
				}

				pAd->ApCfg.MBSSID[idx].AccessControlList.Num++;
				memmove(pAd->ApCfg.MBSSID[idx].AccessControlList.Entry[(pAd->ApCfg.MBSSID[idx].AccessControlList.Num - 1)].Addr, macAddress, MAC_ADDR_LEN);
			}
			DBGPRINT(RT_DEBUG_TRACE, ("%s=Get %ld Mac Address\n", tok_str, pAd->ApCfg.MBSSID[idx].AccessControlList.Num));
 		}
	}
}

/*
    ========================================================================

    Routine Description:
        In kernel mode read parameters from file

    Arguments:
        src                     the location of the file.
        dest                        put the parameters to the destination.
        Length                  size to read.

    Return Value:
        None

    Note:

    ========================================================================
*/
static void rtmp_read_ap_wmm_parms_from_file(IN  struct rtmp_adapter *pAd, char *tmpbuf, char *buffer)
{
	char *				macptr;
	INT						i=0;

	/*WmmCapable*/
	if(RTMPGetKeyParameter("WmmCapable", tmpbuf, 32, buffer, TRUE))
	{
	    bool bEnableWmm = FALSE;
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			if (i >= pAd->ApCfg.BssidNum)
			{
				break;
			}

			if(simple_strtol(macptr, 0, 10) != 0)
			{
				pAd->ApCfg.MBSSID[i].wdev.bWmmCapable = TRUE;
				bEnableWmm = TRUE;
#ifdef MULTI_CLIENT_SUPPORT
				pAd->CommonCfg.bWmm = TRUE;
#endif /* MULTI_CLIENT_SUPPORT */
			}
			else
			{
				pAd->ApCfg.MBSSID[i].wdev.bWmmCapable = FALSE;
			}

			if (bEnableWmm)
			{
				pAd->CommonCfg.APEdcaParm.bValid = TRUE;
				pAd->ApCfg.BssEdcaParm.bValid = TRUE;
			}
			else
			{
				pAd->CommonCfg.APEdcaParm.bValid = FALSE;
				pAd->ApCfg.BssEdcaParm.bValid = FALSE;
			}

			pAd->ApCfg.MBSSID[i].bWmmCapableOrg = \
											pAd->ApCfg.MBSSID[i].wdev.bWmmCapable;

			DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) WmmCapable=%d\n", i, pAd->ApCfg.MBSSID[i].wdev.bWmmCapable));
	    }
	}
	/*DLSCapable*/
	if(RTMPGetKeyParameter("DLSCapable", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			if (i >= pAd->ApCfg.BssidNum)
			{
				break;
			}

			if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
			{
				pAd->ApCfg.MBSSID[i].bDLSCapable = TRUE;
			}
			else /*Disable*/
			{
				pAd->ApCfg.MBSSID[i].bDLSCapable = FALSE;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) DLSCapable=%d\n", i, pAd->ApCfg.MBSSID[i].bDLSCapable));
	    }
	}
	/*APAifsn*/
	if(RTMPGetKeyParameter("APAifsn", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm.Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

			DBGPRINT(RT_DEBUG_TRACE, ("APAifsn[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm.Aifsn[i]));
	    }
	}
	/*APCwmin*/
	if(RTMPGetKeyParameter("APCwmin", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm.Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

#ifdef MULTI_CLIENT_SUPPORT
			/* record profile cwmin */
			if (i == 0)
				pAd->CommonCfg.APCwmin = pAd->CommonCfg.APEdcaParm.Cwmin[0];
#endif /* MULTI_CLIENT_SUPPORT */

			DBGPRINT(RT_DEBUG_TRACE, ("APCwmin[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm.Cwmin[i]));
	    }
	}
	/*APCwmax*/
	if(RTMPGetKeyParameter("APCwmax", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm.Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

#ifdef MULTI_CLIENT_SUPPORT
			/* record profile cwmax */
			if (i == 0)
				pAd->CommonCfg.APCwmax= pAd->CommonCfg.APEdcaParm.Cwmax[0];
#endif /* MULTI_CLIENT_SUPPORT */

			DBGPRINT(RT_DEBUG_TRACE, ("APCwmax[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm.Cwmax[i]));
	    }
	}
	/*APTxop*/
	if(RTMPGetKeyParameter("APTxop", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm.Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);;

			DBGPRINT(RT_DEBUG_TRACE, ("APTxop[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm.Txop[i]));
	    }
	}
	/*APACM*/
	if(RTMPGetKeyParameter("APACM", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.APEdcaParm.bACM[i] = (bool) simple_strtol(macptr, 0, 10);;

			DBGPRINT(RT_DEBUG_TRACE, ("APACM[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm.bACM[i]));
	    }
	}
	/*BSSAifsn*/
	if(RTMPGetKeyParameter("BSSAifsn", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->ApCfg.BssEdcaParm.Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

			DBGPRINT(RT_DEBUG_TRACE, ("BSSAifsn[%d]=%d\n", i, pAd->ApCfg.BssEdcaParm.Aifsn[i]));
	    }
	}
	/*BSSCwmin*/
	if(RTMPGetKeyParameter("BSSCwmin", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->ApCfg.BssEdcaParm.Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

#ifdef MULTI_CLIENT_SUPPORT
			/* record profile cwmin */
			if (i == 0)
				pAd->CommonCfg.BSSCwmin = pAd->ApCfg.BssEdcaParm.Cwmin[0];
#endif /* MULTI_CLIENT_SUPPORT */

			DBGPRINT(RT_DEBUG_TRACE, ("BSSCwmin[%d]=%d\n", i, pAd->ApCfg.BssEdcaParm.Cwmin[i]));
	    }
	}
	/*BSSCwmax*/
	if(RTMPGetKeyParameter("BSSCwmax", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->ApCfg.BssEdcaParm.Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

			DBGPRINT(RT_DEBUG_TRACE, ("BSSCwmax[%d]=%d\n", i, pAd->ApCfg.BssEdcaParm.Cwmax[i]));
	    }
	}
	/*BSSTxop*/
	if(RTMPGetKeyParameter("BSSTxop", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->ApCfg.BssEdcaParm.Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);;

			DBGPRINT(RT_DEBUG_TRACE, ("BSSTxop[%d]=%d\n", i, pAd->ApCfg.BssEdcaParm.Txop[i]));
	    }
	}
	/*BSSACM*/
	if(RTMPGetKeyParameter("BSSACM", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->ApCfg.BssEdcaParm.bACM[i] = (bool) simple_strtol(macptr, 0, 10);;

			DBGPRINT(RT_DEBUG_TRACE, ("BSSACM[%d]=%d\n", i, pAd->ApCfg.BssEdcaParm.bACM[i]));
	    }
	}
	/*AckPolicy*/
	if(RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			pAd->CommonCfg.AckPolicy[i] = (UCHAR) simple_strtol(macptr, 0, 10);;

			DBGPRINT(RT_DEBUG_TRACE, ("AckPolicy[%d]=%d\n", i, pAd->CommonCfg.AckPolicy[i]));
	    }
	}
#ifdef UAPSD_SUPPORT
	/*APSDCapable*/
	if(RTMPGetKeyParameter("APSDCapable", tmpbuf, 10, buffer, TRUE))
	{

	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			if (i < HW_BEACON_MAX_NUM)
			{
				pAd->ApCfg.MBSSID[i].UapsdInfo.bAPSDCapable = \
										(UCHAR) simple_strtol(macptr, 0, 10);
				DBGPRINT(RT_DEBUG_ERROR, ("APSDCapable[%d]=%d\n", i,
						pAd->ApCfg.MBSSID[i].UapsdInfo.bAPSDCapable));
			}
	    }

		if (i == 1)
		{
			/*
				Old format in UAPSD settings: only 1 parameter
				i.e. UAPSD for all BSS is enabled or disabled.
			*/
			for(i=1; i<HW_BEACON_MAX_NUM; i++)
			{
				pAd->ApCfg.MBSSID[i].UapsdInfo.bAPSDCapable =
							pAd->ApCfg.MBSSID[0].UapsdInfo.bAPSDCapable;
				DBGPRINT(RT_DEBUG_ERROR, ("APSDCapable[%d]=%d\n", i,
						pAd->ApCfg.MBSSID[i].UapsdInfo.bAPSDCapable));
			}
		}

#ifdef APCLI_SUPPORT
		if (pAd->ApCfg.FlgApCliIsUapsdInfoUpdated == FALSE)
		{
			/*
				Backward:
				All UAPSD for AP Client interface is same as MBSS0
				when we can not find "ApCliAPSDCapable".
				When we find "ApCliAPSDCapable" hereafter, we will over-write.
			*/
			for(i=0; i<MAX_APCLI_NUM; i++)
			{
				pAd->ApCfg.ApCliTab[i].UapsdInfo.bAPSDCapable = \
								pAd->ApCfg.MBSSID[0].UapsdInfo.bAPSDCapable;
				DBGPRINT(RT_DEBUG_ERROR, ("default ApCliAPSDCapable[%d]=%d\n",
						i, pAd->ApCfg.ApCliTab[i].UapsdInfo.bAPSDCapable));
			}
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* UAPSD_SUPPORT */
}

#ifdef DOT1X_SUPPORT
/*
    ========================================================================

    Routine Description:
        In kernel mode read parameters from file

    Arguments:
        src                     the location of the file.
        dest                        put the parameters to the destination.
        Length                  size to read.

    Return Value:
        None

    Note:

    ========================================================================
*/
static void rtmp_read_radius_parms_from_file(IN  struct rtmp_adapter *pAd, char *tmpbuf, char *buffer)
{
	STRING					tok_str[16];
	char *				macptr;
	uint32_t 				ip_addr;
	INT						i=0;
	bool					bUsePrevFormat = FALSE;
	USHORT					offset;
	INT						count[HW_BEACON_MAX_NUM];

	/* own_ip_addr*/
	if (RTMPGetKeyParameter("own_ip_addr", tmpbuf, 32, buffer, TRUE))
	{
		Set_OwnIPAddr_Proc(pAd, tmpbuf);
	}


	/* session_timeout_interval*/
	if (RTMPGetKeyParameter("session_timeout_interval", tmpbuf, 32, buffer, TRUE))
	{
		pAd->ApCfg.session_timeout_interval = simple_strtol(tmpbuf, 0, 10);
		DBGPRINT(RT_DEBUG_TRACE, ("session_timeout_interval=%d\n", pAd->ApCfg.session_timeout_interval));
	}

	/* quiet_interval*/
	if (RTMPGetKeyParameter("quiet_interval", tmpbuf, 32, buffer, TRUE))
	{
		pAd->ApCfg.quiet_interval = simple_strtol(tmpbuf, 0, 10);
		DBGPRINT(RT_DEBUG_TRACE, ("quiet_interval=%d\n", pAd->ApCfg.quiet_interval));
	}

	/* EAPifname*/
	if (RTMPGetKeyParameter("EAPifname", tmpbuf, 256, buffer, TRUE))
	{
		Set_EAPIfName_Proc(pAd, tmpbuf);
	}

	/* PreAuthifname*/
	if (RTMPGetKeyParameter("PreAuthifname", tmpbuf, 256, buffer, TRUE))
	{
		Set_PreAuthIfName_Proc(pAd, tmpbuf);
	}

	/*PreAuth*/
	if(RTMPGetKeyParameter("PreAuth", tmpbuf, 10, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
				pAd->ApCfg.MBSSID[i].PreAuth = TRUE;
			else /*Disable*/
				pAd->ApCfg.MBSSID[i].PreAuth = FALSE;

			DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) PreAuth=%d\n", i, pAd->ApCfg.MBSSID[i].PreAuth));
	    }
	}

	/*IEEE8021X*/
	if(RTMPGetKeyParameter("IEEE8021X", tmpbuf, 32, buffer, TRUE))
	{
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	    {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
				pAd->ApCfg.MBSSID[i].wdev.IEEE8021X = TRUE;
			else /*Disable*/
				pAd->ApCfg.MBSSID[i].wdev.IEEE8021X = FALSE;

			DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d), IEEE8021X=%d\n", i, pAd->ApCfg.MBSSID[i].wdev.IEEE8021X));
	    }
	}

	/* RADIUS_Server*/
	offset = 0;
	/*if (RTMPGetKeyParameter("RADIUS_Server", tmpbuf, 256, buffer, TRUE))*/
	while (RTMPGetKeyParameterWithOffset("RADIUS_Server", tmpbuf, &offset, 256, buffer, TRUE))
	{
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL,";"), i++)
		{
			if (rtinet_aton(macptr, &ip_addr) && pAd->ApCfg.MBSSID[i].radius_srv_num < MAX_RADIUS_SRV_NUM)
			{
				INT	srv_idx = pAd->ApCfg.MBSSID[i].radius_srv_num;

				pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_ip = ip_addr;
				pAd->ApCfg.MBSSID[i].radius_srv_num++;
				DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d), radius_ip(seq-%d)=%s(%x)\n", i, pAd->ApCfg.MBSSID[i].radius_srv_num, macptr, pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_ip));
			}
		}
	}
	/* RADIUS_Port*/
	/*if (RTMPGetKeyParameter("RADIUS_Port", tmpbuf, 128, buffer, TRUE))*/
	offset = 0;
	memset(&count[0], 0, sizeof(count));
	while (RTMPGetKeyParameterWithOffset("RADIUS_Port", tmpbuf, &offset, 128, buffer, TRUE))
	{
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL,";"), i++)
		{
			if (count[i] < pAd->ApCfg.MBSSID[i].radius_srv_num)
			{
				INT		srv_idx = count[i];

            	pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_port = (uint32_t) simple_strtol(macptr, 0, 10);
				count[i] ++;
				DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d), radius_port(seq-%d)=%d\n", i, count[i], pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_port));
			}
		}
	}
	/* RADIUS_Key*/
	/*if (RTMPGetKeyParameter("RADIUS_Key", tmpbuf, 640, buffer, FALSE))*/
	offset = 0;
	memset(&count[0], 0, sizeof(count));
	while (RTMPGetKeyParameterWithOffset("RADIUS_Key", tmpbuf, &offset, 640, buffer, FALSE))
	{
		if (strlen(tmpbuf) > 0)
			bUsePrevFormat = TRUE;

		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL,";"), i++)
		{
			if (strlen(macptr) > 0 && (count[i] < pAd->ApCfg.MBSSID[i].radius_srv_num))
			{
				INT		srv_idx = count[i];

				pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_key_len = strlen(macptr);
				memmove(pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_key, macptr, strlen(macptr));
				count[i] ++;
				DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d), radius_key(seq-%d)=%s, len=%d\n", i,
															count[i],
															pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_key,
															pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_key_len));
			}
		}
	}

	/* NasIdX, X indicate the interface index(1~8) */
	for (i = 0; i < pAd->ApCfg.BssidNum; i++)
	{
		snprintf(tok_str, sizeof(tok_str), "NasId%d", i + 1);
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, buffer, FALSE))
		{
			if (strlen(tmpbuf) > 0)
			{
				pAd->ApCfg.MBSSID[i].NasIdLen = strlen(tmpbuf);
				memmove(pAd->ApCfg.MBSSID[i].NasId, tmpbuf, strlen(tmpbuf));
				DBGPRINT(RT_DEBUG_TRACE, ("IF-ra%d NAS-ID=%s, len=%d\n", i,
												pAd->ApCfg.MBSSID[i].NasId,
												pAd->ApCfg.MBSSID[i].NasIdLen));
			}
		}
	}

	if (!bUsePrevFormat)
	{
		for (i = 0; i < MAX_MBSSID_NUM(pAd); i++)
		{
			INT	srv_idx = 0;

			snprintf(tok_str, sizeof(tok_str), "RADIUS_Key%d", i + 1);

			/* RADIUS_KeyX (X=1~MAX_MBSSID_NUM)*/
			/*if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, FALSE))			*/
			offset = 0;
			while (RTMPGetKeyParameterWithOffset(tok_str, tmpbuf, &offset, 128, buffer, FALSE))
			{
				if (strlen(tmpbuf) > 0 && (srv_idx < pAd->ApCfg.MBSSID[i].radius_srv_num))
				{
					pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_key_len = strlen(tmpbuf);
					memmove(pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_key, tmpbuf, strlen(tmpbuf));
					DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d), update radius_key(seq-%d)=%s, len=%d\n", i, srv_idx+1,
																pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_key,
																pAd->ApCfg.MBSSID[i].radius_srv_info[srv_idx].radius_key_len));
					srv_idx ++;
				}
			}
		}
	}
}
#endif /* DOT1X_SUPPORT */

static int rtmp_parse_wpapsk_buffer_from_file(IN  struct rtmp_adapter *pAd,IN  char *buffer,IN  INT BSSIdx)
{
	char *	tmpbuf = buffer;
	INT			i = BSSIdx;
	/*UCHAR		keyMaterial[40];*/
	ULONG		len = strlen(tmpbuf);
	int         ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) WPAPSK_KEY=%s\n", i, tmpbuf));

	ret = RT_CfgSetWPAPSKKey(pAd, tmpbuf, len, (u8 *)pAd->ApCfg.MBSSID[i].Ssid, pAd->ApCfg.MBSSID[i].SsidLen, pAd->ApCfg.MBSSID[i].PMK);
	if (ret == FALSE)
		return FALSE;

	/* Keep this key string */
	strcpy(pAd->ApCfg.MBSSID[i].WPAKeyString, tmpbuf);

	return ret;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
static void rtmp_read_sta_wmm_parms_from_file(IN  struct rtmp_adapter *pAd, char *tmpbuf, char *buffer)
{
	char *				macptr;
	INT						i=0;
	bool					bWmmEnable = FALSE;

	/*WmmCapable*/
	if(RTMPGetKeyParameter("WmmCapable", tmpbuf, 32, buffer, TRUE))
	{
		if(simple_strtol(tmpbuf, 0, 10) != 0) /*Enable*/
		{
			pAd->CommonCfg.bWmmCapable = TRUE;
			bWmmEnable = TRUE;
		}
		else /*Disable*/
		{
			pAd->CommonCfg.bWmmCapable = FALSE;
		}

		pAd->StaCfg.wdev.bWmmCapable = pAd->CommonCfg.bWmmCapable;

		DBGPRINT(RT_DEBUG_TRACE, ("WmmCapable=%d\n", pAd->CommonCfg.bWmmCapable));
	}


	/*AckPolicy for AC_BK, AC_BE, AC_VI, AC_VO*/
	if(RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			pAd->CommonCfg.AckPolicy[i] = (UCHAR)simple_strtol(macptr, 0, 10);

			DBGPRINT(RT_DEBUG_TRACE, ("AckPolicy[%d]=%d\n", i, pAd->CommonCfg.AckPolicy[i]));
		}
	}

#ifdef UAPSD_SUPPORT
	if (bWmmEnable)
	{
		/*APSDCapable*/
		if(RTMPGetKeyParameter("APSDCapable", tmpbuf, 10, buffer, TRUE))
		{
			if(simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
				pAd->StaCfg.UapsdInfo.bAPSDCapable = TRUE;
			else
				pAd->StaCfg.UapsdInfo.bAPSDCapable = FALSE;

			DBGPRINT(RT_DEBUG_TRACE, ("APSDCapable=%d\n", pAd->StaCfg.UapsdInfo.bAPSDCapable));
		}

		/*MaxSPLength*/
		if(RTMPGetKeyParameter("MaxSPLength", tmpbuf, 10, buffer, TRUE))
		{
			pAd->CommonCfg.MaxSPLength = simple_strtol(tmpbuf, 0, 10);

			DBGPRINT(RT_DEBUG_TRACE, ("MaxSPLength=%d\n", pAd->CommonCfg.MaxSPLength));
		}

		/*APSDAC for AC_BE, AC_BK, AC_VI, AC_VO*/
		if(RTMPGetKeyParameter("APSDAC", tmpbuf, 32, buffer, TRUE))
		{
			bool apsd_ac[4];

			for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			{
				apsd_ac[i] = (bool)simple_strtol(macptr, 0, 10);

				DBGPRINT(RT_DEBUG_TRACE, ("APSDAC%d  %d\n", i,  apsd_ac[i]));
			}

			pAd->CommonCfg.bAPSDAC_BE = apsd_ac[0];
			pAd->CommonCfg.bAPSDAC_BK = apsd_ac[1];
			pAd->CommonCfg.bAPSDAC_VI = apsd_ac[2];
			pAd->CommonCfg.bAPSDAC_VO = apsd_ac[3];

			pAd->CommonCfg.bACMAPSDTr[0] = apsd_ac[0];
			pAd->CommonCfg.bACMAPSDTr[1] = apsd_ac[1];
			pAd->CommonCfg.bACMAPSDTr[2] = apsd_ac[2];
			pAd->CommonCfg.bACMAPSDTr[3] = apsd_ac[3];
		}
	}
#endif /* UAPSD_SUPPORT */
}

#endif /* CONFIG_STA_SUPPORT */


static void VHTParametersHook(
	IN struct rtmp_adapter *pAd,
	IN char *pValueStr,
	IN char *pInput)
{
	long Value;

	/* Channel Width */
	if (RTMPGetKeyParameter("VHT_BW", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == VHT_BW_80)
			pAd->CommonCfg.vht_bw = VHT_BW_80;
		else
			pAd->CommonCfg.vht_bw = VHT_BW_2040;

		DBGPRINT(RT_DEBUG_TRACE, ("VHT: Channel Width = %s\n",
					(pAd->CommonCfg.vht_bw == VHT_BW_80) ? "80 MHz" : "20/40 MHz" ));
	}

	/* VHT GI setting */
	if (RTMPGetKeyParameter("VHT_SGI", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == GI_800)
			pAd->CommonCfg.vht_sgi_80 = GI_800;
		else
			pAd->CommonCfg.vht_sgi_80 = GI_400;

		DBGPRINT(RT_DEBUG_TRACE, ("VHT: Short GI for 80Mhz  = %s\n",
					(pAd->CommonCfg.vht_sgi_80==GI_800) ? "Disabled" : "Enable" ));
	}

	/* VHT STBC */
	if (RTMPGetKeyParameter("VHT_STBC", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		pAd->CommonCfg.vht_stbc = (Value == 1 ? STBC_USE : STBC_NONE);
		DBGPRINT(RT_DEBUG_TRACE, ("VHT: STBC = %d\n",
					pAd->CommonCfg.vht_stbc));
	}

	/* bandwidth signaling */
	if (RTMPGetKeyParameter("VHT_BW_SIGNAL", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value >= 0 && Value <= 2)
			pAd->CommonCfg.vht_bw_signal = Value;
		else
			pAd->CommonCfg.vht_bw_signal = BW_SIGNALING_DISABLE;
		DBGPRINT(RT_DEBUG_TRACE, ("VHT: BW SIGNALING = %d\n", pAd->CommonCfg.vht_bw_signal));
	}

	/* Disallow non-VHT connection */
	if (RTMPGetKeyParameter("VHT_DisallowNonVHT", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == 0)
			pAd->CommonCfg.bNonVhtDisallow = FALSE;
		else
			pAd->CommonCfg.bNonVhtDisallow = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("VHT: VHT_DisallowNonVHT = %d\n", pAd->CommonCfg.bNonVhtDisallow));
	}

	/* VHT LDPC */
	if (RTMPGetKeyParameter("VHT_LDPC", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == 0)
			pAd->CommonCfg.vht_ldpc = FALSE;
		else
			pAd->CommonCfg.vht_ldpc = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("VHT: VHT_LDPC = %d\n", pAd->CommonCfg.vht_ldpc));
	}

#ifdef WFA_VHT_PF
	/* VHT highest Tx Rate with LGI */
	if (RTMPGetKeyParameter("VHT_TX_HRATE", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value >= 0 && Value <= 2)
			pAd->CommonCfg.vht_tx_hrate = Value;
		else
			pAd->CommonCfg.vht_tx_hrate = 0;
		DBGPRINT(RT_DEBUG_TRACE, ("VHT: TX HighestRate = %d\n", pAd->CommonCfg.vht_tx_hrate));
	}

	if (RTMPGetKeyParameter("VHT_RX_HRATE", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value >= 0 && Value <= 2)
			pAd->CommonCfg.vht_rx_hrate = Value;
		else
			pAd->CommonCfg.vht_rx_hrate = 0;
		DBGPRINT(RT_DEBUG_TRACE, ("VHT: RX HighestRate = %d\n", pAd->CommonCfg.vht_rx_hrate));
	}

	if (RTMPGetKeyParameter("VHT_MCS_CAP", pValueStr, 25, pInput, TRUE))
		set_vht_nss_mcs_cap(pAd, pValueStr);
#endif /* WFA_VHT_PF */

}



static void HTParametersHook(
	IN	struct rtmp_adapter *pAd,
	IN	char *	  pValueStr,
	IN	char *	  pInput)
{
	long Value;
#ifdef CONFIG_AP_SUPPORT
	INT			i=0;
	char *	Bufptr;
#endif /* CONFIG_AP_SUPPORT */

    if (RTMPGetKeyParameter("HT_PROTECT", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.bHTProtect = FALSE;
        }
        else
        {
            pAd->CommonCfg.bHTProtect = TRUE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("HT: Protection  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }


    if (RTMPGetKeyParameter("HT_MIMOPSMode", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value > MMPS_DISABLE)
        {
			pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_DISABLE;
        }
        else
        {
            /*TODO: add mimo power saving mechanism*/
            pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_DISABLE;
			/*pAd->CommonCfg.BACapability.field.MMPSmode = Value;*/
        }
        DBGPRINT(RT_DEBUG_TRACE, ("HT: MIMOPS Mode  = %d\n", (INT) Value));
    }

    if (RTMPGetKeyParameter("HT_BADecline", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.bBADecline = FALSE;
        }
        else
        {
            pAd->CommonCfg.bBADecline = TRUE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("HT: BA Decline  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }


    if (RTMPGetKeyParameter("HT_AutoBA", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
			pAd->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
        }
        else
        {
            pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
			pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
        }
        pAd->CommonCfg.REGBACapability.field.AutoBA = pAd->CommonCfg.BACapability.field.AutoBA;
		pAd->CommonCfg.REGBACapability.field.Policy = pAd->CommonCfg.BACapability.field.Policy;
        DBGPRINT(RT_DEBUG_TRACE, ("HT: Auto BA  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }

	/* Tx_+HTC frame*/
    if (RTMPGetKeyParameter("HT_HTC", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->HTCEnable = FALSE;
		}
		else
		{
            pAd->HTCEnable = TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Tx +HTC frame = %s\n", (Value==0) ? "Disable" : "Enable"));
	}


	/* Reverse Direction Mechanism*/
    if (RTMPGetKeyParameter("HT_RDG", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->CommonCfg.bRdg = FALSE;
		}
		else
		{
			pAd->HTCEnable = TRUE;
            pAd->CommonCfg.bRdg = TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: RDG = %s\n", (Value==0) ? "Disable" : "Enable(+HTC)"));
	}




	/* Tx A-MSUD ?*/
    if (RTMPGetKeyParameter("HT_AMSDU", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		pAd->CommonCfg.BACapability.field.AmsduEnable = (Value == 0) ? FALSE : TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Tx A-MSDU = %s\n", (Value==0) ? "Disable" : "Enable"));
	}

#ifdef WFA_VHT_PF
	if (RTMPGetKeyParameter("FORCE_AMSDU", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		pAd->force_amsdu = (Value == 0) ? FALSE : TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("HT: FORCE A-MSDU = %s\n", (Value==0) ? "Disable" : "Enable"));
	}
#endif /* WFA_VHT_PF */

	/* MPDU Density*/
    if (RTMPGetKeyParameter("HT_MpduDensity", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value <=7 && Value >= 0)
		{
			pAd->CommonCfg.BACapability.field.MpduDensity = Value;
			DBGPRINT(RT_DEBUG_TRACE, ("HT: MPDU Density = %d\n", (INT) Value));
		}
		else
		{
			pAd->CommonCfg.BACapability.field.MpduDensity = 4;
			DBGPRINT(RT_DEBUG_TRACE, ("HT: MPDU Density = %d (Default)\n", 4));
		}
	}

	/* Max Rx BA Window Size*/
    if (RTMPGetKeyParameter("HT_BAWinSize", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
		/* Intel IOT*/
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		Value = 64;
#endif /* CONFIG_AP_SUPPORT */
		if (Value >=1 && Value <= 64)
		{
			pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = Value;
			pAd->CommonCfg.BACapability.field.RxBAWinLimit = Value;
			DBGPRINT(RT_DEBUG_TRACE, ("HT: BA Windw Size = %d\n", (INT) Value));
		}
		else
		{
            pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = 64;
			pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64;
			DBGPRINT(RT_DEBUG_TRACE, ("HT: BA Windw Size = 64 (Defualt)\n"));
		}

	}

	/* Guard Interval*/
	if (RTMPGetKeyParameter("HT_GI", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == GI_400)
		{
			pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_400;
		}
		else
		{
			pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_800;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Guard Interval = %s\n", (Value==GI_400) ? "400" : "800" ));
	}

	/* HT LDPC */
	if (RTMPGetKeyParameter("HT_LDPC", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == 0)
			pAd->CommonCfg.ht_ldpc = FALSE;
		else
			pAd->CommonCfg.ht_ldpc = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("HT: HT_LDPC = %d\n", pAd->CommonCfg.ht_ldpc));
	}

	/* HT Operation Mode : Mixed Mode , Green Field*/
	if (RTMPGetKeyParameter("HT_OpMode", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == HTMODE_GF)
		{

			pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_GF;
		}
		else
		{
			pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_MM;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Operate Mode = %s\n", (Value==HTMODE_GF) ? "Green Field" : "Mixed Mode" ));
	}

	/* Fixed Tx mode : CCK, OFDM*/
	if (RTMPGetKeyParameter("FixedTxMode", pValueStr, 25, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				pAd->ApCfg.MBSSID[i].wdev.DesiredTransmitSetting.field.FixedTxMode =
														RT_CfgSetFixedTxPhyMode(Bufptr);
				DBGPRINT(RT_DEBUG_TRACE, ("(IF-ra%d) Fixed Tx Mode = %d\n", i,
											pAd->ApCfg.MBSSID[i].wdev.DesiredTransmitSetting.field.FixedTxMode));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			pAd->StaCfg.wdev.DesiredTransmitSetting.field.FixedTxMode =
										RT_CfgSetFixedTxPhyMode(pValueStr);
			DBGPRINT(RT_DEBUG_TRACE, ("Fixed Tx Mode = %d\n",
											pAd->StaCfg.wdev.DesiredTransmitSetting.field.FixedTxMode));
		}
#endif /* CONFIG_STA_SUPPORT */
	}


	/* Channel Width */
	if (RTMPGetKeyParameter("HT_BW", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == BW_40)
			pAd->CommonCfg.RegTransmitSetting.field.BW = BW_40;
		else
			pAd->CommonCfg.RegTransmitSetting.field.BW = BW_20;

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Channel Width = %s\n", (Value==BW_40) ? "40 MHz" : "20 MHz" ));
	}

	if (RTMPGetKeyParameter("HT_EXTCHA", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == 0)
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA  = EXTCHA_BELOW;
		else
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_ABOVE;

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Ext Channel = %s\n", (Value==0) ? "BELOW" : "ABOVE" ));
	}

	/* MSC*/
	if (RTMPGetKeyParameter("HT_MCS", pValueStr, 50, pInput, TRUE))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			for (i = 0, Bufptr = rstrtok(pValueStr,";"); (Bufptr && i < MAX_MBSSID_NUM(pAd)); Bufptr = rstrtok(NULL,";"), i++)
			{
				struct rtmp_wifi_dev *wdev = &pAd->ApCfg.MBSSID[i].wdev;
				Value = simple_strtol(Bufptr, 0, 10);
				if ((Value >= 0 && Value <= 23) || (Value == 32))
					wdev->DesiredTransmitSetting.field.MCS = Value;
				else
					wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;

				DBGPRINT(RT_DEBUG_TRACE, ("(IF-ra%d) HT: MCS = %s(%d)\n", i,
						(wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : "Fixed"),
						wdev->DesiredTransmitSetting.field.MCS));
			}
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			struct rtmp_wifi_dev *wdev = &pAd->StaCfg.wdev;
			Value = simple_strtol(pValueStr, 0, 10);
			if ((Value >= 0 && Value <= 23) || (Value == 32))
			{
				wdev->DesiredTransmitSetting.field.MCS  = Value;
				wdev->bAutoTxRateSwitch = FALSE;
				DBGPRINT(RT_DEBUG_TRACE, ("HT: MCS = %d\n", wdev->DesiredTransmitSetting.field.MCS));
			}
			else
			{
				wdev->DesiredTransmitSetting.field.MCS  = MCS_AUTO;
				wdev->bAutoTxRateSwitch = TRUE;
				DBGPRINT(RT_DEBUG_TRACE, ("HT: MCS = AUTO\n"));
			}
	}
#endif /* CONFIG_STA_SUPPORT */
	}

	/* STBC */
    if (RTMPGetKeyParameter("HT_STBC", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == STBC_USE)
		{
			pAd->CommonCfg.RegTransmitSetting.field.STBC = STBC_USE;
		}
		else
		{
			pAd->CommonCfg.RegTransmitSetting.field.STBC = STBC_NONE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: STBC = %d\n", pAd->CommonCfg.RegTransmitSetting.field.STBC));
	}

	/* 40_Mhz_Intolerant*/
	if (RTMPGetKeyParameter("HT_40MHZ_INTOLERANT", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->CommonCfg.bForty_Mhz_Intolerant = FALSE;
		}
		else
		{
			pAd->CommonCfg.bForty_Mhz_Intolerant = TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: 40MHZ INTOLERANT = %d\n", pAd->CommonCfg.bForty_Mhz_Intolerant));
	}
	/*HT_TxStream*/
	if(RTMPGetKeyParameter("HT_TxStream", pValueStr, 10, pInput, TRUE))
	{
		switch (simple_strtol(pValueStr, 0, 10))
		{
			case 1:
				pAd->CommonCfg.TxStream = 1;
				break;
			case 2:
				pAd->CommonCfg.TxStream = 2;
				break;
			case 3: /* 3*3*/
			default:
				pAd->CommonCfg.TxStream = 3;

				if (pAd->MACVersion < RALINK_2883_VERSION)
					pAd->CommonCfg.TxStream = 2; /* only 2 tx streams for RT2860 series*/
				break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Tx Stream = %d\n", pAd->CommonCfg.TxStream));
	}
	/*HT_RxStream*/
	if(RTMPGetKeyParameter("HT_RxStream", pValueStr, 10, pInput, TRUE))
	{
		switch (simple_strtol(pValueStr, 0, 10))
		{
			case 1:
				pAd->CommonCfg.RxStream = 1;
				break;
			case 2:
				pAd->CommonCfg.RxStream = 2;
				break;
			case 3:
			default:
				pAd->CommonCfg.RxStream = 3;

				if (pAd->MACVersion < RALINK_2883_VERSION)
					pAd->CommonCfg.RxStream = 2; /* only 2 rx streams for RT2860 series*/
				break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Rx Stream = %d\n", pAd->CommonCfg.RxStream));
	}
	/* HT_DisallowTKIP*/
	if (RTMPGetKeyParameter("HT_DisallowTKIP", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == 1)
		{
			pAd->CommonCfg.HT_DisallowTKIP = TRUE;
		}
		else
		{
			pAd->CommonCfg.HT_DisallowTKIP = FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Disallow TKIP mode = %s\n", (pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "ON" : "OFF" ));
	}

			if (RTMPGetKeyParameter("OBSSScanParam", pValueStr, 32, pInput, TRUE))
			{
				int ObssScanValue, idx;
				char *macptr;
				for (idx = 0, macptr = rstrtok(pValueStr,";"); macptr; macptr = rstrtok(NULL,";"), idx++)
				{
					ObssScanValue = simple_strtol(macptr, 0, 10);
					switch (idx)
					{
						case 0:
							if (ObssScanValue < 5 || ObssScanValue > 1000)
							{
								DBGPRINT(RT_DEBUG_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanPassiveDwell(%d), should in range 5~1000\n", ObssScanValue));
							}
							else
							{
								pAd->CommonCfg.Dot11OBssScanPassiveDwell = ObssScanValue;	/* Unit : TU. 5~1000*/
								DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveDwell=%d\n", ObssScanValue));
							}
							break;
						case 1:
							if (ObssScanValue < 10 || ObssScanValue > 1000)
							{
								DBGPRINT(RT_DEBUG_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanActiveDwell(%d), should in range 10~1000\n", ObssScanValue));
							}
							else
							{
								pAd->CommonCfg.Dot11OBssScanActiveDwell = ObssScanValue;	/* Unit : TU. 10~1000*/
								DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11OBssScanActiveDwell=%d\n", ObssScanValue));
							}
							break;
						case 2:
							pAd->CommonCfg.Dot11BssWidthTriggerScanInt = ObssScanValue;	/* Unit : Second*/
							DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11BssWidthTriggerScanInt=%d\n", ObssScanValue));
							break;
						case 3:
							if (ObssScanValue < 200 || ObssScanValue > 10000)
							{
								DBGPRINT(RT_DEBUG_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel(%d), should in range 200~10000\n", ObssScanValue));
							}
							else
							{
								pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 200~10000*/
								DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel=%d\n", ObssScanValue));
							}
							break;
						case 4:
							if (ObssScanValue < 20 || ObssScanValue > 10000)
							{
								DBGPRINT(RT_DEBUG_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanActiveTotalPerChannel(%d), should in range 20~10000\n", ObssScanValue));
							}
							else
							{
								pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 20~10000*/
								DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11OBssScanActiveTotalPerChannel=%d\n", ObssScanValue));
							}
							break;
						case 5:
							pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = ObssScanValue;
							DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n", ObssScanValue));
							break;
						case 6:
							pAd->CommonCfg.Dot11OBssScanActivityThre = ObssScanValue;	/* Unit : percentage*/
							DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n", ObssScanValue));
							break;
					}
				}

				if (idx != 7)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Wrong OBSSScanParamtetrs format in dat file!!!!! Use default value.\n"));

					pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
					pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
					pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
					pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
					pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
					pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
					pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
				}
				pAd->CommonCfg.Dot11BssWidthChanTranDelay = (pAd->CommonCfg.Dot11BssWidthTriggerScanInt * pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
							DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelay=%ld\n", pAd->CommonCfg.Dot11BssWidthChanTranDelay));
			}

			if (RTMPGetKeyParameter("HT_BSSCoexistence", pValueStr, 25, pInput, TRUE))
			{
				Value = simple_strtol(pValueStr, 0, 10);
				pAd->CommonCfg.bBssCoexEnable = ((Value == 1) ? TRUE : FALSE);

				DBGPRINT(RT_DEBUG_TRACE, ("HT: 20/40 BssCoexSupport = %s\n", (pAd->CommonCfg.bBssCoexEnable == TRUE) ? "ON" : "OFF" ));
			}


			if (RTMPGetKeyParameter("HT_BSSCoexApCntThr", pValueStr, 25, pInput, TRUE))
			{
				pAd->CommonCfg.BssCoexApCntThr = simple_strtol(pValueStr, 0, 10);;

				DBGPRINT(RT_DEBUG_TRACE, ("HT: 20/40 BssCoexApCntThr = %d\n", pAd->CommonCfg.BssCoexApCntThr));
			}


	if (RTMPGetKeyParameter("BurstMode", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		pAd->CommonCfg.bRalinkBurstMode = ((Value == 1) ? 1 : 0);
		DBGPRINT(RT_DEBUG_TRACE, ("HT: RaBurstMode= %d\n", pAd->CommonCfg.bRalinkBurstMode));
	}

}


#ifdef CONFIG_STA_SUPPORT
void RTMPSetSTASSID(struct rtmp_adapter *pAd, char *SSID)
{
	pAd->CommonCfg.SsidLen = (UCHAR) strlen(SSID);
	memset(pAd->CommonCfg.Ssid, 0, NDIS_802_11_LENGTH_SSID);
	memmove(pAd->CommonCfg.Ssid, SSID, pAd->CommonCfg.SsidLen);
	pAd->CommonCfg.LastSsidLen= pAd->CommonCfg.SsidLen;
	memset(pAd->CommonCfg.LastSsid, 0, NDIS_802_11_LENGTH_SSID);
	memmove(pAd->CommonCfg.LastSsid, SSID, pAd->CommonCfg.LastSsidLen);
	pAd->MlmeAux.AutoReconnectSsidLen = pAd->CommonCfg.SsidLen;
	memset(pAd->MlmeAux.AutoReconnectSsid, 0, NDIS_802_11_LENGTH_SSID);
	memmove(pAd->MlmeAux.AutoReconnectSsid, SSID, pAd->MlmeAux.AutoReconnectSsidLen);
	pAd->MlmeAux.SsidLen = pAd->CommonCfg.SsidLen;
	memset(pAd->MlmeAux.Ssid, 0, NDIS_802_11_LENGTH_SSID);
	memmove(pAd->MlmeAux.Ssid, SSID, pAd->MlmeAux.SsidLen);
}


void RTMPSetSTAPassPhrase(struct rtmp_adapter *pAd, char *PassPh)
{
	struct rtmp_wifi_dev *wdev = &pAd->StaCfg.wdev;
	int ret = TRUE;

	PassPh[strlen(PassPh)] = '\0'; /* make STA can process .$^& for WPAPSK input */

	if ((wdev->AuthMode != Ndis802_11AuthModeWPAPSK) &&
		(wdev->AuthMode != Ndis802_11AuthModeWPA2PSK) &&
		(wdev->AuthMode != Ndis802_11AuthModeWPANone)
		)
	{
		ret = FALSE;
	}
	else
	{
		ret = RT_CfgSetWPAPSKKey(pAd, PassPh, strlen(PassPh), (u8 *)pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen, pAd->StaCfg.PMK);
	}

	if (ret == TRUE)
	{
		RTMPZeroMemory(pAd->StaCfg.WpaPassPhrase, 64);
		RTMPMoveMemory(pAd->StaCfg.WpaPassPhrase, PassPh, strlen(PassPh));
		pAd->StaCfg.WpaPassPhraseLen= strlen(PassPh);

	    if ((wdev->AuthMode == Ndis802_11AuthModeWPAPSK) ||
			(wdev->AuthMode == Ndis802_11AuthModeWPA2PSK))
		{
			/* Start STA supplicant state machine*/
			pAd->StaCfg.WpaState = SS_START;
		}
		else if (wdev->AuthMode == Ndis802_11AuthModeWPANone)
		{
			pAd->StaCfg.WpaState = SS_NOTUSE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("%s::(WPAPSK=%s)\n", __FUNCTION__, PassPh));
	}
}


inline void RTMPSetSTACipherSuites(struct rtmp_adapter *pAd, NDIS_802_11_ENCRYPTION_STATUS WepStatus)
{
	/* Update all wepstatus related*/
	pAd->StaCfg.PairCipher		= WepStatus;
	pAd->StaCfg.GroupCipher 	= WepStatus;
	pAd->StaCfg.bMixCipher 		= FALSE;
}

#ifdef  CREDENTIAL_STORE

/*RECOVER THE OLD CONNECT INFO */
int RecoverConnectInfo(
	IN  struct rtmp_adapter *pAd)
{
	INT idx;
	char ssidStr[NDIS_802_11_LENGTH_SSID + 1];

	memset(&ssidStr[0], sizeof(ssidStr));

	RTMP_SEM_LOCK(&pAd->StaCtIf.Lock);
	if ((pAd->StaCtIf.Changeable== FALSE) || (pAd->StaCtIf.SsidLen > NDIS_802_11_LENGTH_SSID))
	{
		DBGPRINT(RT_DEBUG_TRACE, (" DRIVER INIT  not need to RecoverConnectInfo() \n"));
		RTMP_SEM_UNLOCK(&pAd->StaCtIf.Lock);
		return 0;
	}
	DBGPRINT(RT_DEBUG_TRACE, ("-->RecoverConnectInfo()\n"));

	memmove(ssidStr, pAd->StaCtIf.Ssid, pAd->StaCtIf.SsidLen);
	RTMPSetSTASSID(pAd, &ssidStr[0]);

	pAd->StaCfg.AuthMode = pAd->StaCtIf.AuthMode;
	pAd->StaCfg.WepStatus = pAd->StaCtIf.WepStatus;
#ifdef WPA_SUPPLICANT_SUPPORT
	pAd->StaCfg.wdev.IEEE8021X = pAd->StaCtIf.IEEE8021X;
	pAd->StaCfg.wpa_supplicant_info.DesireSharedKeyId = pAd->StaCtIf.DefaultKeyId;
#endif // WPA_SUPPLICANT_SUPPORT //
	pAd->StaCfg.DefaultKeyId = pAd->StaCtIf.DefaultKeyId;
	memmove( pAd->StaCfg.PMK, pAd->StaCtIf.PMK, 32);
	RTMPMoveMemory(pAd->StaCfg.WpaPassPhrase, pAd->StaCtIf.WpaPassPhrase, pAd->StaCfg.WpaPassPhraseLen);
	pAd->StaCfg.WpaPassPhraseLen = pAd->StaCtIf.WpaPassPhraseLen;
	for (idx = 0; idx < 4; idx++)
	{
		memmove(&pAd->SharedKey[BSS0][idx], &pAd->StaCtIf.SharedKey[BSS0][idx], sizeof(CIPHER_KEY));
#ifdef WPA_SUPPLICANT_SUPPORT
		memmove(&pAd->StaCfg.wpa_supplicant_info.DesireSharedKey[idx], &pAd->StaCtIf.SharedKey[BSS0][idx], sizeof(CIPHER_KEY));
#endif // WPA_SUPPLICANT_SUPPORT //

	}

	 if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
			(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
			// Start STA supplicant state machine
			pAd->StaCfg.WpaState = SS_START;
	}
	else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
	{
			pAd->StaCfg.WpaState = SS_NOTUSE;
	}
	RTMP_SEM_UNLOCK(&pAd->StaCtIf.Lock);

	DBGPRINT(RT_DEBUG_TRACE, ("<--RecoverConnectInfo()\n"));

	return 0;
}


/*STORE THE CONNECT INFO*/
int StoreConnectInfo(
	IN  struct rtmp_adapter *pAd)
{
	INT idx;
	DBGPRINT(RT_DEBUG_TRACE, ("-->StoreConnectInfo()\n"));

	RTMP_SEM_LOCK(&pAd->StaCtIf.Lock);
	pAd->StaCtIf.Changeable = TRUE;
 	pAd->StaCtIf.SsidLen = pAd->CommonCfg.SsidLen;
	memmove(pAd->StaCtIf.Ssid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
	pAd->StaCtIf.AuthMode = pAd->StaCfg.AuthMode;
	pAd->StaCtIf.WepStatus = pAd->StaCfg.WepStatus;

	pAd->StaCtIf.DefaultKeyId = pAd->StaCfg.DefaultKeyId;
#ifdef WPA_SUPPLICANT_SUPPORT
	pAd->StaCtIf.wpa_supplicant_info.DefaultKeyId = pAd->StaCfg.DesireSharedKeyId;
	pAd->StaCtIf.IEEE8021X = pAd->StaCfg.wdev.IEEE8021X;
#endif // WPA_SUPPLICANT_SUPPORT //
	memmove(pAd->StaCtIf.PMK, pAd->StaCfg.PMK, 32);
	RTMPMoveMemory(pAd->StaCtIf.WpaPassPhrase, pAd->StaCfg.WpaPassPhrase, pAd->StaCfg.WpaPassPhraseLen);
	pAd->StaCtIf.WpaPassPhraseLen = pAd->StaCfg.WpaPassPhraseLen;

	for (idx = 0; idx < 4; idx++)
	{
		memmove(&pAd->StaCtIf.SharedKey[BSS0][idx], &pAd->SharedKey[BSS0][idx], sizeof(CIPHER_KEY));
	}

	RTMP_SEM_UNLOCK(&pAd->StaCtIf.Lock);

	DBGPRINT(RT_DEBUG_TRACE, ("<--StoreConnectInfo()\n"));

	return 0;
}

#endif /* CREDENTIAL_STORE */

#endif /* CONFIG_STA_SUPPORT */


void RTMPSetCountryCode(struct rtmp_adapter *pAd, char *CountryCode)
{
	memmove(pAd->CommonCfg.CountryCode, CountryCode , 2);
	pAd->CommonCfg.CountryCode[2] = ' ';
	if (strlen(pAd->CommonCfg.CountryCode) != 0)
		pAd->CommonCfg.bCountryFlag = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("CountryCode=%s\n", pAd->CommonCfg.CountryCode));
}


int RTMPSetProfileParameters(
	IN struct rtmp_adapter *pAd,
	IN char *pBuffer)
{
	char *				tmpbuf;
	ULONG					RtsThresh;
	ULONG					FragThresh;
	char *				macptr;
	INT						i = 0, retval;

	tmpbuf = kmalloc(MAX_PARAM_BUFFER_SIZE, GFP_ATOMIC);
	if(tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;

	do
	{
		/*CountryRegion*/
		if(RTMPGetKeyParameter("CountryRegion", tmpbuf, 25, pBuffer, TRUE))
		{
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_24G);
			DBGPRINT(RT_DEBUG_TRACE, ("CountryRegion=%d\n", pAd->CommonCfg.CountryRegion));
		}
		/*CountryRegionABand*/
		if(RTMPGetKeyParameter("CountryRegionABand", tmpbuf, 25, pBuffer, TRUE))
		{
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_5G);
			DBGPRINT(RT_DEBUG_TRACE, ("CountryRegionABand=%d\n", pAd->CommonCfg.CountryRegionForABand));
		}

		/*CountryCode*/
		if (pAd->CommonCfg.bCountryFlag == 0)
		{
		if(RTMPGetKeyParameter("CountryCode", tmpbuf, 25, pBuffer, TRUE))
			RTMPSetCountryCode(pAd, tmpbuf);
		}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef MBSS_SUPPORT
			/*BSSIDNum; This must read first of other multiSSID field, so list this field first in configuration file*/
			if(RTMPGetKeyParameter("BssidNum", tmpbuf, 25, pBuffer, TRUE))
			{
				pAd->ApCfg.BssidNum = (UCHAR) simple_strtol(tmpbuf, 0, 10);
				if(pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
				{
					pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
					DBGPRINT(RT_DEBUG_TRACE, ("BssidNum=%d(MAX_MBSSID_NUM is %d)\n", pAd->ApCfg.BssidNum,MAX_MBSSID_NUM(pAd)));
				}
				else
				DBGPRINT(RT_DEBUG_TRACE, ("BssidNum=%d\n", pAd->ApCfg.BssidNum));
			}

			if (HW_BEACON_OFFSET > (HW_BEACON_MAX_SIZE(pAd) / pAd->ApCfg.BssidNum))
			{
				DBGPRINT(RT_DEBUG_OFF, ("mbss> fatal error! beacon offset is error in driver! "
						"Please re-assign HW_BEACON_OFFSET!\n"));
			}
#else
			pAd->ApCfg.BssidNum = 1;
#endif /* MBSS_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			/*NetworkType*/
			if (RTMPGetKeyParameter("NetworkType", tmpbuf, 25, pBuffer, TRUE))
			{
				pAd->bConfigChanged = TRUE;
				if (strcmp(tmpbuf, "Adhoc") == 0)
					pAd->StaCfg.BssType = BSS_ADHOC;
				else /*Default Infrastructure mode*/
					pAd->StaCfg.BssType = BSS_INFRA;
				/* Reset Ralink supplicant to not use, it will be set to start when UI set PMK key*/
				pAd->StaCfg.WpaState = SS_NOTUSE;
				DBGPRINT(RT_DEBUG_TRACE, ("%s::(NetworkType=%d)\n", __FUNCTION__, pAd->StaCfg.BssType));
			}
		}
#endif /* CONFIG_STA_SUPPORT */
		/*Channel*/
		if(RTMPGetKeyParameter("Channel", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->CommonCfg.Channel = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			DBGPRINT(RT_DEBUG_TRACE, ("Channel=%d\n", pAd->CommonCfg.Channel));
			if (pAd->CommonCfg.Channel > 14)
				pAd->Dot11_H.org_ch = pAd->CommonCfg.Channel;
		}

		/*WirelessMode*/
		/*Note: BssidNum must be put before WirelessMode in dat file*/
		if(RTMPGetKeyParameter("WirelessMode", tmpbuf, 32, pBuffer, TRUE))
		{
			UCHAR cfg_mode;
			for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			{
				cfg_mode = simple_strtol(macptr, 0, 10);
#ifdef CONFIG_AP_SUPPORT
				if (i >= pAd->ApCfg.BssidNum)
					break;

				pAd->ApCfg.MBSSID[i].wdev.PhyMode = cfgmode_2_wmode(cfg_mode);
				DBGPRINT(RT_DEBUG_TRACE, ("BSS%d PhyMode=%d\n", i, pAd->ApCfg.MBSSID[i].wdev.PhyMode));
#endif /* CONFIG_AP_SUPPORT */

				if (i == 0)
				{
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
					/* for first time, update all phy mode is same as ra0 */
					{
						uint32_t IdBss;
						for(IdBss=1; IdBss<pAd->ApCfg.BssidNum; IdBss++)
							pAd->ApCfg.MBSSID[IdBss].wdev.PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
					}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
					/* set mode for 1st time */
					RT_CfgSetWirelessMode(pAd, macptr);
				}
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
				else
					RT_CfgSetMbssWirelessMode(pAd, macptr);
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
			}

			DBGPRINT(RT_DEBUG_TRACE, ("PhyMode=%d\n", pAd->CommonCfg.PhyMode));
		}

	    /*BasicRate*/
		if(RTMPGetKeyParameter("BasicRate", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->CommonCfg.BasicRateBitmap = (ULONG) simple_strtol(tmpbuf, 0, 10);
			pAd->CommonCfg.BasicRateBitmapOld = (ULONG) simple_strtol(tmpbuf, 0, 10);
			DBGPRINT(RT_DEBUG_TRACE, ("BasicRate=%ld\n", pAd->CommonCfg.BasicRateBitmap));
		}
		/*BeaconPeriod*/
		if(RTMPGetKeyParameter("BeaconPeriod", tmpbuf, 10, pBuffer, TRUE))
		{
			USHORT bcn_val = (USHORT) simple_strtol(tmpbuf, 0, 10);

			/* The acceptable is 20~1000 ms. Refer to WiFi test plan. */
			if (bcn_val >= 20 && bcn_val <= 1000)
				pAd->CommonCfg.BeaconPeriod = bcn_val;
			else
				pAd->CommonCfg.BeaconPeriod = 100;	/* Default value*/

#ifdef APCLI_CONNECTION_TRIAL
			pAd->CommonCfg.BeaconPeriod = 200;
#endif /* APCLI_CONNECTION_TRIAL */

			DBGPRINT(RT_DEBUG_TRACE, ("BeaconPeriod=%d\n", pAd->CommonCfg.BeaconPeriod));
		}



#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/*DtimPeriod*/
			if(RTMPGetKeyParameter("DtimPeriod", tmpbuf, 10, pBuffer, TRUE))
			{
				pAd->ApCfg.DtimPeriod = (UCHAR) simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_TRACE, ("DtimPeriod=%d\n", pAd->ApCfg.DtimPeriod));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	    /*TxPower*/
		if(RTMPGetKeyParameter("TxPower", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->CommonCfg.TxPowerPercentage = (ULONG) simple_strtol(tmpbuf, 0, 10);
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				pAd->CommonCfg.TxPowerDefault = pAd->CommonCfg.TxPowerPercentage;
#endif /* CONFIG_STA_SUPPORT */
			DBGPRINT(RT_DEBUG_TRACE, ("TxPower=%ld\n", pAd->CommonCfg.TxPowerPercentage));
		}
		/*BGProtection*/
		if(RTMPGetKeyParameter("BGProtection", tmpbuf, 10, pBuffer, TRUE))
		{
	/*#if 0	#ifndef WIFI_TEST*/
	/*		pAd->CommonCfg.UseBGProtection = 2; disable b/g protection for throughput test*/
	/*#else*/
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case 1: /*Always On*/
					pAd->CommonCfg.UseBGProtection = 1;
					break;
				case 2: /*Always OFF*/
					pAd->CommonCfg.UseBGProtection = 2;
					break;
				case 0: /*AUTO*/
				default:
					pAd->CommonCfg.UseBGProtection = 0;
					break;
			}
	/*#endif*/
			DBGPRINT(RT_DEBUG_TRACE, ("BGProtection=%ld\n", pAd->CommonCfg.UseBGProtection));
		}

#ifdef CONFIG_AP_SUPPORT
		/*OLBCDetection*/
		if(RTMPGetKeyParameter("DisableOLBC", tmpbuf, 10, pBuffer, TRUE))
		{
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case 1: /*disable OLBC Detection*/
					pAd->CommonCfg.DisableOLBCDetect = 1;
					break;
				case 0: /*enable OLBC Detection*/
					pAd->CommonCfg.DisableOLBCDetect = 0;
					break;
				default:
					pAd->CommonCfg.DisableOLBCDetect= 0;
					break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("OLBCDetection=%ld\n", pAd->CommonCfg.DisableOLBCDetect));
		}
#endif /* CONFIG_AP_SUPPORT */
		/*TxPreamble*/
		if(RTMPGetKeyParameter("TxPreamble", tmpbuf, 10, pBuffer, TRUE))
		{
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case Rt802_11PreambleShort:
					pAd->CommonCfg.TxPreamble = Rt802_11PreambleShort;
					break;
				case Rt802_11PreambleLong:
				default:
					pAd->CommonCfg.TxPreamble = Rt802_11PreambleLong;
					break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("TxPreamble=%ld\n", pAd->CommonCfg.TxPreamble));
		}
		/*RTSThreshold*/
		if(RTMPGetKeyParameter("RTSThreshold", tmpbuf, 10, pBuffer, TRUE))
		{
			RtsThresh = simple_strtol(tmpbuf, 0, 10);
			if( (RtsThresh >= 1) && (RtsThresh <= MAX_RTS_THRESHOLD) )
				pAd->CommonCfg.RtsThreshold  = (USHORT)RtsThresh;
			else
				pAd->CommonCfg.RtsThreshold = MAX_RTS_THRESHOLD;

			DBGPRINT(RT_DEBUG_TRACE, ("RTSThreshold=%d\n", pAd->CommonCfg.RtsThreshold));
		}
		/*FragThreshold*/
		if(RTMPGetKeyParameter("FragThreshold", tmpbuf, 10, pBuffer, TRUE))
		{
			FragThresh = simple_strtol(tmpbuf, 0, 10);
			pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;

			if (FragThresh > MAX_FRAG_THRESHOLD || FragThresh < MIN_FRAG_THRESHOLD)
			{ /*illegal FragThresh so we set it to default*/
				pAd->CommonCfg.FragmentThreshold = MAX_FRAG_THRESHOLD;
				pAd->CommonCfg.bUseZeroToDisableFragment = TRUE;
			}
			else if (FragThresh % 2 == 1)
			{
				/* The length of each fragment shall always be an even number of octets, except for the last fragment*/
				/* of an MSDU or MMPDU, which may be either an even or an odd number of octets.*/
				pAd->CommonCfg.FragmentThreshold = (USHORT)(FragThresh - 1);
			}
			else
			{
				pAd->CommonCfg.FragmentThreshold = (USHORT)FragThresh;
			}
			/*pAd->CommonCfg.AllowFragSize = (pAd->CommonCfg.FragmentThreshold) - LENGTH_802_11 - LENGTH_CRC;*/
			DBGPRINT(RT_DEBUG_TRACE, ("FragThreshold=%d\n", pAd->CommonCfg.FragmentThreshold));
		}
		/*TxBurst*/
		if(RTMPGetKeyParameter("TxBurst", tmpbuf, 10, pBuffer, TRUE))
		{
	/*#ifdef WIFI_TEST*/
	/*						pAd->CommonCfg.bEnableTxBurst = FALSE;*/
	/*#else*/
			if(simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
				pAd->CommonCfg.bEnableTxBurst = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bEnableTxBurst = FALSE;
	/*#endif*/
			DBGPRINT(RT_DEBUG_TRACE, ("TxBurst=%d\n", pAd->CommonCfg.bEnableTxBurst));
		}

#ifdef AGGREGATION_SUPPORT
		/*PktAggregate*/
		if(RTMPGetKeyParameter("PktAggregate", tmpbuf, 10, pBuffer, TRUE))
		{
			if(simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
				pAd->CommonCfg.bAggregationCapable = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bAggregationCapable = FALSE;
#ifdef PIGGYBACK_SUPPORT
			pAd->CommonCfg.bPiggyBackCapable = pAd->CommonCfg.bAggregationCapable;
#endif /* PIGGYBACK_SUPPORT */
			DBGPRINT(RT_DEBUG_TRACE, ("PktAggregate=%d\n", pAd->CommonCfg.bAggregationCapable));
		}
#else
		pAd->CommonCfg.bAggregationCapable = FALSE;
		pAd->CommonCfg.bPiggyBackCapable = FALSE;
#endif /* AGGREGATION_SUPPORT */

		/* WmmCapable*/
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			rtmp_read_ap_wmm_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			rtmp_read_sta_wmm_parms_from_file(pAd, tmpbuf, pBuffer);
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* MaxStaNum*/
			if (RTMPGetKeyParameter("MaxStaNum", tmpbuf, 32, pBuffer, TRUE))
			{
			    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			    {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					ApCfg_Set_MaxStaNum_Proc(pAd, i, macptr);
			    }
			}

			/* IdleTimeout*/
			if(RTMPGetKeyParameter("IdleTimeout", tmpbuf, 10, pBuffer, TRUE))
			{
				ApCfg_Set_IdleTimeout_Proc(pAd, tmpbuf);
			}

			/*NoForwarding*/
			if(RTMPGetKeyParameter("NoForwarding", tmpbuf, 32, pBuffer, TRUE))
			{
			    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			    {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
						pAd->ApCfg.MBSSID[i].IsolateInterStaTraffic = TRUE;
					else /*Disable*/
						pAd->ApCfg.MBSSID[i].IsolateInterStaTraffic = FALSE;

					DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) NoForwarding=%ld\n", i, pAd->ApCfg.MBSSID[i].IsolateInterStaTraffic));
			    }
			}

			//NoForwardingMBCast
			if (RTMPGetKeyParameter("NoForwardingMBCast", tmpbuf, 32, pBuffer, TRUE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
				{
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (simple_strtol(macptr, 0, 10) != 0)	//Enable
						pAd->ApCfg.MBSSID[i].IsolateInterStaMBCast = TRUE;
					else //Disable
						pAd->ApCfg.MBSSID[i].IsolateInterStaMBCast = FALSE;

					DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) NoForwardingMBCast=%d\n", i, pAd->ApCfg.MBSSID[i].IsolateInterStaMBCast));
				}
			}

			/*NoForwardingBTNBSSID*/
			if(RTMPGetKeyParameter("NoForwardingBTNBSSID", tmpbuf, 10, pBuffer, TRUE))
			{
				if(simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = TRUE;
				else /*Disable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;

				DBGPRINT(RT_DEBUG_TRACE, ("NoForwardingBTNBSSID=%ld\n", pAd->ApCfg.IsolateInterStaTrafficBTNBSSID));
			}
			/*HideSSID*/
			if(RTMPGetKeyParameter("HideSSID", tmpbuf, 32, pBuffer, TRUE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			    {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
					{
						pAd->ApCfg.MBSSID[apidx].bHideSsid = TRUE;
					}
					else /*Disable*/
						pAd->ApCfg.MBSSID[apidx].bHideSsid = FALSE;

					DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) HideSSID=%d\n", i, pAd->ApCfg.MBSSID[apidx].bHideSsid));
				}
			}

			/*StationKeepAlive*/
			if(RTMPGetKeyParameter("StationKeepAlive", tmpbuf, 32, pBuffer, TRUE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			    {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[apidx].StationKeepAliveTime = simple_strtol(macptr, 0, 10);
					DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) StationKeepAliveTime=%d\n", i, pAd->ApCfg.MBSSID[apidx].StationKeepAliveTime));
				}
			}

			/*AutoChannelSelect*/
			if(RTMPGetKeyParameter("AutoChannelSelect", tmpbuf, 10, pBuffer, TRUE))
			{
				if(simple_strtol(tmpbuf, 0, 10) != 0)  /*Enable*/
				{
					ChannelSel_Alg SelAlg=(ChannelSel_Alg)simple_strtol(tmpbuf, 0, 10);
					if (SelAlg > 2 || SelAlg < 0)
					{
						pAd->ApCfg.bAutoChannelAtBootup = FALSE;
					}
					else /*Enable*/
					{
						pAd->ApCfg.bAutoChannelAtBootup = TRUE;
						pAd->ApCfg.AutoChannelAlg = SelAlg;
					}
				}
				else /*Disable*/
					pAd->ApCfg.bAutoChannelAtBootup = FALSE;
				DBGPRINT(RT_DEBUG_TRACE, ("AutoChannelAtBootup=%d\n", pAd->ApCfg.bAutoChannelAtBootup));
			}

			/*AutoChannelSkipList*/
			if (RTMPGetKeyParameter("AutoChannelSkipList", tmpbuf, 50, pBuffer, FALSE))
			{
				pAd->ApCfg.AutoChannelSkipListNum = delimitcnt(tmpbuf, ";") + 1;
				if ( pAd->ApCfg.AutoChannelSkipListNum > 10 )
				{
					DBGPRINT(RT_DEBUG_TRACE, ("Your no. of AutoChannelSkipList( %d ) is larger than 10 (boundary)\n",pAd->ApCfg.AutoChannelSkipListNum));
					pAd->ApCfg.AutoChannelSkipListNum = 10;
				}

				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr ; macptr = rstrtok(NULL,";"), i++)
				{
					if (i < pAd->ApCfg.AutoChannelSkipListNum )
					{
						pAd->ApCfg.AutoChannelSkipList[i] = simple_strtol(macptr, 0, 10);
						DBGPRINT(RT_DEBUG_TRACE, (" AutoChannelSkipList[%d]= %d \n", i, pAd->ApCfg.AutoChannelSkipList[i]));
					}
					else
					{
						break;
					}
				}
			}

#ifdef AP_SCAN_SUPPORT
			/*ACSCheckTime*/
			if (RTMPGetKeyParameter("ACSCheckTime", tmpbuf, 32, pBuffer, TRUE))
			{
				UINT8 Hour = simple_strtol(tmpbuf, 0, 10);
				pAd->ApCfg.ACSCheckTime = Hour*3600; /* Hour to second */
				DBGPRINT(RT_DEBUG_TRACE, ("ACSCheckTime = %u (hour) \n", Hour));
			}
#endif /* AP_SCAN_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		/*ShortSlot*/
		if(RTMPGetKeyParameter("ShortSlot", tmpbuf, 10, pBuffer, TRUE))
		{
			RT_CfgSetShortSlot(pAd, tmpbuf);
			DBGPRINT(RT_DEBUG_TRACE, ("ShortSlot=%d\n", pAd->CommonCfg.bUseShortSlotTime));
		}

		if (pAd->chipCap.FlgHwTxBfCap)
		{
#if defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT)
			/*ITxBfEn*/
			if(RTMPGetKeyParameter("ITxBfEn", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn = (simple_strtol(tmpbuf, 0, 10) != 0);
				DBGPRINT(RT_DEBUG_TRACE, ("ITxBfEn = %d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn));

				rtmp_asic_set_bf(pAd);
			}

			/* ITxBfTimeout */
			if(RTMPGetKeyParameter("ITxBfTimeout", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ITxBfTimeout = simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_TRACE, ("ITxBfTimeout = %ld\n", pAd->CommonCfg.ITxBfTimeout));
			}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT) */

			/* ETxBfEnCond*/
			if(RTMPGetKeyParameter("ETxBfEnCond", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ETxBfEnCond = simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_TRACE, ("ETxBfEnCond = %ld\n", pAd->CommonCfg.ETxBfEnCond));

				if (pAd->CommonCfg.ETxBfEnCond)
				{
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = TRUE;
				}
				else
				{
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = FALSE;
			}
				rtmp_asic_set_bf(pAd);
			}

			/* ETxBfTimeout*/
			if(RTMPGetKeyParameter("ETxBfTimeout", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ETxBfTimeout = simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_TRACE, ("ETxBfTimeout = %ld\n", pAd->CommonCfg.ETxBfTimeout));
			}

			/* ETxBfNoncompress*/
			if(RTMPGetKeyParameter("ETxBfNoncompress", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ETxBfNoncompress = simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_TRACE, ("ETxBfNoncompress = %d\n", pAd->CommonCfg.ETxBfNoncompress));
			}

			/* ETxBfIncapable */
			if(RTMPGetKeyParameter("ETxBfIncapable", tmpbuf, 32, pBuffer, TRUE))
			{
				pAd->CommonCfg.ETxBfIncapable = simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_TRACE, ("ETxBfIncapable = %d\n", pAd->CommonCfg.ETxBfIncapable));
			}
		}


#ifdef PRE_ANT_SWITCH
		/*PreAntSwitch*/
		if(RTMPGetKeyParameter("PreAntSwitch", tmpbuf, 32, pBuffer, TRUE))
		{
			pAd->CommonCfg.PreAntSwitch = (simple_strtol(tmpbuf, 0, 10) != 0);
			DBGPRINT(RT_DEBUG_TRACE, ("PreAntSwitch = %d\n", pAd->CommonCfg.PreAntSwitch));
		}
#endif /* PRE_ANT_SWITCH */




#ifdef DBG_CTRL_SUPPORT
		/*DebugFlags*/
		if(RTMPGetKeyParameter("DebugFlags", tmpbuf, 32, pBuffer, TRUE))
		{
			pAd->CommonCfg.DebugFlags = simple_strtol(tmpbuf, 0, 16);
			DBGPRINT(RT_DEBUG_TRACE, ("DebugFlags = 0x%02lx\n", pAd->CommonCfg.DebugFlags));
		}
#endif /* DBG_CTRL_SUPPORT */

		/*IEEE80211H*/
		if(RTMPGetKeyParameter("IEEE80211H", tmpbuf, 10, pBuffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				if(simple_strtol(macptr, 0, 10) != 0)  /*Enable*/
					pAd->CommonCfg.bIEEE80211H = TRUE;
				else /*Disable*/
					pAd->CommonCfg.bIEEE80211H = FALSE;

				DBGPRINT(RT_DEBUG_TRACE, ("IEEE80211H=%d\n", pAd->CommonCfg.bIEEE80211H));
		    }
		}

		/*RDRegion*/
		if(RTMPGetKeyParameter("RDRegion", tmpbuf, 128, pBuffer, TRUE))
		{
			if ((strncmp(tmpbuf, "JAP_W53", 7) == 0) || (strncmp(tmpbuf, "jap_w53", 7) == 0))
			{
							pAd->CommonCfg.RDDurRegion = JAP_W53;
							/*pRadarDetect->DfsSessionTime = 15;*/
			}
			else if ((strncmp(tmpbuf, "JAP_W56", 7) == 0) || (strncmp(tmpbuf, "jap_w56", 7) == 0))
			{
							pAd->CommonCfg.RDDurRegion = JAP_W56;
							/*pRadarDetect->DfsSessionTime = 13;*/
			}
			else if ((strncmp(tmpbuf, "JAP", 3) == 0) || (strncmp(tmpbuf, "jap", 3) == 0))
			{
							pAd->CommonCfg.RDDurRegion = JAP;
							/*pRadarDetect->DfsSessionTime = 5;*/
			}
			else  if ((strncmp(tmpbuf, "FCC", 3) == 0) || (strncmp(tmpbuf, "fcc", 3) == 0))
			{
							pAd->CommonCfg.RDDurRegion = FCC;
							/*pRadarDetect->DfsSessionTime = 5;*/
			}
			else if ((strncmp(tmpbuf, "CE", 2) == 0) || (strncmp(tmpbuf, "ce", 2) == 0))
			{
							pAd->CommonCfg.RDDurRegion = CE;
							/*pRadarDetect->DfsSessionTime = 13;*/
			}
			else
			{
							pAd->CommonCfg.RDDurRegion = CE;
							/*pRadarDetect->DfsSessionTime = 13;*/
			}

						DBGPRINT(RT_DEBUG_TRACE, ("RDRegion=%d\n", pAd->CommonCfg.RDDurRegion));
		}
		else
		{
			pAd->CommonCfg.RDDurRegion = CE;
			/*pRadarDetect->DfsSessionTime = 13;*/
		}

#ifdef SYSTEM_LOG_SUPPORT
		/*WirelessEvent*/
		if(RTMPGetKeyParameter("WirelessEvent", tmpbuf, 10, pBuffer, TRUE))
		{
			bool FlgIsWEntSup = FALSE;

			if(simple_strtol(tmpbuf, 0, 10) != 0)
				FlgIsWEntSup = TRUE;

			RtmpOsWlanEventSet(pAd, &pAd->CommonCfg.bWirelessEvent, FlgIsWEntSup);
			DBGPRINT(RT_DEBUG_TRACE, ("WirelessEvent=%d\n", pAd->CommonCfg.bWirelessEvent));
		}
#endif /* SYSTEM_LOG_SUPPORT */


		/*AuthMode*/
		if(RTMPGetKeyParameter("AuthMode", tmpbuf, 128, pBuffer, TRUE))
		{
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
		   		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), i++)
		    	{
					ApCfg_Set_AuthMode_Proc(pAd, i, macptr);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				struct rtmp_wifi_dev *wdev = &pAd->StaCfg.wdev;

				if (rtstrcasecmp(tmpbuf, "WEPAUTO") == TRUE)
					wdev->AuthMode = Ndis802_11AuthModeAutoSwitch;
				else if (rtstrcasecmp(tmpbuf, "SHARED") == TRUE)
					wdev->AuthMode = Ndis802_11AuthModeShared;
				else if (rtstrcasecmp(tmpbuf, "WPAPSK") == TRUE)
					wdev->AuthMode = Ndis802_11AuthModeWPAPSK;
				else if (rtstrcasecmp(tmpbuf, "WPANONE") == TRUE)
					wdev->AuthMode = Ndis802_11AuthModeWPANone;
				else if (rtstrcasecmp(tmpbuf, "WPA2PSK") == TRUE)
					wdev->AuthMode = Ndis802_11AuthModeWPA2PSK;
#ifdef WPA_SUPPLICANT_SUPPORT
				else if (rtstrcasecmp(tmpbuf, "WPA") == TRUE)
					wdev->AuthMode = Ndis802_11AuthModeWPA;
				else if (rtstrcasecmp(tmpbuf, "WPA2") == TRUE)
					wdev->AuthMode = Ndis802_11AuthModeWPA2;
#endif /* WPA_SUPPLICANT_SUPPORT */
				else
					wdev->AuthMode = Ndis802_11AuthModeOpen;

				wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

				DBGPRINT(RT_DEBUG_TRACE, ("%s::(AuthMode=%d)\n",
							__FUNCTION__, wdev->AuthMode));
			}
#endif /* CONFIG_STA_SUPPORT */
		}
		/*EncrypType*/
		if(RTMPGetKeyParameter("EncrypType", tmpbuf, 128, pBuffer, TRUE))
		{
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				/*
					We need to reset the WepStatus of all interfaces as 1 (Ndis802_11WEPDisabled) first.
					Or it may have problem when some interface enabled but didn't configure it.
				*/
				for ( i= 0; i<pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[i].wdev.WepStatus = Ndis802_11WEPDisabled;

		    	for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    	{
					int apidx;
					struct rtmp_wifi_dev *wdev;
					if (i < HW_BEACON_MAX_NUM)
						apidx = i;
		        	else
						break;

					wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
					if ((strncmp(macptr, "NONE", 4) == 0) || (strncmp(macptr, "none", 4) == 0))
		            	wdev->WepStatus = Ndis802_11WEPDisabled;
		        	else if ((strncmp(macptr, "WEP", 3) == 0) || (strncmp(macptr, "wep", 3) == 0))
		            	wdev->WepStatus = Ndis802_11WEPEnabled;
		        	else if ((strncmp(macptr, "TKIPAES", 7) == 0) || (strncmp(macptr, "tkipaes", 7) == 0))
		            	wdev->WepStatus = Ndis802_11TKIPAESMix;
		        	else if ((strncmp(macptr, "TKIP", 4) == 0) || (strncmp(macptr, "tkip", 4) == 0))
		            	wdev->WepStatus = Ndis802_11TKIPEnable;
		        	else if ((strncmp(macptr, "AES", 3) == 0) || (strncmp(macptr, "aes", 3) == 0))
		            	wdev->WepStatus = Ndis802_11AESEnable;
		        	else
		            	wdev->WepStatus = Ndis802_11WEPDisabled;

					/* decide the group key encryption type*/
					if (wdev->WepStatus == Ndis802_11TKIPAESMix)
						wdev->GroupKeyWepStatus = Ndis802_11TKIPEnable;
					else
						wdev->GroupKeyWepStatus = wdev->WepStatus;

					/* move to ap.c::APStartUp to process*/
	        		/*RTMPMakeRSNIE(pAd, pAd->ApCfg.MBSSID[apidx].AuthMode, wdev->WepStatus, apidx);*/
		        	DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) EncrypType=%d\n", i, wdev->WepStatus));
		    	}
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				struct rtmp_wifi_dev *wdev = &pAd->StaCfg.wdev;
				if (rtstrcasecmp(tmpbuf, "WEP") == TRUE)
					wdev->WepStatus = Ndis802_11WEPEnabled;
				else if (rtstrcasecmp(tmpbuf, "TKIP") == TRUE)
					wdev->WepStatus = Ndis802_11TKIPEnable;
				else if (rtstrcasecmp(tmpbuf, "AES") == TRUE)
					wdev->WepStatus = Ndis802_11AESEnable;
				else
					wdev->WepStatus = Ndis802_11WEPDisabled;
				RTMPSetSTACipherSuites(pAd, wdev->WepStatus);
				/*RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, 0);*/
				DBGPRINT(RT_DEBUG_TRACE, ("%s::(EncrypType=%d)\n", __FUNCTION__, wdev->WepStatus));
			}
		#endif /* CONFIG_STA_SUPPORT */
		}

#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
					/* WpaMixPairCipher*/
					if(RTMPGetKeyParameter("WpaMixPairCipher", tmpbuf, 256, pBuffer, TRUE))
					{
						/*
							In WPA-WPA2 mix mode, it provides a more flexible cipher combination.
							-	WPA-AES and WPA2-TKIP
							-	WPA-AES and WPA2-TKIPAES
							-	WPA-TKIP and WPA2-AES
							-	WPA-TKIP and WPA2-TKIPAES
							-	WPA-TKIPAES and WPA2-AES
							-	WPA-TKIPAES and WPA2-TKIP
							-	WPA-TKIPAES and WPA2-TKIPAES (default)
						 */
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							struct rtmp_wifi_dev *wdev;

							// TODO: shiang, check about the sequence of this paramter and BssNum!!
							if (i >= HW_BEACON_MAX_NUM)
								break;

							wdev = &pAd->ApCfg.MBSSID[i].wdev;
							if (wdev->AuthMode != Ndis802_11AuthModeWPA1WPA2 &&
								wdev->AuthMode != Ndis802_11AuthModeWPA1PSKWPA2PSK)
								continue;

							if (wdev->WepStatus != Ndis802_11TKIPAESMix)
								continue;

							if ((strncmp(macptr, "WPA_AES_WPA2_TKIPAES", 20) == 0) || (strncmp(macptr, "wpa_aes_wpa2_tkipaes", 20) == 0))
								wdev->WpaMixPairCipher = WPA_AES_WPA2_TKIPAES;
							else if ((strncmp(macptr, "WPA_AES_WPA2_TKIP", 17) == 0) || (strncmp(macptr, "wpa_aes_wpa2_tkip", 17) == 0))
								wdev->WpaMixPairCipher = WPA_AES_WPA2_TKIP;
							else if ((strncmp(macptr, "WPA_TKIP_WPA2_AES", 17) == 0) || (strncmp(macptr, "wpa_tkip_wpa2_aes", 17) == 0))
								wdev->WpaMixPairCipher = WPA_TKIP_WPA2_AES;
							else if ((strncmp(macptr, "WPA_TKIP_WPA2_TKIPAES", 21) == 0) || (strncmp(macptr, "wpa_tkip_wpa2_tkipaes", 21) == 0))
								wdev->WpaMixPairCipher = WPA_TKIP_WPA2_TKIPAES;
							else if ((strncmp(macptr, "WPA_TKIPAES_WPA2_AES", 20) == 0) || (strncmp(macptr, "wpa_tkipaes_wpa2_aes", 20) == 0))
								wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_AES;
							else if ((strncmp(macptr, "WPA_TKIPAES_WPA2_TKIPAES", 24) == 0) || (strncmp(macptr, "wpa_tkipaes_wpa2_tkipaes", 24) == 0))
								wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES;
							else if ((strncmp(macptr, "WPA_TKIPAES_WPA2_TKIP", 21) == 0) || (strncmp(macptr, "wpa_tkipaes_wpa2_tkip", 21) == 0))
								wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIP;
							else /*Default*/
								wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES;

							DBGPRINT(RT_DEBUG_OFF, ("I/F(ra%d) MixWPACipher=0x%02x\n", i, wdev->WpaMixPairCipher));
						}
					}

					/*RekeyMethod*/
					if(RTMPGetKeyParameter("RekeyMethod", tmpbuf, 128, pBuffer, TRUE))
					{
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							PRT_WPA_REKEY pRekeyInfo = &pAd->ApCfg.MBSSID[i].WPAREKEY;

							if ((strcmp(macptr, "TIME") == 0) || (strcmp(macptr, "time") == 0))
								pRekeyInfo->ReKeyMethod = TIME_REKEY;
							else if ((strcmp(macptr, "PKT") == 0) || (strcmp(macptr, "pkt") == 0))
								pRekeyInfo->ReKeyMethod = PKT_REKEY;
							else if ((strcmp(macptr, "DISABLE") == 0) || (strcmp(macptr, "disable") == 0))
								pRekeyInfo->ReKeyMethod = DISABLE_REKEY;
							else
								pRekeyInfo->ReKeyMethod = DISABLE_REKEY;

							DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) ReKeyMethod=%ld\n", i, pRekeyInfo->ReKeyMethod));
						}

						/* Apply to remaining MBSS*/
						if (i == 1)
						{
							for (i = 1; i < pAd->ApCfg.BssidNum; i++)
							{
								pAd->ApCfg.MBSSID[i].WPAREKEY.ReKeyMethod =
										pAd->ApCfg.MBSSID[0].WPAREKEY.ReKeyMethod;
								DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) ReKeyMethod=%ld\n",
													i, pAd->ApCfg.MBSSID[i].WPAREKEY.ReKeyMethod));
							}
						}
					}
					/*RekeyInterval*/
					if(RTMPGetKeyParameter("RekeyInterval", tmpbuf, 255, pBuffer, TRUE))
					{
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							ULONG	value_interval;
							PRT_WPA_REKEY pRekeyInfo = &pAd->ApCfg.MBSSID[i].WPAREKEY;

							value_interval = simple_strtol(macptr, 0, 10);

							if((value_interval >= 10) && (value_interval < MAX_REKEY_INTER))
								pRekeyInfo->ReKeyInterval = value_interval;
							else /*Default*/
								pRekeyInfo->ReKeyInterval = 3600;

							DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) ReKeyInterval=%ld\n",
															i, pRekeyInfo->ReKeyInterval));
						}

						/* Apply to remaining MBSS*/
						if (i == 1)
						{
							for (i = 1; i < pAd->ApCfg.BssidNum; i++)
							{
								pAd->ApCfg.MBSSID[i].WPAREKEY.ReKeyInterval =
										pAd->ApCfg.MBSSID[0].WPAREKEY.ReKeyInterval;
								DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) ReKeyInterval=%ld\n",
															i, pAd->ApCfg.MBSSID[i].WPAREKEY.ReKeyInterval));
							}
						}

					}
					/*PMKCachePeriod*/
					if(RTMPGetKeyParameter("PMKCachePeriod", tmpbuf, 255, pBuffer, TRUE))
					{
						for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
						{
							pAd->ApCfg.MBSSID[i].PMKCachePeriod =
													simple_strtol(macptr, 0, 10) * 60 * OS_HZ;

							DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) PMKCachePeriod=%ld\n",
														i, pAd->ApCfg.MBSSID[i].PMKCachePeriod));
						}

						/* Apply to remaining MBSS*/
						if (i == 1)
						{
							for (i = 1; i < pAd->ApCfg.BssidNum; i++)
							{
								pAd->ApCfg.MBSSID[i].PMKCachePeriod =
										pAd->ApCfg.MBSSID[0].PMKCachePeriod;

								DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) PMKCachePeriod=%ld\n",
															i, pAd->ApCfg.MBSSID[i].PMKCachePeriod));
							}
						}
					}

					/*WPAPSK_KEY*/
					if(TRUE)
					{
						STRING tok_str[16];
						bool bWPAPSKxIsUsed = FALSE;

						//DBGPRINT(RT_DEBUG_TRACE, ("pAd->ApCfg.BssidNum=%d\n", pAd->ApCfg.BssidNum));
						for (i = 0; i < pAd->ApCfg.BssidNum; i++)
						{
							snprintf(tok_str, sizeof(tok_str), "WPAPSK%d", i + 1);
						if(RTMPGetKeyParameter(tok_str, tmpbuf, 65, pBuffer, FALSE))
							{
								rtmp_parse_wpapsk_buffer_from_file(pAd, tmpbuf, i);

								if (bWPAPSKxIsUsed == FALSE)
								{
									bWPAPSKxIsUsed = TRUE;
								}
							}
						}
						if (bWPAPSKxIsUsed == FALSE)
						{
						if (RTMPGetKeyParameter("WPAPSK", tmpbuf, 512, pBuffer, FALSE))
							{
								if (pAd->ApCfg.BssidNum == 1)
								{
									rtmp_parse_wpapsk_buffer_from_file(pAd, tmpbuf, BSS0);
								}
								else
								{
									/* Anyway, we still do the legacy dissection of the whole WPAPSK passphrase.*/
									for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
									{
										rtmp_parse_wpapsk_buffer_from_file(pAd, macptr, i);
									}

								}
							}
						}

#ifdef DBG
						for (i = 0; i < pAd->ApCfg.BssidNum; i++)
						{
							int j;

											DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) WPAPSK Key => \n", i));
											for (j = 0; j < 32; j++)
											{
												DBGPRINT(RT_DEBUG_TRACE, ("%02x:", pAd->ApCfg.MBSSID[i].PMK[j]));
												if ((j%16) == 15)
														DBGPRINT(RT_DEBUG_TRACE, ("\n"));
											}
											DBGPRINT(RT_DEBUG_TRACE, ("\n"));
						}
#endif
					}
				}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
					if(RTMPGetKeyParameter("WPAPSK", tmpbuf, 512, pBuffer, FALSE))
						RTMPSetSTAPassPhrase(pAd, tmpbuf);
				}
#endif /* CONFIG_STA_SUPPORT */

				/*DefaultKeyID, KeyType, KeyStr*/
				rtmp_read_key_parms_from_file(pAd, tmpbuf, pBuffer);



#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
					/*Access Control List*/
					rtmp_read_acl_parms_from_file(pAd, tmpbuf, pBuffer);

#ifdef APCLI_SUPPORT
					rtmp_read_ap_client_from_file(pAd, tmpbuf, pBuffer);
#endif /* APCLI_SUPPORT */



#ifdef DOT1X_SUPPORT
					rtmp_read_radius_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT1X_SUPPORT */


				}

#endif /* CONFIG_AP_SUPPORT */

				HTParametersHook(pAd, tmpbuf, pBuffer);

				VHTParametersHook(pAd, tmpbuf, pBuffer);


#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{


				}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
					/*PSMode*/
					if (RTMPGetKeyParameter("PSMode", tmpbuf, 10, pBuffer, TRUE))
					{
						if (pAd->StaCfg.BssType == BSS_INFRA)
						{
							if ((strcmp(tmpbuf, "MAX_PSP") == 0) || (strcmp(tmpbuf, "max_psp") == 0))
							{
								/*
									do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange()
									to exclude certain situations
								*/
								/*	MlmeSetPsm(pAd, PWR_SAVE);*/
								OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
								if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
									pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;
								pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
								pAd->StaCfg.DefaultListenCount = 5;
							}
							else if ((strcmp(tmpbuf, "Fast_PSP") == 0) || (strcmp(tmpbuf, "fast_psp") == 0)
								|| (strcmp(tmpbuf, "FAST_PSP") == 0))
							{
								/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange()*/
								/* to exclude certain situations.*/
								OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
								if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
									pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeFast_PSP;
								pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
								pAd->StaCfg.DefaultListenCount = 3;
							}
							else if ((strcmp(tmpbuf, "Legacy_PSP") == 0) || (strcmp(tmpbuf, "legacy_psp") == 0)
								|| (strcmp(tmpbuf, "LEGACY_PSP") == 0))
							{
								/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange()*/
								/* to exclude certain situations.*/
								OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
								if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
									pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;
								pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
								pAd->StaCfg.DefaultListenCount = 3;
							}
							else
							{ /*Default Ndis802_11PowerModeCAM*/
								/* clear PSM bit immediately*/
								RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
								OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
								if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
									pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeCAM;
								pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
							}
							DBGPRINT(RT_DEBUG_TRACE, ("PSMode=%ld\n", pAd->StaCfg.WindowsPowerMode));
						}
					}
					/* AutoRoaming by RSSI*/
					if (RTMPGetKeyParameter("AutoRoaming", tmpbuf, 32, pBuffer, TRUE))
					{
						if (simple_strtol(tmpbuf, 0, 10) == 0)
							pAd->StaCfg.bAutoRoaming = FALSE;
						else
							pAd->StaCfg.bAutoRoaming = TRUE;

						DBGPRINT(RT_DEBUG_TRACE, ("AutoRoaming=%d\n", pAd->StaCfg.bAutoRoaming));
					}
					/* RoamThreshold*/
					if (RTMPGetKeyParameter("RoamThreshold", tmpbuf, 32, pBuffer, TRUE))
					{
						long lInfo = simple_strtol(tmpbuf, 0, 10);

						if (lInfo > 90 || lInfo < 60)
							pAd->StaCfg.dBmToRoam = -70;
						else
							pAd->StaCfg.dBmToRoam = (CHAR)(-1)*lInfo;

						DBGPRINT(RT_DEBUG_TRACE, ("RoamThreshold=%d  dBm\n", pAd->StaCfg.dBmToRoam));
					}




					if(RTMPGetKeyParameter("TGnWifiTest", tmpbuf, 10, pBuffer, TRUE))
					{
						if(simple_strtol(tmpbuf, 0, 10) == 0)
							pAd->StaCfg.bTGnWifiTest = FALSE;
						else
							pAd->StaCfg.bTGnWifiTest = TRUE;
							DBGPRINT(RT_DEBUG_TRACE, ("TGnWifiTest=%d\n", pAd->StaCfg.bTGnWifiTest));
					}

					/* Beacon Lost Time*/
					if (RTMPGetKeyParameter("BeaconLostTime", tmpbuf, 32, pBuffer, TRUE))
					{
						ULONG lInfo = (ULONG)simple_strtol(tmpbuf, 0, 10);

						if ((lInfo != 0) && (lInfo <= 60))
							pAd->StaCfg.BeaconLostTime = (lInfo * OS_HZ);
						DBGPRINT(RT_DEBUG_TRACE, ("BeaconLostTime=%ld \n", pAd->StaCfg.BeaconLostTime));
					}

					/* Auto Connet Setting if no SSID			*/
					if (RTMPGetKeyParameter("AutoConnect", tmpbuf, 32, pBuffer, TRUE))
					{
						if (simple_strtol(tmpbuf, 0, 10) == 0)
							pAd->StaCfg.bAutoConnectIfNoSSID = FALSE;
						else
							pAd->StaCfg.bAutoConnectIfNoSSID = TRUE;
					}



					/* FastConnect*/
					if(RTMPGetKeyParameter("FastConnect", tmpbuf, 32, pBuffer, TRUE))
					{
						if (simple_strtol(tmpbuf, 0, 10) == 0)
							pAd->StaCfg.bFastConnect = FALSE;
						else
							pAd->StaCfg.bFastConnect = TRUE;

						DBGPRINT(RT_DEBUG_TRACE, ("FastConnect=%d\n", pAd->StaCfg.bFastConnect));
					}
				}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
				}
#endif /* CONFIG_AP_SUPPORT */



#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
	                            rtmp_read_pmf_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11W_PMF_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
				/* EntryLifeCheck is used to check */
				if (RTMPGetKeyParameter("EntryLifeCheck", tmpbuf, 256, pBuffer, TRUE))
				{
					long LifeCheckCnt = simple_strtol(tmpbuf, 0, 10);
					if ((LifeCheckCnt <= 65535) && (LifeCheckCnt != 0))
						pAd->ApCfg.EntryLifeCheck = LifeCheckCnt;
					else
						pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;

					DBGPRINT(RT_DEBUG_ERROR, ("EntryLifeCheck=%ld\n", pAd->ApCfg.EntryLifeCheck));
				}

#endif /* CONFIG_AP_SUPPORT */


#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
		/* set GPIO pin for wake-up signal */
		if (RTMPGetKeyParameter("WOW_GPIO", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_GPIO(pAd, tmpbuf);

		/* set WOW enable/disable */
		if (RTMPGetKeyParameter("WOW_Enable", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Enable(pAd, tmpbuf);

		/* set delay time for WOW really enable */
		if (RTMPGetKeyParameter("WOW_Delay", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Delay(pAd, tmpbuf);

		/* set GPIO pulse hold time */
		if (RTMPGetKeyParameter("WOW_Hold", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Hold(pAd, tmpbuf);

		/* set wakeup signal type */
		if (RTMPGetKeyParameter("WOW_InBand", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_InBand(pAd, tmpbuf);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#ifdef RTMP_USB_SUPPORT
		if (RTMPGetKeyParameter("USBAggregation", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->usb_ctl.usb_aggregation = simple_strtol(tmpbuf, 0, 10);
			DBGPRINT(RT_DEBUG_OFF, ("USBAggregation = %d\n", pAd->usb_ctl.usb_aggregation));
		}
#endif /* RTMP_USB_SUPPORT */

	}while(0);

	kfree(tmpbuf);

	return NDIS_STATUS_SUCCESS;
}

uint32_t RalinkRate_Legacy [] = {2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108};
uint32_t RalinkRate_HT_1NSS[Rate_BW_MAX][Rate_GI_MAX][Rate_MCS] =
{
	{
		{13, 26, 39, 52,  78,  104, 117, 130, 0, 0},{14, 29, 43,  57, 87, 115, 130, 144, 0, 0}     /*20MHz, 800ns & 400 ns GI, MCS0~9*/
	},
	{
		{27, 54, 81, 108, 162, 216, 243, 270, 0, 0},{30, 60, 90, 120, 180, 240, 270, 300, 0, 0}	   /*40MHz, 800ns & 400 ns GI, MCS0~9*/
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}			   ,{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}	   			  /*80MHz, 800ns & 400 ns GI, MCS0~9*/
	},
};
uint32_t RalinkRate_VHT_1NSS[Rate_BW_MAX][Rate_GI_MAX][Rate_MCS] =
{
	{
		{13,  26,  39, 52,  78,  104, 117, 130, 156,  0} , {14,  29,  43,  57,  87, 115, 130, 144, 173,   0}      /*20MHz, 800ns & 400 ns GI, MCS0~9*/
	},
	{
		{27,  54,  81, 108, 162, 216, 243, 270, 324, 360}, {30,  60,  90, 120, 180, 240, 270, 300, 360, 400}	 /*40MHz, 800ns & 400 ns GI, MCS0~9*/
	},
	{
		{59, 117, 176, 234, 351, 468, 527, 585, 702, 780}, {65, 130, 195, 260, 390, 520, 585, 650, 780, 867}	  /*80MHz, 800ns & 400 ns GI, MCS0~9*/
	},
};

UINT8 newRateGetAntenna(UINT8 MCS)
{
	return ((MCS>>4) + 1);
}



void print_RalinkRate_HT(void)
{
	uint32_t i,j,k;

	for(i=0;i < Rate_BW_MAX;i++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("======= BW : %d ============\n", i));
		for(j=0;j < Rate_GI_MAX;j++)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("======= GI : %d ============\nMCS: ", j));
			for(k=0;k < Rate_MCS;k++)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("%d, ", RalinkRate_HT_1NSS[i][j][k]));
			}
			DBGPRINT(RT_DEBUG_TRACE, ("\n======= END GI : %d ============\n", j));
		}
		DBGPRINT(RT_DEBUG_TRACE, ("======= END BW : %d ============\n", i));
	}

	for(i=0;i < Rate_BW_MAX;i++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("======= BW : %d ============\n", i));
		for(j=0;j < Rate_GI_MAX;j++)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("======= GI : %d ============\nMCS: ", j));
			for(k=0;k < Rate_MCS;k++)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("%d, ", RalinkRate_VHT_1NSS[i][j][k]));
			}
			DBGPRINT(RT_DEBUG_TRACE, ("\n======= END GI : %d ============\n", j));
		}
		DBGPRINT(RT_DEBUG_TRACE, ("======= END BW : %d ============\n", i));
	}
}

