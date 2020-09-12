
#include <gba_interrupt.h>
#include <gba_systemcalls.h>


int main(void) {
    irqInit();
    irqEnable(IRQ_VBLANK);

    while (1) {
        VBlankIntrWait();
    }
}


