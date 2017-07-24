#include "deffs.h"

struct rec
{
    char body[MSG_LEN]; //use first symbol
};

struct GSM_LIFO
{
   struct rec Buff[LIFO_SIZE];
};

union GSM_LIFO_BUFF
{
    struct GSM_LIFO glifo;
    unsigned char data[LIFO_SIZE *(MSG_LEN)];
};

union GSM_LIFO_BUFF ulifo;
unsigned int  LHead;
unsigned int LLastHead;

void LBInit()
{
   LHead = 0;
   LLastHead = 0;
}

void Lifo_Add(char * d)
{
    int i;
    if(LHead > (LIFO_SIZE - 1))
    {
        AddSdCard((unsigned char*)&ulifo.data[0]);
        LHead = 0;
        LLastHead = 0;
    }
    for(i = 0; i < MSG_LEN; i++)
    {
        ulifo.glifo.Buff[LHead].body[i] = d[i];  //first symbol is  need now
    }
    LHead++;
    if(LLastHead < LIFO_SIZE) LLastHead++;
}

unsigned int Lifo_Used(void)
{
   if(!LLastHead)
   {
     if(GetSdCard((unsigned char *)&ulifo.data[0]))
       {
          LHead = LIFO_SIZE;  //will be incremented
          LLastHead = LIFO_SIZE;
       }
   }
    return LLastHead;
}

#ifdef SEND_6
#ifdef SEND_DIFF
unsigned char try_convert_frame_to_diff(char *d)
 {
    char *p = d;
     unsigned char type = 1; //returned frame type
     static long latp, lonp, altp, coursep, speedp; //time do not use
     long dlat, dlon, dalt, dcourse, dspeed;
     long lat, lon, alt, course, speed;
     d += 4; //time do not use
     memcpy(&lat, d, sizeof(long)); //need check where is every parameter
     memcpy(&lon, d + sizeof(long), sizeof(long));
     memcpy(&alt, d + sizeof(long), sizeof(long) <<1);
     memcpy(&course, d + sizeof(long), sizeof(long) * 3);
     memcpy(&speed, d + sizeof(long), sizeof(long)<<2);
     if(connect_flag)
     {
         connect_flag = 0;
         latp = lat;
         lonp = lon;
         altp = alt;
         coursep = course;
         speedp = speed;
         return type; //control frame
     }
  dlat = lat - latp;
  dlon = lon - lonp;
  dalt = alt - altp;
  dcourse = course - coursep;
  dspeed = speed - speedp;
  if((abs(dlat) < 127) && (abs(dlon) < 127) && (abs(dalt) < 127) && (abs(dcourse) < 127) && (abs(dspeed) < 127))
  {
      //hier can make diff frame
      type = 0;
      *p++ = 0;
      *p++ = dlat;
      *p++ = dlon;
      *p++ = dalt;
      *p++ = dcourse;
      *p = dspeed;
  }
  else
  {
    latp = lat;
    lonp = lon;
    altp = alt;
    coursep = course;
    speedp = speed;
  //keep values in control frame
  }
  return type;
 }
#endif
#endif

unsigned char Lifo_Get(char * p, char * p1)
{
    char *d  = p;
    char *d1  = p1;
    int i;
    unsigned short have_data;
    unsigned char type;
    if(LHead >= 1)
    {
        LHead--;
    }
    else
    {
        have_data = GetSdCard((unsigned char *)&ulifo.data[0]);
         if( have_data > 0)
              {
                 LHead = LIFO_SIZE - 1;
                 LLastHead = LIFO_SIZE;
               }
      }
    if(LLastHead)
   {
     LLastHead--;
     type =  ulifo.glifo.Buff[LHead].body[0];
     for(i = 0; i < SEND_DATA_SIZE; i++) //first symbol used
     {
       *d = ulifo.glifo.Buff[LHead].body[i];  //hier need right restore diff packets (lower length) if d[0] == 1 contorl frame if d[0] == 0 diff frame
       *d1 = ulifo.glifo.Buff[LHead].body[i];  //hier need right restore diff packets (lower length) if d[0] == 1 contorl frame if d[0] == 0 diff frame
       d++; d1++;
     }
#ifdef SEND_6
#ifdef SEND_DIFF
     type = try_convert_frame_to_diff(d);
#endif
#endif
   }
    return type;
}

unsigned short UseLifo(char *Target, char *Target1)
{
    unsigned char frame_type;
    unsigned short fill_size = 0;
    unsigned short enabled_size = MAX_GET_LiFO * SEND_DATA_SIZE - used_len;
    while(enabled_size >= SEND_DATA_SIZE)
    {
        if(Lifo_Used())
        {
          frame_type =  Lifo_Get(&Target[fill_size], &Target1[fill_size]);  //do not fill first symbols
          if(!frame_type) { fill_size += SEND_DATA_SIZE_DIFF; enabled_size -= SEND_DATA_SIZE_DIFF;}
          else  { fill_size += SEND_DATA_SIZE; enabled_size -= SEND_DATA_SIZE;}
        }
        else break;
    }
 return fill_size;
}
