#include "instance.h"

#if defined(L051_DEV)
#include "lowpower.h"
#endif

/* poll数据帧格式 */
static uint8_t tx_poll_msg[POLL_MSG_LEN] = {0x41, 0x88, 0, 0xCA, 0xDE, 0xFF, 0xFF, 0x00, 0x00, FUNC_CODE_POLL, 0x00, 0x00, 0x00, 0x00};
/* final数据帧格式 */
static uint8_t tx_final_msg[FIANL_MSG_LEN] = {0x41, 0x88, 0, 0xCA, 0xDE, 0xFF, 0xFF, 0x00, 0x00, FUNC_CODE_FINAL, 0X00};
/* 接收数据buffer */
static uint8_t rx_buffer[FRAME_LEN_MAX];
uint8_t g_CalFlag = 0;
static uint8_t resp_slot;
static uint8_t single_resp_id;                         //本轮只接收此基站，MAX_AHCHOR_NUMBER 表示扫描全部
static uint16_t resp_rx_advance = TAG_DIAG_RX_WINDOW_OFF;
static uint8_t resp_rx_retry;
static uint8_t resp_rx_retry_count;
/* DX_TIME 使用时间戳的 [39:8] 位，单位为 1/256 UUS；有符号差处理计时回卷。 */
static uint32_t resp_expected_hi;
static uint32_t resp_window_end_hi;
static uint32_t resp_rx_back_uus;
static int tagSleepCorrection_ms = 0;                   //标签时序校准，用于slot分配防冲突管理
uint32_t volatile next_period_time = 0;                //标签下次测距周期开始时间，用于防冲突管理
uint8_t Correction_flag = 0;                            //该标签已经被时序校准的标志位

/* TWR时间戳，用于计算飞行时间 */
static uint64_t poll_tx_ts;                     
static uint64_t resp_rx_ts[MAX_AHCHOR_NUMBER];
static uint64_t final_tx_ts;

/* 发送和接收数据中断标志 */
static volatile uint8_t rx_status = RX_WAIT;
static volatile uint8_t tx_status = TX_WAIT;
static volatile uint16_t rx_frame_len;
static volatile uint32_t rx_event_status;

static uint8_t resp_valid = 0x00;               //基站数据有效标志
uint32 diff_time;                               //标签增加发起测距随机时间，避免冲突


static uint8_t led_flag = 0;
static uint32_t led_time = 0;     

#define TAG_DIAG_PERIOD_MS 10000U
#define TAG_DIAG_DX_PER_UUS (UUS_TO_DWT_TIME >> 8)
#define TAG_DIAG_SFD_RETRY_MAX 2U
#define TAG_DIAG_SFD_REARM_UUS 64U

typedef struct
{
    uint32_t resp_ok;
    uint32_t preamble_timeout;
    uint32_t frame_timeout;
    uint32_t rx_error;
    uint32_t other_frame;
    uint32_t rx_late;
    uint32_t no_irq;
    uint32_t phr_error;
    uint32_t fcs_error;
    uint32_t rs_error;
    uint32_t sfd_timeout;
    uint32_t filter_reject;
    uint32_t lde_error;
    uint32_t sfd_before_resp;
    uint32_t sfd_after_resp;
    uint32_t sfd_rearm;
    uint32_t sfd_recovered;
    uint32_t sfd_rearm_late;
    int32_t sfd_offset_min;
    int32_t sfd_offset_max;
    uint32_t resp_time_count;
    int32_t resp_offset_min;
    int32_t resp_offset_max;
    int64_t resp_offset_sum;
} tag_diag_slot_t;

static struct
{
    uint32_t polls;
    uint32_t poll_tx_ok;
    uint32_t final_tx_ok;
    uint32_t final_tx_fail;
    uint32_t no_resp;
    uint32_t rx_abort;
    tag_diag_slot_t slots[MAX_AHCHOR_NUMBER];
} tag_diag;

static uint8_t tag_diag_enabled;
static uint8_t tag_diag_selected_id = MAX_AHCHOR_NUMBER;
static uint8_t tag_diag_selected_slot = MAX_AHCHOR_NUMBER;
static uint16_t tag_diag_selected_advance = TAG_DIAG_RX_WINDOW_OFF;
static uint8_t tag_diag_selected_retry;
static uint8_t tag_diag_collect;
static uint32_t tag_diag_started;

uint8_t tag_diag_is_enabled(void)
{
    return tag_diag_enabled;
}

uint8_t tag_diag_get_slot(void)
{
    return tag_diag_selected_slot;
}

uint16_t tag_diag_get_rx_advance(void)
{
    return tag_diag_selected_advance;
}

void tag_diag_set_rx_window(uint16_t advance_uus)
{
    /* 单槽和全槽诊断均可提前 RX，参数从下一轮 Poll 生效。 */
    tag_diag_selected_advance = tag_diag_enabled && advance_uus <= TAG_DIAG_RX_ADVANCE_MAX_UUS
        ? advance_uus : TAG_DIAG_RX_WINDOW_OFF;
}

uint8_t tag_diag_get_rx_retry(void)
{
    return tag_diag_selected_retry;
}

void tag_diag_set_rx_retry(uint8_t enabled)
{
    tag_diag_selected_retry = tag_diag_enabled && tag_diag_selected_advance != TAG_DIAG_RX_WINDOW_OFF && enabled;
}

void tag_diag_set_enabled(uint8_t enabled, uint8_t id, uint8_t slot)
{
    memset(&tag_diag, 0, sizeof(tag_diag));
    tag_diag_started = portGetTickCnt();
    tag_diag_collect = 0;  // 从下一次 Poll 开始采样，避免计入半轮
    tag_diag_enabled = enabled;
    uint8_t single_slot = enabled && id < MAX_AHCHOR_NUMBER && slot < MAX_AHCHOR_NUMBER;
    tag_diag_selected_id = single_slot ? id : MAX_AHCHOR_NUMBER;
    tag_diag_selected_slot = single_slot ? slot : MAX_AHCHOR_NUMBER;
    tag_diag_selected_advance = TAG_DIAG_RX_WINDOW_OFF;
    tag_diag_selected_retry = 0;
}

static void tag_diag_record_rx(uint8_t slot, uint8_t valid_resp)
{
    tag_diag_slot_t *stats = &tag_diag.slots[slot];

    if (rx_status == RX_TIMEOUT)
    {
        if (rx_event_status & SYS_STATUS_RXPTO)
            stats->preamble_timeout++;
        else
            stats->frame_timeout++;
    }
    else if (rx_status == RX_ERROR)
    {
        uint32_t status = rx_event_status;
        stats->rx_error++;
        stats->phr_error += (status & SYS_STATUS_RXPHE) != 0;
        stats->fcs_error += (status & SYS_STATUS_RXFCE) != 0;
        stats->rs_error += (status & SYS_STATUS_RXRFSL) != 0;
        stats->sfd_timeout += (status & SYS_STATUS_RXSFDTO) != 0;
        stats->filter_reject += (status & SYS_STATUS_AFFREJ) != 0;
        stats->lde_error += (status & SYS_STATUS_LDEERR) != 0;
    }
    else if (valid_resp && (single_resp_id < MAX_AHCHOR_NUMBER || rx_buffer[SENDER_SHORT_ADD_IDX] == slot))
    {
        stats->resp_ok++;
        if (resp_rx_retry_count != 0)
            stats->sfd_recovered++;
    }
    else
        stats->other_frame++;
}

/* 汇总只在 STA_IDLE 输出，避免串口发送占用当前测距轮的时隙。 */
static void tag_diag_report(void)
{
    char line[384];
    unsigned rx_mask = (tag_diag_selected_slot < MAX_AHCHOR_NUMBER)
        ? (1U << tag_diag_selected_slot) : ((1U << MAX_AHCHOR_NUMBER) - 1U);
    int resp_id = (tag_diag_selected_id < MAX_AHCHOR_NUMBER) ? (int)tag_diag_selected_id : -1;
    int rx_advance = tag_diag_selected_advance == TAG_DIAG_RX_WINDOW_OFF ? -1 : (int)tag_diag_selected_advance;
    snprintf(line, sizeof(line),
        "UWBdiag poll=%lu poll_tx=%lu final_tx=%lu final_fail=%lu no_resp=%lu rx_abort=%lu rx_mask=0x%02X resp_id=%d rx_adv=%d rx_retry=%u\r\n",
        (unsigned long)tag_diag.polls, (unsigned long)tag_diag.poll_tx_ok,
        (unsigned long)tag_diag.final_tx_ok, (unsigned long)tag_diag.final_tx_fail,
        (unsigned long)tag_diag.no_resp, (unsigned long)tag_diag.rx_abort, rx_mask, resp_id, rx_advance,
        (unsigned)tag_diag_selected_retry);
    HAL_UART_Transmit(&UART_PORT, (uint8_t *)line, (uint16_t)strlen(line), 50);

    for (uint8_t i = 0; i < MAX_AHCHOR_NUMBER; i++)
    {
        const tag_diag_slot_t *slot = &tag_diag.slots[i];
        snprintf(line, sizeof(line),
            "UWBdiag slot=%u resp=%lu pre_to=%lu frame_to=%lu rx_err=%lu other=%lu late=%lu no_irq=%lu"
            " phr=%lu fcs=%lu rs=%lu sfd=%lu filter=%lu lde=%lu\r\n",
            (unsigned)i, (unsigned long)slot->resp_ok, (unsigned long)slot->preamble_timeout,
            (unsigned long)slot->frame_timeout, (unsigned long)slot->rx_error,
            (unsigned long)slot->other_frame, (unsigned long)slot->rx_late,
            (unsigned long)slot->no_irq, (unsigned long)slot->phr_error,
            (unsigned long)slot->fcs_error, (unsigned long)slot->rs_error,
            (unsigned long)slot->sfd_timeout, (unsigned long)slot->filter_reject,
            (unsigned long)slot->lde_error);
        HAL_UART_Transmit(&UART_PORT, (uint8_t *)line, (uint16_t)strlen(line), 50);
        if (tag_diag_selected_advance != TAG_DIAG_RX_WINDOW_OFF)
        {
            int32_t resp_average = slot->resp_time_count
                ? (int32_t)(slot->resp_offset_sum / slot->resp_time_count / TAG_DIAG_DX_PER_UUS) : 0;
            snprintf(line, sizeof(line),
                "UWBdiagRX slot=%u n=%lu sfd_pre=%lu sfd_post=%lu rearm=%lu recovered=%lu rearm_late=%lu"
                " sfd_off_uus=%ld/%ld resp_off_uus=%ld/%ld/%ld\r\n",
                (unsigned)i, (unsigned long)slot->resp_time_count,
                (unsigned long)slot->sfd_before_resp, (unsigned long)slot->sfd_after_resp,
                (unsigned long)slot->sfd_rearm, (unsigned long)slot->sfd_recovered,
                (unsigned long)slot->sfd_rearm_late,
                (long)(slot->sfd_offset_min / (int32_t)TAG_DIAG_DX_PER_UUS),
                (long)(slot->sfd_offset_max / (int32_t)TAG_DIAG_DX_PER_UUS),
                (long)(slot->resp_offset_min / (int32_t)TAG_DIAG_DX_PER_UUS), (long)resp_average,
                (long)(slot->resp_offset_max / (int32_t)TAG_DIAG_DX_PER_UUS));
            HAL_UART_Transmit(&UART_PORT, (uint8_t *)line, (uint16_t)strlen(line), 50);
        }
    }

    memset(&tag_diag, 0, sizeof(tag_diag));
    tag_diag_started = portGetTickCnt();
}

static int tag_start_resp_rx(uint8_t slot)
{
    uint32_t first_resp_delay = FIRST_RESP_SEND_6P8M;
    resp_rx_back_uus = ANC_RESP_SEND_BACK_6P8M;
    if (inst_dataRate == DWT_BR_110K)
    {
        first_resp_delay = FIRST_RESP_SEND_110K;
        resp_rx_back_uus = ANC_RESP_SEND_BACK_110K;
    }
    else if (inst_dataRate == DWT_BR_850K)
    {
        first_resp_delay = FIRST_RESP_SEND_850K;
        resp_rx_back_uus = ANC_RESP_SEND_BACK_850K;
    }

    uint32_t normal_delay = first_resp_delay + slot * inst_data_interval;
    resp_expected_hi = (uint32_t)((poll_tx_ts + (normal_delay + resp_rx_back_uus) * (uint64_t)UUS_TO_DWT_TIME) >> 8);
    resp_window_end_hi = (uint32_t)((poll_tx_ts + (normal_delay + inst_resp_rx_timeout) * (uint64_t)UUS_TO_DWT_TIME) >> 8);
    resp_rx_retry_count = 0;
    uint32_t advance = resp_rx_advance == TAG_DIAG_RX_WINDOW_OFF ? 0U : resp_rx_advance;
    uint64_t delay = (normal_delay - advance) * (uint64_t)UUS_TO_DWT_TIME;
    dwt_setdelayedtrxtime((uint32_t)((poll_tx_ts + delay) >> 8));
    rx_status = RX_WAIT;
    int ret = dwt_rxenable(DWT_START_RX_DELAYED | DWT_IDLE_ON_DLY_ERR);
    if (tag_diag_collect && ret == DWT_ERROR)
        tag_diag.slots[slot].rx_late++;
    return ret;
}

static void tag_diag_record_resp_time(uint8_t slot, uint64_t timestamp)
{
    if (!tag_diag_collect || resp_rx_advance == TAG_DIAG_RX_WINDOW_OFF)
        return;
    tag_diag_slot_t *stats = &tag_diag.slots[slot];
    int32_t offset = (int32_t)((uint32_t)(timestamp >> 8) - resp_expected_hi);
    if (stats->resp_time_count == 0 || offset < stats->resp_offset_min)
        stats->resp_offset_min = offset;
    if (stats->resp_time_count == 0 || offset > stats->resp_offset_max)
        stats->resp_offset_max = offset;
    stats->resp_offset_sum += offset;
    stats->resp_time_count++;
}

/* 驱动已在 RX 错误中断中关闭并复位接收机。这里只恢复足够早的纯 SFD 超时，
 * 最多两次；重新预约 RX 起点并按原绝对截止时刻计算超时，防止占用下一槽。 */
static uint8_t tag_try_sfd_retry(uint8_t slot)
{
    uint32_t status = rx_event_status;
    if (!tag_diag_collect || resp_rx_advance == TAG_DIAG_RX_WINDOW_OFF || rx_status != RX_ERROR
        || (status & (SYS_STATUS_ALL_RX_ERR | SYS_STATUS_ALL_RX_TO)) != SYS_STATUS_RXSFDTO)
        return 0;

    tag_diag_slot_t *stats = &tag_diag.slots[slot];
    uint32_t now_hi = dwt_readsystimestamphi32();
    /* 这是主循环处理错误时的时刻，包含中断处理延迟，不是硬件错误时间戳。 */
    int32_t offset = (int32_t)(now_hi - resp_expected_hi);
    if (stats->sfd_before_resp + stats->sfd_after_resp == 0 || offset < stats->sfd_offset_min)
        stats->sfd_offset_min = offset;
    if (stats->sfd_before_resp + stats->sfd_after_resp == 0 || offset > stats->sfd_offset_max)
        stats->sfd_offset_max = offset;
    if (offset < 0)
        stats->sfd_before_resp++;
    else
        stats->sfd_after_resp++;

    if (!resp_rx_retry || resp_rx_retry_count >= TAG_DIAG_SFD_RETRY_MAX)
        return 0;
    uint32_t retry_hi = (now_hi + TAG_DIAG_SFD_REARM_UUS * TAG_DIAG_DX_PER_UUS) & ~1UL;
    int32_t remaining = (int32_t)(resp_window_end_hi - retry_hi);
    /* 恢复后的 RX 起点不晚于原命令使用的正常起点，保留前导码捕获余量。 */
    if ((int32_t)(resp_expected_hi - retry_hi) < (int32_t)(resp_rx_back_uus * TAG_DIAG_DX_PER_UUS)
        || remaining < (int32_t)TAG_DIAG_DX_PER_UUS)
        return 0;

    dwt_setrxtimeout((uint16_t)((uint32_t)remaining / TAG_DIAG_DX_PER_UUS));
    dwt_setdelayedtrxtime(retry_hi);
    rx_status = RX_WAIT;
    rx_frame_len = 0;
    rx_event_status = 0;
    if (dwt_rxenable(DWT_START_RX_DELAYED | DWT_IDLE_ON_DLY_ERR) == DWT_SUCCESS)
    {
        resp_rx_retry_count++;
        stats->sfd_rearm++;
        return 1;
    }
    stats->sfd_rearm_late++;
    rx_event_status = status;
    rx_status = RX_ERROR;
    return 0;
}

void tag_app(void)
{
    switch (state)
    {
        case STA_SEND_POLL:  //打包和发送poll消息
        {
            if(inst_slot_number > 1)
            {
                diff_time += (tag_id + 1); //产生随机时间避免一直冲突     
                if(diff_time > inst_one_slot_time * inst_slot_number / 2)
                {
                    diff_time = 0;
                }
            }
            else
            {
                diff_time = 0;
            }
            
#if (DEEP_SLEEP == 1)
            no_move_to_sleep(POLL_TO_SLEEP);
#endif            
            tag_diag_collect = tag_diag_enabled;
            if (tag_diag_collect)
                tag_diag.polls++;
            tagSleepCorrection_ms = 0;
            range_nb++;
            /* poll数据打包 */
            tx_poll_msg[SEQ_NB_IDX] = frame_seq_nb++;  
            tx_poll_msg[PANID_IDX] = (uint8_t)PAN_ID; 
            tx_poll_msg[PANID_IDX + 1] = (uint8_t)(PAN_ID>>8); 
            tx_poll_msg[RANGE_NB_IDX] = range_nb;  
            tx_poll_msg[SENDER_SHORT_ADD_IDX] = tag_id;
            tx_poll_msg[FUNC_CODE_IDX] = FUNC_CODE_POLL;
            tx_poll_msg[POLL_MSG_SOS_IDX] = sos;
            if(alarm > 0)
                alarm = 1;
            tx_poll_msg[POLL_MSG_ALARM_STA_IDX] = alarm;
#if defined(ANCRANGE)  
            if(ancrange_flag > 0)
            {
                battery = 0xff;
            }
#endif
            tx_poll_msg[POLL_MSG_BATTERY_IDX] = battery;

            for(int i = 0; i < 10; i++)  //打包用户字节
            {
                tx_poll_msg[POLL_MSG_USER_IDX + i] = user_data[i];
            }

            range_time = portGetTickCnt();                           //获取测距时间，用于dw_main.c中串口打包
            dwt_writetxdata(POLL_MSG_LEN + FCS_LEN, tx_poll_msg, 0); //数据写入DW3000数据缓冲区
            dwt_writetxfctrl(POLL_MSG_LEN + FCS_LEN, 0, 1);          //配置TX帧控制寄存器
            tx_status = TX_WAIT;                                     //发送状态标志，在中断回调函数变更
            int ret = dwt_starttx(DWT_START_TX_IMMEDIATE);           //立即发送POLL消息
			
			/* 调用发送失败 */
            if(ret == DWT_ERROR)
            {
                next_period_time = range_time + inst_one_slot_time * inst_slot_number + ((Correction_flag == 1)?0:(diff_time));  //如该标签未被时序校准，初次上电，增加随机时间，避免一直冲突      
                range_status = RANGE_ERROR;
                state = STA_IDLE;
                break;
            }
//            while(tx_status == TX_WAIT);                        //等待发送成功，tx_status在发送成功中断内变更状态
			uint32_t tx_wait=uwTick;
			uint8_t tx_err=0;
			while (tx_status == TX_WAIT)
			{
				if (JudgeTick(tx_wait, 10) == true)
				{
					tx_err = 1;
					break;
				}
			}
			if (tx_err)
			{
				dwt_forcetrxoff();
				tx_status = TX_WAIT;
                next_period_time = range_time + inst_one_slot_time * inst_slot_number + ((Correction_flag == 1)?0:(diff_time));  //如该标签未被时序校准，初次上电，增加随机时间，避免一直冲突      
                range_status = RANGE_ERROR;
				state = STA_IDLE;
                break;
			}
            tx_status = TX_WAIT;                                //清标志
            if (tag_diag_collect)
                tag_diag.poll_tx_ok++;
            poll_tx_ts = get_tx_timestamp_u64();                //取得poll_tx时间戳
            single_resp_id = tag_diag_selected_id;             //整轮沿用 Poll 发出后的基站和槽选择
            resp_slot = (single_resp_id < MAX_AHCHOR_NUMBER) ? tag_diag_selected_slot : 0;
            resp_rx_advance = tag_diag_selected_advance;        //全槽扫描也沿用本轮提前量
            resp_rx_retry = tag_diag_selected_retry;
            resp_valid = 0;                                     //resp有效校验初始化
            memset(rx_buffer, 0, sizeof(rx_buffer));
            //发送POLL之后，延时开启接收等待resp
            uint16_t advance = resp_rx_advance == TAG_DIAG_RX_WINDOW_OFF ? 0U : resp_rx_advance;
            dwt_setrxtimeout(inst_resp_rx_timeout + advance);
            /* 窗口对照的 0 和提前量均关闭前导码超时，只改变 RX 起点。
             * 帧超时同步延长，使结束时刻保持不变。旧命令保持原超时。 */
            dwt_setpreambledetecttimeout(resp_rx_advance == TAG_DIAG_RX_WINDOW_OFF ? PRE_TIMEOUT : 0);

            if (tag_start_resp_rx(resp_slot) == DWT_ERROR)
            {
                next_period_time = range_time + inst_one_slot_time * inst_slot_number + ((Correction_flag == 1)?0:(diff_time));  //如该标签未被时序校准，初次上电，增加随机时间，避免一直冲突      
                range_status = RANGE_ERROR;
				state = STA_IDLE;
                break;
            }
            alarm = 0;
            for(int i = 0; i < 10; i++)  //清除用户字节
            {
                user_data[i] = 0;
            }
#if defined(ANCRANGE)        
            if(ancrange_flag == 2)
            {
                ancrange_count--;
            }
#endif
            state = STA_WAIT_RESP;
        }
        break;

        case STA_WAIT_RESP: //等待resp数据接收
        {
            if (rx_status != RX_WAIT)
            {
                state = STA_RECV_RESP;
            }
            else if ((uint32_t)(portGetTickCnt() - range_time) >= 50U)
            {
                if (!tag_diag_enabled)
                {
                    HAL_NVIC_SystemReset();
                    break;
                }

                if (tag_diag_collect)
                    tag_diag.slots[resp_slot].no_irq++;
                dwt_forcetrxoff();
                dwt_rxreset();
                rx_status = RX_WAIT;
                next_period_time = portGetTickCnt();
                range_status = RANGE_ERROR;
                state = STA_IDLE;
            }
        }
		break;

		/* 接收Resp帧 */
        case STA_RECV_RESP:
        {
            static uint8_t no_resp_count = 0;   //未接收到任何基站回复的周期计数
            uint8_t has_function_code = rx_status == RX_OK &&
                rx_frame_len >= FUNC_CODE_IDX + 1 + FCS_LEN && rx_frame_len <= sizeof(rx_buffer);
            uint8_t other_function = has_function_code && rx_buffer[FUNC_CODE_IDX] != FUNC_CODE_RESP;
            uint8_t valid_resp = has_function_code && !other_function &&
                rx_frame_len == RESP_MSG_LEN + FCS_LEN && rx_buffer[RANGE_NB_IDX] == range_nb &&
                rx_buffer[SENDER_SHORT_ADD_IDX] < MAX_AHCHOR_NUMBER &&
                (single_resp_id == MAX_AHCHOR_NUMBER || rx_buffer[SENDER_SHORT_ADD_IDX] == single_resp_id);

            if (tag_try_sfd_retry(resp_slot))
            {
                state = STA_WAIT_RESP;
                break;
            }
            if (tag_diag_collect)
                tag_diag_record_rx(resp_slot, valid_resp);

            if (other_function)
            {
                if (tag_diag_collect)
                    tag_diag.rx_abort++;
                next_period_time = range_time + inst_one_slot_time * inst_slot_number + ((Correction_flag == 1)?0:(diff_time));  //如该标签未被时序校准，初次上电，增加随机时间，避免一直冲突      
                rx_status = RX_WAIT;
                range_status = RANGE_ERROR;
                state = STA_IDLE;
                break;
            }

            if (valid_resp)
            {
                recv_anc_id = rx_buffer[SENDER_SHORT_ADD_IDX];
                uint8_t resp_bit = 1U << recv_anc_id;
                if ((resp_valid & resp_bit) == 0)
                {
                    resp_valid |= resp_bit;
                    resp_rx_ts[recv_anc_id] = get_rx_timestamp_u64();
                    if (single_resp_id < MAX_AHCHOR_NUMBER || recv_anc_id == resp_slot)
                        tag_diag_record_resp_time(resp_slot, resp_rx_ts[recv_anc_id]);

                    /* Resp 携带的是基站上一轮的距离。 */
                    uint32_t distance = ((uint32_t)rx_buffer[RESP_MSG_PREV_DIS_IDX] << 24) |
                        ((uint32_t)rx_buffer[RESP_MSG_PREV_DIS_IDX + 1] << 16) |
                        ((uint32_t)rx_buffer[RESP_MSG_PREV_DIS_IDX + 2] << 8) |
                        rx_buffer[RESP_MSG_PREV_DIS_IDX + 3];
                    distance_report[recv_anc_id] = (int32_t)distance;
                    group_report[recv_anc_id] = rx_buffer[RESP_MSG_GROUP_IDX] & 0x7f;

                    dwt_rxdiag_t rx_diag;
                    dwt_readdiagnostics(&rx_diag);
                    rx_power = rx_diag.rxPower;
                    rssi[recv_anc_id] = rx_power;

                    alarm += rx_buffer[RESP_MSG_ALARM_IDX];
                    /* 组 ID 最高位为 1 表示该基站提供标签时隙校准。 */
                    if (rx_buffer[RESP_MSG_GROUP_IDX] != 0xff && (rx_buffer[RESP_MSG_GROUP_IDX] & 0x80))
                    {
                        tagSleepCorrection_ms = (int16)(((uint16)rx_buffer[RESP_MSG_SLEEP_COR_IDX] << 8) |
                            rx_buffer[RESP_MSG_SLEEP_COR_IDX + 1]);
                        Correction_flag = 1;
                    }
                }
            }

            rx_status = RX_WAIT;
            if (single_resp_id < MAX_AHCHOR_NUMBER || resp_slot == MAX_AHCHOR_NUMBER - 1)
            {
                if (resp_valid == 0)
                {
                    if (tag_diag_collect)
                        tag_diag.no_resp++;
                    no_resp_count++;
                    if (no_resp_count > 10)
                    {
                        Correction_flag = 0;
                        no_resp_count = 0;
                    }
                    next_period_time = range_time + inst_one_slot_time * inst_slot_number + ((Correction_flag == 1)?0:(diff_time));
                    state = STA_IDLE;
                    range_status = RANGE_ERROR;
                }
                else
                {
                    state = STA_SEND_FINAL;
                    no_resp_count = 0;
                }
                break;
            }

            /* 继续等待下一槽，空窗口照常推进。 */
            resp_slot++;
            if (tag_start_resp_rx(resp_slot) == DWT_ERROR)
            {
                next_period_time = range_time + inst_one_slot_time * inst_slot_number + ((Correction_flag == 1)?0:(diff_time));
                range_status = RANGE_ERROR;
                state = STA_IDLE;
                break;
            }
            state = STA_WAIT_RESP;
            break;
        }

        case STA_SEND_FINAL:
        {
            uint64_t final_tx_time;  //设置final发送时间
			
            if(inst_dataRate == DWT_BR_110K)
                final_tx_time = (poll_tx_ts + inst_poll2final_time + TAG_FINALE_SEND_BACK_110K * UUS_TO_DWT_TIME);  //设置final发送时间
            else if(inst_dataRate == DWT_BR_6M8)
                final_tx_time = (poll_tx_ts + inst_poll2final_time + TAG_FINALE_SEND_BACK_6P8M * UUS_TO_DWT_TIME);  //设置final发送时间
            else if(inst_dataRate == DWT_BR_850K)
                final_tx_time = (poll_tx_ts + inst_poll2final_time + TAG_FINALE_SEND_BACK_850K * UUS_TO_DWT_TIME);  //设置final发送时间
            final_tx_time = final_tx_time >> 8;
            dwt_setdelayedtrxtime((uint32)final_tx_time); //在final_tx_time这个时间发送数据
            final_tx_ts = (((uint64_t)(final_tx_time & 0xFFFFFFFEUL)) << 8) + ant_dly;  //final发送时间戳

            //final数据包内写入poll_tx时间戳，final_tx时间戳，4个resp_rx时间戳
            final_msg_set_ts(&tx_final_msg[FINAL_MSG_POLL_TX_TS_IDX], poll_tx_ts);
            final_msg_set_ts(&tx_final_msg[FINAL_MSG_FINAL_TX_TS_IDX], final_tx_ts);
            for(int i = 0; i < MAX_AHCHOR_NUMBER; i++)
            {
                final_msg_set_ts(&tx_final_msg[FINAL_MSG_RESP1_RX_TS_IDX + i * (FINAL_MSG_TS_LEN + 1)], resp_rx_ts[i]);
            }

            //final数据打包
            tx_final_msg[SEQ_NB_IDX] = frame_seq_nb++;
            tx_final_msg[PANID_IDX] = (uint8_t)PAN_ID; 
            tx_final_msg[PANID_IDX + 1] = (uint8_t)(PAN_ID>>8); 
            tx_final_msg[RANGE_NB_IDX] = range_nb;
            tx_final_msg[SENDER_SHORT_ADD_IDX] = tag_id;
            tx_final_msg[FINAL_MSG_FINAL_VALID_IDX] = resp_valid;
            tx_final_msg[FUNC_CODE_IDX] = FUNC_CODE_FINAL;

            for(int i = 0; i < MAX_AHCHOR_NUMBER; i++)
            {
                tx_final_msg[FINAL_MSG_A0_GROUP_ID_IDX + i*5] = group_report[i];
            }
//			tx_final_msg[FINAL_MSG_A4_GROUP_ID_IDX] = g_CalFlag;
            dwt_writetxdata(FIANL_MSG_LEN + FCS_LEN, tx_final_msg, 0); //数据写入数据缓冲区
            dwt_writetxfctrl(FIANL_MSG_LEN + FCS_LEN, 0, 1); 

            tx_status = TX_WAIT;                              //发送状态标志，在中断回调函数变更
            int ret = dwt_starttx(DWT_START_TX_DELAYED);      //延时发送
            if(ret == DWT_ERROR)
            {
                if (tag_diag_collect)
                    tag_diag.final_tx_fail++;
                next_period_time = range_time + inst_one_slot_time * inst_slot_number;//设置下个周期开始时间
                range_status = RANGE_ERROR;
                state = STA_IDLE;
                break;
            }
//            while(tx_status == TX_WAIT);            //等待发送成功，tx_status在发送成功中断内变更状态
			//等待发送成功
			uint32_t tx_wait=uwTick;
			while (tx_status == TX_WAIT)
			{
				if (JudgeTick(tx_wait, 10) == true)	//10ms超时
				{
					if (tag_diag_collect)
						tag_diag.final_tx_fail++;
					dwt_forcetrxoff();
					tx_status = TX_WAIT;
					range_status = RANGE_ERROR;
					next_period_time = range_time + inst_one_slot_time * inst_slot_number + tagSleepCorrection_ms;  //设置下个周期开始时间
					tagSleepCorrection_ms = 0;
					state = STA_IDLE;
					break;
				}
			}
			if (tx_status != TX_WAIT)	//检测正常
			{
				if (tag_diag_collect)
					tag_diag.final_tx_ok++;
				tx_status = TX_WAIT;                    //清标志
				range_status = RANGE_TWR_OK;            //设置TWR成功测距标志，在dw_main.c里判断打包串口输出
				next_period_time = range_time + inst_one_slot_time * inst_slot_number + tagSleepCorrection_ms;  //设置下个周期开始时间
				tagSleepCorrection_ms = 0;
				state = STA_IDLE;
			}
            break;
        }
        
        case STA_IDLE:
        {
#if defined(L051_DEV)
//            read_key();
#endif
            static unsigned long nowtime;
            nowtime = portGetTickCnt();
            if (tag_diag_enabled && (uint32_t)(nowtime - tag_diag_started) >= TAG_DIAG_PERIOD_MS)
            {
                tag_diag_report();
                nowtime = portGetTickCnt();
            }

            {
                if(nowtime >= next_period_time)  //下个周期发送时间到
                {
					tag_id = dev_id;
					instance_mode = TAG; 
					set_instance();	
                    dwt_forcetrxoff();
                    battery = 0;
//                    if(USE_CW2015 == 1)
//                    {
//                        battery = cw2015_read_battery();
//                    }
                    state = STA_SEND_POLL;
                    break;
                }
            }

            //LED控制
            if((portGetTickCnt()%200 == 0) && (portGetTickCnt() > (led_time + 5)) && ((sos == 1) || (alarm > 0)))
            {
                led_time = portGetTickCnt();
                if(led_flag)
                {
//                    led_on(LED1); //红灯
//                    led_off(LED2); //绿灯
//                    led_off(LED3); //蓝
                }
                else
                {
//                    led_off(LED1); //红灯
//                    led_on(LED2); //绿灯
//                    led_off(LED3); //蓝
                }
                led_flag = !led_flag;
                //motor_on();
            }
        }
            break;
        
        default:
            break;
    }

}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn tag_rx_ok_cb()
 *
 * @brief Callback to process RX good frame events
 *
 * @param  cb_data  callback data
 *
 * @return  none
 */
void tag_rx_ok_cb(const dwt_cb_data_t *cb_data)
{
    rx_frame_len = cb_data->datalength;
    if (rx_frame_len <= sizeof(rx_buffer))
    {
        dwt_readrxdata(rx_buffer, rx_frame_len, 0);
    }
    rx_status = RX_OK;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn tag_rx_to_cb()
 *
 * @brief Callback to process RX timeout events
 *
 * @param  cb_data  callback data
 *
 * @return  none
 */
void tag_rx_to_cb(const dwt_cb_data_t *cb_data)
{
    rx_event_status = cb_data->status;
    rx_status = RX_TIMEOUT;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn tag_rx_err_cb()
 *
 * @brief Callback to process RX error events
 *
 * @param  cb_data  callback data
 *
 * @return  none
 */
void tag_rx_err_cb(const dwt_cb_data_t *cb_data)
{
    rx_event_status = cb_data->status;
    rx_status = RX_ERROR;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn tag_tx_conf_cb()
 *
 * @brief Callback to process TX confirmation events
 *
 * @param  cb_data  callback data
 *
 * @return  none
 */
void tag_tx_conf_cb(const dwt_cb_data_t *cb_data)
{
    tx_status = TX_OK;
    UNUSED(cb_data);
}

