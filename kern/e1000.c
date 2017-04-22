#include <kern/e1000.h>


// LAB 6: Your driver code here

int e1000_attach(struct pci_func *pcif)
{
	pci_func_enable(pcif);
	cprintf ("%x\n", pcif->reg_base[0]);
	cprintf ("%x\n", pcif->reg_size[0]);
	mmio_e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
	
	cprintf("%x\n",mmio_e1000[E1000_STATUS]);
	assert(mmio_e1000[E1000_STATUS] == 0x80080783);
	return 0;
}
	
