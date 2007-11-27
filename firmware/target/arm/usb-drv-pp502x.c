/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id:  $
 *
 * Driver for ARC USBOTG Device Controller
 *
 * Copyright (C) 2007 by Bj�rn Stenberg
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include "system.h"
#include "string.h"
#include "usb_ch9.h"
#include "usb_core.h"
//#define LOGF_ENABLE
#include "logf.h"

/* USB device mode registers (Little Endian) */

#define REG_ID               (*(volatile unsigned int *)(USB_BASE+0x000))
#define REG_HWGENERAL        (*(volatile unsigned int *)(USB_BASE+0x004))
#define REG_CAPLENGTH        (*(volatile unsigned char*)(USB_BASE+0x100))
#define REG_DCIVERSION       (*(volatile unsigned int *)(USB_BASE+0x120))
#define REG_DCCPARAMS        (*(volatile unsigned int *)(USB_BASE+0x124))
#define REG_USBCMD           (*(volatile unsigned int *)(USB_BASE+0x140))
#define REG_USBSTS           (*(volatile unsigned int *)(USB_BASE+0x144))
#define REG_USBINTR          (*(volatile unsigned int *)(USB_BASE+0x148))
#define REG_FRINDEX          (*(volatile unsigned int *)(USB_BASE+0x14c))
#define REG_DEVICEADDR       (*(volatile unsigned int *)(USB_BASE+0x154))
#define REG_ENDPOINTLISTADDR (*(volatile unsigned int *)(USB_BASE+0x158))
#define REG_BURSTSIZE        (*(volatile unsigned int *)(USB_BASE+0x160))
#define REG_CONFIGFLAG       (*(volatile unsigned int *)(USB_BASE+0x180))
#define REG_PORTSC1          (*(volatile unsigned int *)(USB_BASE+0x184))
#define REG_OTGSC            (*(volatile unsigned int *)(USB_BASE+0x1a4))
#define REG_USBMODE          (*(volatile unsigned int *)(USB_BASE+0x1a8))
#define REG_ENDPTSETUPSTAT   (*(volatile unsigned int *)(USB_BASE+0x1ac))
#define REG_ENDPTPRIME       (*(volatile unsigned int *)(USB_BASE+0x1b0))
#define REG_ENDPTFLUSH       (*(volatile unsigned int *)(USB_BASE+0x1b4))
#define REG_ENDPTSTATUS      (*(volatile unsigned int *)(USB_BASE+0x1b8))
#define REG_ENDPTCOMPLETE    (*(volatile unsigned int *)(USB_BASE+0x1bc))
#define REG_ENDPTCTRL0       (*(volatile unsigned int *)(USB_BASE+0x1c0))
#define REG_ENDPTCTRL1       (*(volatile unsigned int *)(USB_BASE+0x1c4))
#define REG_ENDPTCTRL2       (*(volatile unsigned int *)(USB_BASE+0x1c8))
#define REG_ENDPTCTRL(_x_)   (*(volatile unsigned int *)(USB_BASE+0x1c0+4*(_x_)))

/* Frame Index Register Bit Masks */
#define USB_FRINDEX_MASKS                      (0x3fff)

/* USB CMD  Register Bit Masks */
#define USBCMD_RUN                            (0x00000001)
#define USBCMD_CTRL_RESET                     (0x00000002)
#define USBCMD_PERIODIC_SCHEDULE_EN           (0x00000010)
#define USBCMD_ASYNC_SCHEDULE_EN              (0x00000020)
#define USBCMD_INT_AA_DOORBELL                (0x00000040)
#define USBCMD_ASP                            (0x00000300)
#define USBCMD_ASYNC_SCH_PARK_EN              (0x00000800)
#define USBCMD_SUTW                           (0x00002000)
#define USBCMD_ATDTW                          (0x00004000)
#define USBCMD_ITC                            (0x00FF0000)

/* bit 15,3,2 are frame list size */
#define USBCMD_FRAME_SIZE_1024                (0x00000000)
#define USBCMD_FRAME_SIZE_512                 (0x00000004)
#define USBCMD_FRAME_SIZE_256                 (0x00000008)
#define USBCMD_FRAME_SIZE_128                 (0x0000000C)
#define USBCMD_FRAME_SIZE_64                  (0x00008000)
#define USBCMD_FRAME_SIZE_32                  (0x00008004)
#define USBCMD_FRAME_SIZE_16                  (0x00008008)
#define USBCMD_FRAME_SIZE_8                   (0x0000800C)

/* bit 9-8 are async schedule park mode count */
#define USBCMD_ASP_00                         (0x00000000)
#define USBCMD_ASP_01                         (0x00000100)
#define USBCMD_ASP_10                         (0x00000200)
#define USBCMD_ASP_11                         (0x00000300)
#define USBCMD_ASP_BIT_POS                    (8)

/* bit 23-16 are interrupt threshold control */
#define USBCMD_ITC_NO_THRESHOLD               (0x00000000)
#define USBCMD_ITC_1_MICRO_FRM                (0x00010000)
#define USBCMD_ITC_2_MICRO_FRM                (0x00020000)
#define USBCMD_ITC_4_MICRO_FRM                (0x00040000)
#define USBCMD_ITC_8_MICRO_FRM                (0x00080000)
#define USBCMD_ITC_16_MICRO_FRM               (0x00100000)
#define USBCMD_ITC_32_MICRO_FRM               (0x00200000)
#define USBCMD_ITC_64_MICRO_FRM               (0x00400000)
#define USBCMD_ITC_BIT_POS                    (16)

/* USB STS Register Bit Masks */
#define USBSTS_INT                            (0x00000001)
#define USBSTS_ERR                            (0x00000002)
#define USBSTS_PORT_CHANGE                    (0x00000004)
#define USBSTS_FRM_LST_ROLL                   (0x00000008)
#define USBSTS_SYS_ERR                        (0x00000010) /* not used */
#define USBSTS_IAA                            (0x00000020)
#define USBSTS_RESET                          (0x00000040)
#define USBSTS_SOF                            (0x00000080)
#define USBSTS_SUSPEND                        (0x00000100)
#define USBSTS_HC_HALTED                      (0x00001000)
#define USBSTS_RCL                            (0x00002000)
#define USBSTS_PERIODIC_SCHEDULE              (0x00004000)
#define USBSTS_ASYNC_SCHEDULE                 (0x00008000)

/* USB INTR Register Bit Masks */
#define USBINTR_INT_EN                        (0x00000001)
#define USBINTR_ERR_INT_EN                    (0x00000002)
#define USBINTR_PTC_DETECT_EN                 (0x00000004)
#define USBINTR_FRM_LST_ROLL_EN               (0x00000008)
#define USBINTR_SYS_ERR_EN                    (0x00000010)
#define USBINTR_ASYN_ADV_EN                   (0x00000020)
#define USBINTR_RESET_EN                      (0x00000040)
#define USBINTR_SOF_EN                        (0x00000080)
#define USBINTR_DEVICE_SUSPEND                (0x00000100)

/* Device Address bit masks */
#define USBDEVICEADDRESS_MASK                 (0xFE000000)
#define USBDEVICEADDRESS_BIT_POS              (25)

/* endpoint list address bit masks */
#define USB_EP_LIST_ADDRESS_MASK               (0xfffff800)

/* PORTSCX  Register Bit Masks */
#define PORTSCX_CURRENT_CONNECT_STATUS         (0x00000001)
#define PORTSCX_CONNECT_STATUS_CHANGE          (0x00000002)
#define PORTSCX_PORT_ENABLE                    (0x00000004)
#define PORTSCX_PORT_EN_DIS_CHANGE             (0x00000008)
#define PORTSCX_OVER_CURRENT_ACT               (0x00000010)
#define PORTSCX_OVER_CURRENT_CHG               (0x00000020)
#define PORTSCX_PORT_FORCE_RESUME              (0x00000040)
#define PORTSCX_PORT_SUSPEND                   (0x00000080)
#define PORTSCX_PORT_RESET                     (0x00000100)
#define PORTSCX_LINE_STATUS_BITS               (0x00000C00)
#define PORTSCX_PORT_POWER                     (0x00001000)
#define PORTSCX_PORT_INDICTOR_CTRL             (0x0000C000)
#define PORTSCX_PORT_TEST_CTRL                 (0x000F0000)
#define PORTSCX_WAKE_ON_CONNECT_EN             (0x00100000)
#define PORTSCX_WAKE_ON_CONNECT_DIS            (0x00200000)
#define PORTSCX_WAKE_ON_OVER_CURRENT           (0x00400000)
#define PORTSCX_PHY_LOW_POWER_SPD              (0x00800000)
#define PORTSCX_PORT_FORCE_FULL_SPEED          (0x01000000)
#define PORTSCX_PORT_SPEED_MASK                (0x0C000000)
#define PORTSCX_PORT_WIDTH                     (0x10000000)
#define PORTSCX_PHY_TYPE_SEL                   (0xC0000000)

/* bit 11-10 are line status */
#define PORTSCX_LINE_STATUS_SE0                (0x00000000)
#define PORTSCX_LINE_STATUS_JSTATE             (0x00000400)
#define PORTSCX_LINE_STATUS_KSTATE             (0x00000800)
#define PORTSCX_LINE_STATUS_UNDEF              (0x00000C00)
#define PORTSCX_LINE_STATUS_BIT_POS            (10)

/* bit 15-14 are port indicator control */
#define PORTSCX_PIC_OFF                        (0x00000000)
#define PORTSCX_PIC_AMBER                      (0x00004000)
#define PORTSCX_PIC_GREEN                      (0x00008000)
#define PORTSCX_PIC_UNDEF                      (0x0000C000)
#define PORTSCX_PIC_BIT_POS                    (14)

/* bit 19-16 are port test control */
#define PORTSCX_PTC_DISABLE                    (0x00000000)
#define PORTSCX_PTC_JSTATE                     (0x00010000)
#define PORTSCX_PTC_KSTATE                     (0x00020000)
#define PORTSCX_PTC_SEQNAK                     (0x00030000)
#define PORTSCX_PTC_PACKET                     (0x00040000)
#define PORTSCX_PTC_FORCE_EN                   (0x00050000)
#define PORTSCX_PTC_BIT_POS                    (16)

/* bit 27-26 are port speed */
#define PORTSCX_PORT_SPEED_FULL                (0x00000000)
#define PORTSCX_PORT_SPEED_LOW                 (0x04000000)
#define PORTSCX_PORT_SPEED_HIGH                (0x08000000)
#define PORTSCX_PORT_SPEED_UNDEF               (0x0C000000)
#define PORTSCX_SPEED_BIT_POS                  (26)

/* bit 28 is parallel transceiver width for UTMI interface */
#define PORTSCX_PTW                            (0x10000000)
#define PORTSCX_PTW_8BIT                       (0x00000000)
#define PORTSCX_PTW_16BIT                      (0x10000000)

/* bit 31-30 are port transceiver select */
#define PORTSCX_PTS_UTMI                       (0x00000000)
#define PORTSCX_PTS_ULPI                       (0x80000000)
#define PORTSCX_PTS_FSLS                       (0xC0000000)
#define PORTSCX_PTS_BIT_POS                    (30)

/* USB MODE Register Bit Masks */
#define USBMODE_CTRL_MODE_IDLE                (0x00000000)
#define USBMODE_CTRL_MODE_DEVICE              (0x00000002)
#define USBMODE_CTRL_MODE_HOST                (0x00000003)
#define USBMODE_CTRL_MODE_RSV                 (0x00000001)
#define USBMODE_SETUP_LOCK_OFF                (0x00000008)
#define USBMODE_STREAM_DISABLE                (0x00000010)

/* Endpoint Flush Register */
#define EPFLUSH_TX_OFFSET                      (0x00010000)
#define EPFLUSH_RX_OFFSET                      (0x00000000)

/* Endpoint Setup Status bit masks */
#define EPSETUP_STATUS_MASK                   (0x0000003F)
#define EPSETUP_STATUS_EP0                    (0x00000001)

/* ENDPOINTCTRLx  Register Bit Masks */
#define EPCTRL_TX_ENABLE                       (0x00800000)
#define EPCTRL_TX_DATA_TOGGLE_RST              (0x00400000)    /* Not EP0 */
#define EPCTRL_TX_DATA_TOGGLE_INH              (0x00200000)    /* Not EP0 */
#define EPCTRL_TX_TYPE                         (0x000C0000)
#define EPCTRL_TX_DATA_SOURCE                  (0x00020000)    /* Not EP0 */
#define EPCTRL_TX_EP_STALL                     (0x00010000)
#define EPCTRL_RX_ENABLE                       (0x00000080)
#define EPCTRL_RX_DATA_TOGGLE_RST              (0x00000040)    /* Not EP0 */
#define EPCTRL_RX_DATA_TOGGLE_INH              (0x00000020)    /* Not EP0 */
#define EPCTRL_RX_TYPE                         (0x0000000C)
#define EPCTRL_RX_DATA_SINK                    (0x00000002)    /* Not EP0 */
#define EPCTRL_RX_EP_STALL                     (0x00000001)

/* bit 19-18 and 3-2 are endpoint type */
#define EPCTRL_EP_TYPE_CONTROL                 (0)
#define EPCTRL_EP_TYPE_ISO                     (1)
#define EPCTRL_EP_TYPE_BULK                    (2)
#define EPCTRL_EP_TYPE_INTERRUPT               (3)
#define EPCTRL_TX_EP_TYPE_SHIFT                (18)
#define EPCTRL_RX_EP_TYPE_SHIFT                (2)

/* pri_ctrl Register Bit Masks */
#define PRI_CTRL_PRI_LVL1                      (0x0000000C)
#define PRI_CTRL_PRI_LVL0                      (0x00000003)

/* si_ctrl Register Bit Masks */
#define SI_CTRL_ERR_DISABLE                    (0x00000010)
#define SI_CTRL_IDRC_DISABLE                   (0x00000008)
#define SI_CTRL_RD_SAFE_EN                     (0x00000004)
#define SI_CTRL_RD_PREFETCH_DISABLE            (0x00000002)
#define SI_CTRL_RD_PREFEFETCH_VAL              (0x00000001)

/* control Register Bit Masks */
#define USB_CTRL_IOENB                         (0x00000004)
#define USB_CTRL_ULPI_INT0EN                   (0x00000001)

/* OTGSC Register Bit Masks */
#define OTGSC_B_SESSION_VALID                  (0x00000800)

#define QH_MULT_POS                            (30)
#define QH_ZLT_SEL                             (0x20000000)
#define QH_MAX_PKT_LEN_POS                     (16)
#define QH_IOS                                 (0x00008000)
#define QH_NEXT_TERMINATE                      (0x00000001)
#define QH_IOC                                 (0x00008000)
#define QH_MULTO                               (0x00000C00)
#define QH_STATUS_HALT	                       (0x00000040)
#define QH_STATUS_ACTIVE                       (0x00000080)
#define EP_QUEUE_CURRENT_OFFSET_MASK         (0x00000FFF)
#define EP_QUEUE_HEAD_NEXT_POINTER_MASK      (0xFFFFFFE0)
#define EP_QUEUE_FRINDEX_MASK                (0x000007FF)
#define EP_MAX_LENGTH_TRANSFER               (0x4000)

#define DTD_NEXT_TERMINATE                   (0x00000001)
#define DTD_IOC                              (0x00008000)
#define DTD_STATUS_ACTIVE                    (0x00000080)
#define DTD_STATUS_HALTED                    (0x00000040)
#define DTD_STATUS_DATA_BUFF_ERR             (0x00000020)
#define DTD_STATUS_TRANSACTION_ERR           (0x00000008)
#define DTD_RESERVED_FIELDS                  (0x80007300)
#define DTD_ADDR_MASK                        (0xFFFFFFE0)
#define DTD_PACKET_SIZE                      (0x7FFF0000)
#define DTD_LENGTH_BIT_POS                   (16)
#define DTD_ERROR_MASK                       (DTD_STATUS_HALTED | \
                                               DTD_STATUS_DATA_BUFF_ERR | \
                                               DTD_STATUS_TRANSACTION_ERR)

/*-------------------------------------------------------------------------*/

/* manual: 32.13.2 Endpoint Transfer Descriptor (dTD) */
struct transfer_descriptor {
    unsigned int next_td_ptr;           /* Next TD pointer(31-5), T(0) set
                                           indicate invalid */
    unsigned int size_ioc_sts;          /* Total bytes (30-16), IOC (15),
                                           MultO(11-10), STS (7-0)  */
    unsigned int buff_ptr0;             /* Buffer pointer Page 0 */
    unsigned int buff_ptr1;             /* Buffer pointer Page 1 */
    unsigned int buff_ptr2;             /* Buffer pointer Page 2 */
    unsigned int buff_ptr3;             /* Buffer pointer Page 3 */
    unsigned int buff_ptr4;             /* Buffer pointer Page 4 */
    unsigned int reserved;
} __attribute__ ((packed));

static struct transfer_descriptor _td_array[NUM_ENDPOINTS*2] __attribute((aligned (32)));
static struct transfer_descriptor* td_array;

/* manual: 32.13.1 Endpoint Queue Head (dQH) */
struct queue_head {
    unsigned int max_pkt_length;    /* Mult(31-30) , Zlt(29) , Max Pkt len
                                       and IOS(15) */
    unsigned int curr_dtd_ptr;      /* Current dTD Pointer(31-5) */
    struct transfer_descriptor dtd; /* dTD overlay */
    unsigned int setup_buffer[2];   /* Setup data 8 bytes */
    unsigned int reserved[4];
} __attribute__((packed));

static struct queue_head _qh_array[NUM_ENDPOINTS*2] __attribute((aligned (2048)));
static struct queue_head* qh_array;


static const unsigned int pipe2mask[NUM_ENDPOINTS*2] = {
    0x01, 0x010000,
    0x02, 0x020000,
    0x04, 0x040000,
};

/*-------------------------------------------------------------------------*/
static void transfer_completed(void);
static void prime_transfer(int endpoint, void* ptr, int len, bool send);
static void bus_reset(void);
static void init_queue_heads(void);
static void init_endpoints(void);
/*-------------------------------------------------------------------------*/

bool usb_drv_powered(void)
{
    return (REG_OTGSC & OTGSC_B_SESSION_VALID) ? true : false;
}

/* manual: 32.14.1 Device Controller Initialization */
void usb_drv_init(void)
{
    REG_USBCMD &= ~USBCMD_RUN;
    udelay(50000);
    REG_USBCMD |= USBCMD_CTRL_RESET;
    while (REG_USBCMD & USBCMD_CTRL_RESET);

    REG_USBMODE = USBMODE_CTRL_MODE_DEVICE;

    td_array = (struct transfer_descriptor*)UNCACHED_ADDR(&_td_array);
    qh_array = (struct queue_head*)UNCACHED_ADDR(&_qh_array);
    init_queue_heads();
    memset(td_array, 0, sizeof _td_array);

    REG_ENDPOINTLISTADDR = (unsigned int)qh_array;
    REG_DEVICEADDR = 0;

    /* enable USB interrupts */
    REG_USBINTR =
        USBINTR_INT_EN |
        USBINTR_ERR_INT_EN |
        USBINTR_PTC_DETECT_EN |
        USBINTR_RESET_EN |
        USBINTR_SYS_ERR_EN;

    /* enable USB IRQ in CPU */
    CPU_INT_EN |= USB_MASK;

    /* go go go */
    REG_USBCMD |= USBCMD_RUN;

    logf("usb_drv_init() finished");
    logf("usb id %x", REG_ID);
    logf("usb dciversion %x", REG_DCIVERSION);
    logf("usb dccparams %x", REG_DCCPARAMS);

    /* now a bus reset will occur. see bus_reset() */
}

void usb_drv_exit(void)
{
    /* disable interrupts */
    REG_USBINTR = 0;

    /* stop usb controller */
    REG_USBCMD &= ~USBCMD_RUN;
}

void usb_drv_int(void)
{
    unsigned int status = REG_USBSTS;

#if 0
    if (status & USBSTS_INT) logf("int: usb ioc");
    if (status & USBSTS_ERR) logf("int: usb err");
    if (status & USBSTS_PORT_CHANGE) logf("int: portchange");
    if (status & USBSTS_RESET) logf("int: reset");
    if (status & USBSTS_SYS_ERR) logf("int: syserr");
#endif

    /* usb transaction interrupt */
    if (status & USBSTS_INT) {
        REG_USBSTS |= USBSTS_INT;

        /* a control packet? */
        if (REG_ENDPTSETUPSTAT & EPSETUP_STATUS_EP0) {
            /* copy setup data from packet */
            unsigned int tmp[2];
            tmp[0] = qh_array[0].setup_buffer[0];
            tmp[1] = qh_array[0].setup_buffer[1];

            /* acknowledge packet recieved */
            REG_ENDPTSETUPSTAT |= EPSETUP_STATUS_EP0;

            usb_core_control_request((struct usb_ctrlrequest*)tmp);
        }

        if (REG_ENDPTCOMPLETE)
            transfer_completed();
    }

    /* error interrupt */
    if (status & USBSTS_ERR) {
        REG_USBSTS |= USBSTS_ERR;
        logf("usb error int");
    }

    /* reset interrupt */
    if (status & USBSTS_RESET) {
        REG_USBSTS |= USBSTS_RESET;
        bus_reset();
        usb_core_bus_reset(); /* tell mom */
    }

    /* port change */
    if (status & USBSTS_PORT_CHANGE) {
        REG_USBSTS |= USBSTS_PORT_CHANGE;
    }
}

void usb_drv_stall(int endpoint, bool stall)
{
    logf("%sstall %d", stall?"":"un", endpoint);

    if (stall) {
        REG_ENDPTCTRL(endpoint) |= EPCTRL_RX_EP_STALL;
        REG_ENDPTCTRL(endpoint) |= EPCTRL_TX_EP_STALL;
    }
    else {
        REG_ENDPTCTRL(endpoint) &= ~EPCTRL_RX_EP_STALL;
        REG_ENDPTCTRL(endpoint) &= ~EPCTRL_TX_EP_STALL;
    }
}

void usb_drv_send(int endpoint, void* ptr, int length)
{
    prime_transfer(endpoint, ptr, length, true);
}

void usb_drv_recv(int endpoint, void* ptr, int length)
{
    //logf("usbrecv(%x, %d)", ptr, length);
    prime_transfer(endpoint, ptr, length, false);
}

void usb_drv_wait(int endpoint, bool send)
{
    int pipe = endpoint * 2 + (send ? 1 : 0);
    struct queue_head* qh = &qh_array[pipe];

    while (qh->dtd.size_ioc_sts & QH_STATUS_ACTIVE) {
        if (REG_USBSTS & USBSTS_RESET)
            break;
    }
}


void usb_drv_set_address(int address)
{
    REG_DEVICEADDR = address << USBDEVICEADDRESS_BIT_POS;
    init_endpoints();
}

void usb_drv_reset_endpoint(int endpoint, bool send)
{
    int pipe = endpoint * 2 + (send ? 1 : 0);
    unsigned int mask = pipe2mask[pipe];
    REG_ENDPTFLUSH = mask;
    while (REG_ENDPTFLUSH & mask);
}

/*-------------------------------------------------------------------------*/

/* manual: 32.14.5.2 */
static void prime_transfer(int endpoint, void* ptr, int len, bool send)
{
    int timeout;
    int pipe = endpoint * 2 + (send ? 1 : 0);
    unsigned int mask = pipe2mask[pipe];
    struct transfer_descriptor* td = &td_array[pipe];
    struct queue_head* qh = &qh_array[pipe];

    if (send && endpoint > EP_CONTROL) {
        logf("usb: sent %d bytes", len);
    }

    memset(td, 0, sizeof(struct transfer_descriptor));
    td->next_td_ptr = DTD_NEXT_TERMINATE;
    td->size_ioc_sts = (len << DTD_LENGTH_BIT_POS) |
        DTD_STATUS_ACTIVE | DTD_IOC;
    td->buff_ptr0 = (unsigned int)ptr;
    td->buff_ptr1 = (unsigned int)ptr + 0x1000;
    td->buff_ptr2 = (unsigned int)ptr + 0x2000;
    td->buff_ptr3 = (unsigned int)ptr + 0x3000;
    td->buff_ptr4 = (unsigned int)ptr + 0x4000;
    td->reserved = len;
    qh->dtd.next_td_ptr = (unsigned int)td;
    qh->dtd.size_ioc_sts &= ~(QH_STATUS_HALT | QH_STATUS_ACTIVE);

    REG_ENDPTPRIME |= mask;

    timeout = 10000;
    while ((REG_ENDPTPRIME & mask) && --timeout) {
        if (REG_USBSTS & USBSTS_RESET)
            break;
    }
    if (!timeout) {
        logf("prime timeout");
    }

    if (!(REG_ENDPTSTATUS & mask)) {
        logf("no prime! %d %d %x", endpoint, pipe, qh->dtd.size_ioc_sts & 0xff );
    }

    if (send) {
        /* wait for transfer to finish */
        timeout = 100000;
        while ((td->size_ioc_sts & DTD_STATUS_ACTIVE) && --timeout) {
            if (REG_ENDPTCOMPLETE & mask)
                REG_ENDPTCOMPLETE |= mask;

            if (REG_USBSTS & USBSTS_RESET)
                return;
        }
        if (!timeout) {
            logf("td never finished");
            return;
        }
    }
}

static void transfer_completed(void)
{
    int i;
    unsigned int mask = REG_ENDPTCOMPLETE;
    REG_ENDPTCOMPLETE |= mask;

    //logf("usb comp %x", mask);

    for (i=0; i<NUM_ENDPOINTS; i++) {
        int x;
        for (x=0; x<2; x++) {
            int pipe = i * 2 + x;
            if (mask & pipe2mask[pipe])
                usb_core_transfer_complete(i, x ? true : false);
                                           
            if ((mask & pipe2mask[pipe]) &&
                (td_array[pipe].size_ioc_sts & DTD_ERROR_MASK)) {
                logf("pipe %d err %x", pipe, td_array[pipe].size_ioc_sts & DTD_ERROR_MASK);
            }
        }
    }
}

/* manual: 32.14.2.1 Bus Reset */
static void bus_reset(void)
{
    int i;
    logf("usb bus_reset");

    REG_DEVICEADDR = 0;
    REG_ENDPTSETUPSTAT = REG_ENDPTSETUPSTAT;
    REG_ENDPTCOMPLETE  = REG_ENDPTCOMPLETE;

    for (i=0; i<100; i++) {
        if (!REG_ENDPTPRIME)
            break;

        if (REG_USBSTS & USBSTS_RESET) {
            logf("usb: double reset");
            return;
        }

        udelay(100);
    }
    if (REG_ENDPTPRIME) {
        logf("usb: short reset timeout");
    }

    REG_ENDPTFLUSH = ~0;
    //while (REG_ENDPTFLUSH);
    
    if (!(REG_PORTSC1 & PORTSCX_PORT_RESET)) {
        logf("usb: slow reset!");
    }
}

/* manual: 32.14.4.1 Queue Head Initialization */
static void init_queue_heads(void)
{
    memset(qh_array, 0, sizeof _qh_array);

    /*** control ***/
    qh_array[EP_CONTROL].max_pkt_length = 512 << QH_MAX_PKT_LEN_POS | QH_IOS;
    qh_array[EP_CONTROL].dtd.next_td_ptr = QH_NEXT_TERMINATE;
    qh_array[EP_CONTROL+1].max_pkt_length = 512 << QH_MAX_PKT_LEN_POS;
    qh_array[EP_CONTROL+1].dtd.next_td_ptr = QH_NEXT_TERMINATE;

    /*** bulk ***/
    qh_array[EP_RX*2].max_pkt_length = 512 << QH_MAX_PKT_LEN_POS;
    qh_array[EP_RX*2].dtd.next_td_ptr = QH_NEXT_TERMINATE;
    qh_array[EP_TX*2+1].max_pkt_length = 512 << QH_MAX_PKT_LEN_POS;
    qh_array[EP_TX*2+1].dtd.next_td_ptr = QH_NEXT_TERMINATE;
}

static void init_endpoints(void)
{
    /* bulk */
    REG_ENDPTCTRL(EP_RX) =
        EPCTRL_RX_DATA_TOGGLE_RST | EPCTRL_RX_ENABLE |
        (EPCTRL_EP_TYPE_BULK << EPCTRL_RX_EP_TYPE_SHIFT) |
        (EPCTRL_EP_TYPE_BULK << EPCTRL_TX_EP_TYPE_SHIFT);

    REG_ENDPTCTRL(EP_TX) =
        EPCTRL_TX_DATA_TOGGLE_RST | EPCTRL_TX_ENABLE |
        (EPCTRL_EP_TYPE_BULK << EPCTRL_RX_EP_TYPE_SHIFT) |
        (EPCTRL_EP_TYPE_BULK << EPCTRL_TX_EP_TYPE_SHIFT);
}