#if !defined(ISR80H_MISC_H)
#define ISR80H_MISC_H

struct interrupt_frame;

void* isr80h_command0_sum(struct interrupt_frame* frame);
void* isr80h_command1_print(struct interrupt_frame* frame);
void* isr80h_command2_putchar(struct interrupt_frame* frame);
void* isr80h_command3_getkey(struct interrupt_frame* frame);

#endif // ISR80H_MISC_H
