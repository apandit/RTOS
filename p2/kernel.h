/**
 * @author: apandit igrabovi ltng jlfeng
 * @brief: ECE 354 S10 RTX Project P2-(c) 
 * @date: 2010/06/05
 */
#ifndef _RTX_KERNEL_H_
#define _RTX_KERNEL_H_

/* Kernel Interprocess Communications*/
void kernel_send_message();
void kernel_receive_message();

/* Kernel Memory Management*/
void kernel_request_memory_block();
void kernel_release_memory_block();

/* Kernel Process Management*/
void kernel_release_processor();

/* Kernel Timing Service*/
void kernel_delayed_send();

/* Kernel Process Priority*/
void kernel_set_process_priority ();
void kernel_get_process_priority ();

// Save context and trigger kernel method
void kernel_save_and_trigger();

#endif
