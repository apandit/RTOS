/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
Documentation file is called 

To Compile for CF-SERVER
comment out
CFLAGS= -Wall -m5307 -pipe -nostdlib -D_DEBUG_HOTKEYS
with # in front
and uncomment
CFLAGS= -Wall -m5307 -pipe -nostdlib -D_CFSERVER_ -D_DEBUG_HOTKEYS

To Compile for Coldfire Boards in Lab
comment out
CFLAGS= -Wall -m5307 -pipe -nostdlib -D_CFSERVER_ -D_DEBUG_HOTKEYS
with # in front
and uncomment
CFLAGS= -Wall -m5307 -pipe -nostdlib -D_DEBUG_HOTKEYS