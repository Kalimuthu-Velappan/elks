#include "3c503.h"

struct dpeth *dep;
struct sendq de_sendq[3];

void el2init()
{
  int ix,irq;

  printk("In el2init()\n");
  /* Map the address PROM to lower I/O address range */
  outb(0x6,dep->de_baseport+0x406);

  /* Read station address from PROM */
  for(ix=0;ix<5;ix++) dep->de_macaddr[ix]=inb(dep->de_baseport+ix);

  /* Map the 8390 back to lower I/O address range */
  outb(0x2,dep->de_baseport+0x406);

  /* Enable memory, but turn off interrupts until we are ready */
  outb(0xc9,dep->de_baseport+0x405);

  /* On board RAM test.   Not really useful but ..... */
#ifdef ETH_MEMTEST
  IF (0 = 1) THEN
    ix = el2memtest(&HFFFF) + el2memtest(&HAAAA) + el2memtest(&H5555) + el2memtest(0)

    IF (ix <> 0) THEN
      PRINT "3c503: on board memory failure"
    END IF
  END IF
#endif

  dep->de_dataport = dep->de_baseport;
  dep->de_dp8390port = dep->de_baseport;
 
  dep->de_16bit = 0;

  /* Allocate one send buffer (1.5KB) per 8KB of on board memory. */
  dep->de_sendq_nr=dep->de_ramsize / 0x2000;
  dep->de_sendq_nr=(dep->de_sendq_nr<1)?1:(dep->de_sendq_nr>2?2:dep->de_sendq_nr);

  dep->de_sendq_nr = dep->de_sendq_nr;

  dep->de_startpage=(ix * 6) + 0x20;
  dep->de_stoppage = 0x40;

  outb(dep->de_startpage,dep->de_baseport+0x400);
  outb(dep->de_stoppage,dep->de_baseport+0x401);

  /* Point the vector pointer registers somewhere ?harmless?. */
  outb(0xff,dep->de_baseport+0x40b);  /* Point at the ROM restart location    */
  outb(0xff,dep->de_baseport+0x40c);  /* 0xFFFF:0000  (from original sources) */
  outb(0x00,dep->de_baseport+0x40d);  /*           - What for protected mode? */

  /* Set interrupt level for 3c503 */
  irq = dep->de_irq;
 
  if(irq==9) irq=2;

  if(irq<2 || irq>5) {
    printk("3c503: bad irq configuration\n");
    return;
  }

  outb(irq<<2,dep->de_baseport+0x408);
  outb(0x8,dep->de_baseport+0x402);
  outb(0x20,dep->de_baseport+0x409);
  outb(0x0,dep->de_baseport+0x40a);
  outb(0x49,dep->de_baseport+0x405);

  return;
}

int eth_init()
{
  int i;

  printk("In eth_init\n");
  dep->de_baseport = 0x250;
  dep->de_irq = 0x2;
  if (el2probe() == 0) return 0;
  el2init();

  printk("eth0: 3c503%s at %04x, with %dk RAM onboard, IRQ %d , at ",
         dep->de_16bit==1?" (16-bit)":"",dep->de_linmem,dep->de_ramsize/1024,
         dep->de_irq);

  for(i=0;i<5;i++)
    printk("%02x%c",dep->de_macaddr[i],i==5?'\n':':');

}

int el2probe()
{
  int iobase,membase,membaseaddr,ix;

  printk("In el2probe()\n");

  if(inb(dep->de_baseport+0x408)==0x408) {
    printk("ETH: Is this card really an ne2k?  Bombing out.\n");
    return -1;
  }

  printk("Read one byte\n");

  if((iobase = inb(dep->de_baseport+0x403))==0)
  if(iobase = 0) return 0;
  membase = inb(0x404);
  if(membase&0xf0==0) return 0;
  if(iobase&(iobase-1)) return 0;
  if(membase&(membase-1)) return 0;

  printk("Resetting board\n");
  /* Resets board */
  outb (0x6,iobase+0x406);
  printk("Getting addr\n");
/* outb (0x2,iobase+0x406); */

  for(ix=0;ix<dep->de_sendq_nr;ix++) de_sendq[ix].sq_sendpage=(ix*6) + 0x20;

/*  if(!(dep->de_macaddr[0]==0x2 && dep->de_macaddr[1]==0x60 &&
       dep->de_macaddr[2]==0x8c)) return 0; */

  printk("eth addr okay\n");
  /* Map the 8390 back to lower I/O address range */
  outb (0x2,iobase+0x406);

  /* Setup shared memory addressing for 3c503 */
  membaseaddr=((membase&0xc0)?0xd8000:0xc8000)+((membase&0xa0)?0x4000:0);

  if(dep->de_linmem==0)
    dep->de_linmem=membaseaddr;
  else
    if(dep->de_linmem!=membaseaddr) {
      printk("3c503: bad memory configuration.\n");
      return 0;
    }

  dep->de_linmem=dep->de_linmem - (dep->de_offsetpage = (0x20 * 256));
  dep->de_ramsize=0x20*256;

  return 1;
}

void el2stop()
{
  printk("3c503: stopping Etherlink");
  outb (0xc9,dep->de_baseport+0x405);
}
