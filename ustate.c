#include "deffs.h"

struct GSM_STATE
{
     unsigned long signature;
     unsigned long restarts;
     unsigned short current_sector;
     unsigned short sd_data;
};

union UGSM_STATE
{
    struct GSM_STATE st;
    unsigned char data[STATE_SIZE];
};
union UGSM_STATE uState;

unsigned short uStateInit(void)
{
    GetSdCardBase(&uState.data[0]);
    if(uState.st.signature != SIGNATURE)
    {
        uState.st.signature = SIGNATURE;
        uState.st.restarts = 0;
        uState.st.current_sector = (unsigned short)START_SECTOR;
        uState.st.sd_data = 0;
    }
    else
    {
        uState.st.restarts++;
    }
#ifdef DPRINT
  printf("\n\r restarts : %d ",(unsigned int)uState.st.restarts);
#endif

   if(key_pressed())
   {
#ifdef DPRINT
       printf("\n\r reset parameters!!!");
#endif
       uState.st.current_sector =  (unsigned short)START_SECTOR;
       uState.st.sd_data = 0;
       uState.st.restarts = 0;
   }
   sd_data = uState.st.sd_data;  //init hier
   cursector =  uState.st.current_sector;
   return (unsigned short)uState.st.current_sector;
}

void uStateKeep(void)
{
    uState.st.current_sector =  cursector;
    uState.st.sd_data =  sd_data;
    AddSdCardBase(&uState.data[0]);
}

