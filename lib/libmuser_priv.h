#include "dma.h"

typedef enum {
    IRQ_NONE = 0,
    IRQ_INTX,
    IRQ_MSI,
    IRQ_MSIX,
} irq_type_t;

typedef struct {
    irq_type_t  type;       /* irq type this device is using */
    int         err_efd;    /* eventfd for irq err */
    int         req_efd;    /* eventfd for irq req */
    uint32_t    max_ivs;    /* maximum number of ivs supported */
    int         efds[0];    /* XXX must be last */
} lm_irqs_t;

/*
 * Macro that ensures that a particular struct member is last. Doesn't work for
 * flexible array members.
 */
#define MUST_BE_LAST(s, m, t) \
    _Static_assert(sizeof(s) - offsetof(s, m) == sizeof(t), \
        #t " " #m " must be last member in " #s)

struct lm_ctx {
    void                    *pvt;
    dma_controller_t        *dma;
    int                     fd;
    bool                    extended;
    int (*reset)            (void *pvt);
    lm_log_lvl_t            log_lvl;
    lm_log_fn_t             *log;
    lm_pci_info_t           pci_info;
    lm_pci_config_space_t   *pci_config_space;
    struct caps             *caps;
    lm_irqs_t               irqs; /* XXX must be last */
};
MUST_BE_LAST(struct lm_ctx, irqs, lm_irqs_t);

int dev_attach(const char *);
