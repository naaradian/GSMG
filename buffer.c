#include "deffs.h"
const char OK_STR[] = {'O','K', 0};
char RcvBuff[RCV_BUFF_SIZE];
unsigned short rlen;

void BInit()
{
   memset(RcvBuff, 0xFE, RCV_BUFF_SIZE);
   rlen = 0;
}

void parse_bcl(char* Buff)
{
    Buff = strchr(Buff, ',');
    Buff++;
    bcl = (char)strtoul(Buff, NULL, 10);
}

char findchar(char s, char * Buff,int len)
{
     int i;
     for(i = 0; i < len; i++)
     {
        if(s == Buff[i])
         {
             return 1;
         }
     }
     return 0;
}

char ParseBuffer(char type)
{
   char ret = 0;
#ifdef DPRINT
   int i;
#endif
   if(rlen)
   {
    switch (type)
    {
    case 6  :  //hier need parse battery
               parse_bcl(RcvBuff);
               if(strstr(RcvBuff,  OK_STR))
               ret = 1;
               break;
    case 9  :  //receive zeros from server
               if(findchar(0, RcvBuff,rlen)) ret = 1;//server get zero to answer
               break;
    case 10 :  //receive zeros from server
               if(findchar(0, RcvBuff,rlen)) ret = 1;//server get zero to answer
               break;
    default : ret = 1;
    } //switch

#ifdef DPRINT
    if((type != 7) && (type != 3))// && (type != 9) )
         {
            printf("\n\r ");
         }
    for(i = 0; i < rlen; i ++)
        {
           if((type != 7) && (type != 3) && (type != 9) && (type != 10))
           {
               printf("%c", RcvBuff[i] );
           }
           else if ((type == 9) || (type == 10))
           {
              printf("%c (%d)", RcvBuff[i], RcvBuff[i] );
          //     printf("%c", RcvBuff[i] );
           }
        } //for
#endif
  }//if rlen
   BInit();
return ret;
}
