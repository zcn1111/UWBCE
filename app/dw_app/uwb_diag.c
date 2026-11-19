#include "uwb_diag.h"

uwb_diag_t g_uwb_diag;

typedef struct
{
    uint32_t samples;
    float rssi_min;
    float rssi_max;
    float rssi_sum;
    uint64_t preamble_sum;
} uwb_diag_rx_quality_t;

static uwb_diag_rx_quality_t tag_rx_quality[MAX_AHCHOR_NUMBER];

static enum
{
    TAG_RUN_CONTINUOUS,
    TAG_RUN_START_PENDING,
    TAG_RUN_COUNTED,
    TAG_RUN_STOP_PENDING,
    TAG_RUN_PAUSED
} tag_run_mode;
static uint32_t tag_run_limit;

static void uwb_diag_reset(void)
{
    memset(&g_uwb_diag, 0, sizeof(g_uwb_diag));
    memset(tag_rx_quality, 0, sizeof(tag_rx_quality));
    g_uwb_diag.start_ms = HAL_GetTick();
    g_uwb_diag.active = 1;
}

bool uwb_diag_start(void)
{
    if (tag_run_limit != 0)
    {
        return false;
    }
    uwb_diag_reset();
    return true;
}

void uwb_diag_rx_fault(uwb_diag_rx_t *rx, uint32_t status)
{
    if (!g_uwb_diag.active)
    {
        return;
    }

    rx->fault++;
    rx->preamble_timeout += (status & SYS_STATUS_RXPTO) != 0;
    rx->frame_timeout += (status & SYS_STATUS_RXRFTO) != 0;
    rx->sfd_timeout += (status & SYS_STATUS_RXSFDTO) != 0;
    rx->phr_error += (status & SYS_STATUS_RXPHE) != 0;
    rx->crc_error += (status & SYS_STATUS_RXFCE) != 0;
    rx->sync_loss += (status & SYS_STATUS_RXRFSL) != 0;
    rx->filter_reject += (status & SYS_STATUS_AFFREJ) != 0;
    rx->lde_error += (status & SYS_STATUS_LDEERR) != 0;
    rx->other += (status & (SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)) == 0;
}

void uwb_diag_tag_rx_sample(uint8_t slot, const dwt_rxdiag_t *diag)
{
    /* RX power estimation divides by the preamble count and uses CIR power. */
    if ((diag->rxPreamCount == 0) || (diag->maxGrowthCIR == 0))
    {
        return;
    }

    uwb_diag_rx_quality_t *quality = &tag_rx_quality[slot];
    float rssi = diag->rxPower;
    if ((quality->samples == 0) || (rssi < quality->rssi_min))
    {
        quality->rssi_min = rssi;
    }
    if ((quality->samples == 0) || (rssi > quality->rssi_max))
    {
        quality->rssi_max = rssi;
    }
    quality->samples++;
    quality->rssi_sum += rssi;
    quality->preamble_sum += diag->rxPreamCount;
}

static void uwb_diag_print_rx(const char *stage, const uwb_diag_rx_t *rx)
{
    debug_printf("UWB_DIAG %s ok=%lu reject=%lu late=%lu fault=%lu "
                 "pto=%lu fto=%lu sfd=%lu phr=%lu crc=%lu sync=%lu filter=%lu lde=%lu other=%lu\r\n",
                 stage, (unsigned long)rx->ok, (unsigned long)rx->rejected,
                 (unsigned long)rx->late, (unsigned long)rx->fault,
                 (unsigned long)rx->preamble_timeout, (unsigned long)rx->frame_timeout,
                 (unsigned long)rx->sfd_timeout, (unsigned long)rx->phr_error,
                 (unsigned long)rx->crc_error, (unsigned long)rx->sync_loss,
                 (unsigned long)rx->filter_reject, (unsigned long)rx->lde_error,
                 (unsigned long)rx->other);
}

static void uwb_diag_print_tx(const char *stage, const uwb_diag_tx_t *tx)
{
    debug_printf("UWB_DIAG %s sent=%lu start_fail=%lu done_timeout=%lu\r\n",
                 stage, (unsigned long)tx->sent, (unsigned long)tx->start_fail,
                 (unsigned long)tx->completion_timeout);
}

static bool uwb_diag_report(void)
{
    if (!g_uwb_diag.active)
    {
        return false;
    }

    g_uwb_diag.active = 0;
    debug_printf("UWB_DIAG role=%s id=%u elapsed_ms=%lu\r\n",
                 instance_mode == TAG ? "tag" : "anchor", (unsigned int)dev_id,
                 (unsigned long)(HAL_GetTick() - g_uwb_diag.start_ms));
    if (instance_mode == TAG)
    {
        debug_printf("UWB_DIAG rounds=%lu resp_rounds=%lu no_resp=%lu unexpected_abort=%lu rx_irq_timeout=%lu\r\n",
                     (unsigned long)g_uwb_diag.tag.rounds, (unsigned long)g_uwb_diag.tag.resp_rounds,
                     (unsigned long)g_uwb_diag.tag.no_resp, (unsigned long)g_uwb_diag.tag.unexpected_frame_abort,
                     (unsigned long)g_uwb_diag.tag.rx_irq_timeout);
        uwb_diag_print_tx("poll_tx", &g_uwb_diag.tag.poll);
        for (unsigned int slot = 0; slot < MAX_AHCHOR_NUMBER; slot++)
        {
            char stage[16];
            snprintf(stage, sizeof(stage), "resp_rx_A%u", slot);
            uwb_diag_print_rx(stage, &g_uwb_diag.tag.resp[slot]);
            const uwb_diag_rx_quality_t *quality = &tag_rx_quality[slot];
            if (quality->samples != 0)
            {
                debug_printf("UWB_DIAG resp_quality_A%u samples=%lu "
                             "rssi_dbm_min=%.1f rssi_dbm_avg=%.1f rssi_dbm_max=%.1f rxpacc_avg=%.1f\r\n",
                             slot, (unsigned long)quality->samples, (double)quality->rssi_min,
                             (double)(quality->rssi_sum / quality->samples), (double)quality->rssi_max,
                             (double)quality->preamble_sum / quality->samples);
            }
        }
        uwb_diag_print_tx("final_tx", &g_uwb_diag.tag.final);
    }
    else
    {
        uwb_diag_print_rx("poll_rx", &g_uwb_diag.anchor.poll);
        uwb_diag_print_tx("resp_tx", &g_uwb_diag.anchor.resp);
        uwb_diag_print_rx("final_rx", &g_uwb_diag.anchor.final);
        debug_printf("UWB_DIAG peer_rx_late=%lu distance_updates=%lu\r\n",
                     (unsigned long)g_uwb_diag.anchor.peer_rx_late,
                     (unsigned long)g_uwb_diag.anchor.distance_updates);
    }
    return true;
}

bool uwb_diag_stop(void)
{
    if (tag_run_limit != 0)
    {
        return uwb_diag_tag_run(0);
    }
    return uwb_diag_report();
}

bool uwb_diag_tag_run(uint32_t rounds)
{
    if (rounds == 0)
    {
        if (tag_run_mode == TAG_RUN_START_PENDING)
        {
            tag_run_limit = 0;
        }
        tag_run_mode = TAG_RUN_STOP_PENDING;
        return true;
    }

    if ((tag_run_mode != TAG_RUN_CONTINUOUS) && (tag_run_mode != TAG_RUN_PAUSED))
    {
        return false;
    }
    tag_run_limit = rounds;
    tag_run_mode = TAG_RUN_START_PENDING;
    return true;
}

/* Only call this between rounds, including just before the first Poll. */
bool uwb_diag_tag_poll_allowed(bool starting_poll)
{
    if ((tag_run_mode == TAG_RUN_STOP_PENDING) ||
        ((tag_run_mode == TAG_RUN_COUNTED) && (g_uwb_diag.tag.rounds >= tag_run_limit)))
    {
        dwt_forcetrxoff();
        tag_run_mode = TAG_RUN_PAUSED;
        if (tag_run_limit != 0)
        {
            uwb_diag_report();
            debug_printf("UWB_DIAG run=%s target_rounds=%lu rounds=%lu\r\n",
                         g_uwb_diag.tag.rounds >= tag_run_limit ? "done" : "stopped",
                         (unsigned long)tag_run_limit, (unsigned long)g_uwb_diag.tag.rounds);
            tag_run_limit = 0;
        }
        else
        {
            debug_printf("UWB_DIAG run=paused\r\n");
        }
    }

    if (starting_poll && (tag_run_mode == TAG_RUN_START_PENDING))
    {
        uwb_diag_reset();
        tag_run_mode = TAG_RUN_COUNTED;
    }
    return tag_run_mode != TAG_RUN_PAUSED;
}

bool uwb_diag_tag_run_controlled(void)
{
    return tag_run_mode != TAG_RUN_CONTINUOUS;
}
