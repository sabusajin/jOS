#include <kern/e1000.h>


// LAB 6: Your driver code here
struct e1000_tx_desc tx_desc_buf[TXRING_LEN] __attribute__ ((aligned (PGSIZE)));
struct packet packet_buf[TXRING_LEN] __attribute__ ((aligned (16)));

static void 
e1000_init()
{
	int i;
	for (i=0; i < TXRING_LEN; i++)
	{
		tx_desc_buf[i].buffer_addr = PADDR(&tx_desc_buf[i]);
		tx_desc_buf[i].upper.fields.status = E1000_TXD_STAT_DD;
	} 
	mmio_e1000[E1000_TDBAL] = PADDR(tx_desc_buf);
	mmio_e1000[E1000_TDLEN] = TXRING_LEN * sizeof(struct e1000_tx_desc);
	mmio_e1000[E1000_TDBAH] = 0x0;
	mmio_e1000[E1000_TDH] = 0x0;
	mmio_e1000[E1000_TDT] = 0x0;
	
	mmio_e1000[E1000_TCTL] |= E1000_TCTL_EN;
	mmio_e1000[E1000_TCTL] |= E1000_TCTL_PSP;
	mmio_e1000[E1000_TCTL] |= E1000_TCTL_CT;	
	mmio_e1000[E1000_TCTL] |= E1000_TCTL_COLD;	

	mmio_e1000[E1000_TIPG] |= (E1000_TIPG_IPGT | E1000_TIPG_IPGR1 | E1000_TIPG_IPGR2);

	
	
}

int e1000_transmit(char *pkt, size_t length)
{
	if (length > PGSIZE)
		length = PGSIZE;
	uint32_t tail = mmio_e1000[E1000_TDT];
	struct e1000_tx_desc *tail_desc = &tx_desc_buf[tail];
	if (tail_desc->upper.fields.status != E1000_TXD_STAT_DD)
		return -1;
	memmove((void *) &packet_buf[tail], (void *) pkt, length);
	tail_desc->lower.flags.length = length;
	tail_desc->upper.fields.status = ~E1000_TXD_STAT_DD;
	tail_desc->lower.data |=  (E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP);
	mmio_e1000[E1000_TDT] = (tail + 1) % TXRING_LEN;
	return 0;
	
}


int 
e1000_attach(struct pci_func *pcif)
{
	pci_func_enable(pcif);
	mmio_e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);

	assert(mmio_e1000[E1000_STATUS] == 0x80080783);
	e1000_init();	
	return 0;
}
	
