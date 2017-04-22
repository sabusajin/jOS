#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#endif	// JOS_KERN_E1000_H

#include <kern/pci.h>
#include <kern/pmap.h>

#define E1000_STATUS   0x00008/4
#define E1000_DEV_ID_82540EM	0x100E

int e1000_attach(struct pci_func *pcif);
volatile uint32_t *mmio_e1000;
