#include <kern/e1000.h>


// LAB 6: Your driver code here
struct e1000_tx_desc tx_desc_buf[TXRING_LEN] __attribute__ ((aligned (PGSIZE)));
struct packet packet_buf[TXRING_LEN] __attribute__ ((aligned (4096)));

struct e1000_rx_desc rx_desc_buf[RXRING_LEN] __attribute__ ((aligned (PGSIZE)));
struct packet rcv_packet_buf[RXRING_LEN] __attribute__ ((aligned (4096)));

static void 
e1000_init()
{
	int i;
	for (i=0; i < TXRING_LEN; i++)
	{
		tx_desc_buf[i].buffer_addr = PADDR(&packet_buf[i]);
		tx_desc_buf[i].upper.fields.status = E1000_TXD_STAT_DD;
		tx_desc_buf[i].lower.flags.cmd |= E1000_TXD_RS;			// set RS bit
		tx_desc_buf[i].lower.flags.cmd &= ~E1000_TXD_DEXT; // set legacy descriptor mode
	} 
	i = PADDR(&rcv_packet_buf[0]);
	cprintf ("%x\n", PADDR(&rcv_packet_buf[0]));
	cprintf ("%x\n", PADDR(&rcv_packet_buf[RXRING_LEN-1]));
	for (i = 0; i < RXRING_LEN; i++)
	{
		rx_desc_buf[i].buffer_addr = PADDR(&rcv_packet_buf[i]);
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
	mmio_e1000[E1000_TIPG] = 0x0C0200A;
	cprintf ("%x\n", mmio_e1000[E1000_TIPG]);
	cprintf ("%x\n", mmio_e1000[E1000_TCTL]);

	

	mmio_e1000[E1000_RAL] = 0x12005452;
	mmio_e1000[E1000_RAH] = 0x00005634 | E1000_RAH_AV;
	mmio_e1000[E1000_RDBAL] = PADDR(rx_desc_buf);
	mmio_e1000[E1000_RDBAH] = 0x0;
	mmio_e1000[E1000_RDLEN] = RXRING_LEN * sizeof(struct e1000_rx_desc);
	mmio_e1000[E1000_RDH] = 0x0;
	mmio_e1000[E1000_RDT] = RXRING_LEN;
	mmio_e1000[E1000_RCTL] = E1000_RCTL_EN |
           !E1000_RCTL_LPE |
           E1000_RCTL_LBM_NO |
           E1000_RCTL_RDMTS_HALF |
           E1000_RCTL_MO_0 |
           E1000_RCTL_BAM |
           E1000_RCTL_BSEX |
           E1000_RCTL_SZ_4096 |
           E1000_RCTL_SECRC;

	
	
}

int e1000_transmit(char *pkt, size_t length)
{
	if (length > PGSIZE)
		panic ("max size exceeded \n");
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
	
