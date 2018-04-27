/***************************************************************************//**
 *   @file   ad9361_get_config.c
 *   @brief  AD9361 API Driver.
********************************************************************************
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <yaml.h>
#include "ad9361_get_config.h"

/***************************************************************************//**
 * @brief ad9361_get_config
*******************************************************************************/
int32_t ad9361_get_config(AD9361_InitParam *init_param)
{
	FILE *fh;
	yaml_parser_t parser;
	yaml_token_t  token;

	fh = fopen(AD9361_CONF_FILE, "r");
	if(fh == NULL)
	{
		printf("Failed to open %s file!\n", AD9361_CONF_FILE);
		return -1;
	}
	/* Initialize parser */
	if(!yaml_parser_initialize(&parser))
	{
		printf("Failed to initialize parser!\n");
		return -1;
	}

	/* Set input file */
	yaml_parser_set_input_file(&parser, fh);

	/* As this is an example, I'll just use:
	 *  state = 0 = expect key
	 *  state = 1 = expect value
	 */
	uint32_t state = 0;

	uint32_t key = 0;
	char *tk = NULL;

	do
	{
		yaml_parser_scan(&parser, &token);
		switch(token.type)
		{
			/* Stream start/end */
			case YAML_STREAM_START_TOKEN:
			/* puts("STREAM START"); */
			break;

			case YAML_STREAM_END_TOKEN:
				/* puts("STREAM END"); */
				break;

			/* Token types (read before actual token) */
			case YAML_KEY_TOKEN:
				/* printf("(Key token)   "); */
				state = 0;
				break;

			case YAML_VALUE_TOKEN:
				/* printf("(Value token) "); */
				state = 1;
				break;

			/* Block delimeters */
			case YAML_BLOCK_SEQUENCE_START_TOKEN:
				/* puts("<b>Start Block (Sequence)</b>"); */
				break;

			case YAML_BLOCK_ENTRY_TOKEN:
				/* puts("<b>Start Block (Entry)</b>"); */
				break;

			case YAML_BLOCK_END_TOKEN:
				/* puts("<b>End block</b>"); */
				break;

			/* Data */
			case YAML_BLOCK_MAPPING_START_TOKEN:
				/* puts("[Block mapping]"); */
				break;

			case YAML_SCALAR_TOKEN:
				tk = (char*)token.data.scalar.value;
				if (state == 0)
				{
					parse_keys((char*)tk, &key);
				}
				else if (state == 1)
				{
					fill_ad9361_st(init_param, (char*)tk, &key);
				}
				break;

			/* Others */
			default:
				/* printf("Got token of type %d\n", token.type); */
				continue;
		}
		if(token.type != YAML_STREAM_END_TOKEN)
		{
			yaml_token_delete(&token);
		}
	} while(token.type != YAML_STREAM_END_TOKEN);

	yaml_token_delete(&token);
	/* END new code */

	/* Cleanup */
	yaml_parser_delete(&parser);
	fclose(fh);

	return 0;
}

/***************************************************************************//**
 * @brief ad9361_get_config_fir
*******************************************************************************/
int32_t ad9361_get_config_fir(AD9361_InitParam *init_param, AD9361_TXFIRConfig *tx, AD9361_RXFIRConfig *rx)
{
	FILE *fh;
	char *buf;
	uint32_t size;
	uint8_t index;

	fh = fopen(AD9361_CONF_FIR_FILE, "r");
	if (fh == NULL)
	{
		printf("Failed to open %s file!\n", AD9361_CONF_FIR_FILE);
		return -1;
	}

	fseek (fh, 0, SEEK_END);
	size = ftell(fh);
	fseek (fh, 0, SEEK_SET);

	index = 0;

	buf = (char*)malloc(sizeof(char) * size);

	memset(&tx->tx_coef, 0, sizeof(tx->tx_coef));
	memset(&rx->rx_coef, 0, sizeof(rx->rx_coef));

	while(fgets(buf, size, fh))
	{
		if (strstr(buf, "TX") != NULL && strstr(buf, "RTX") == NULL && strstr(buf, "BWTX") == NULL)
		{
			sscanf(buf, "TX %"SCNu32" GAIN %"SCNi32" INT %"SCNu32"", &tx->tx, &tx->tx_gain, &tx->tx_int);
		}
		else if (strstr(buf, "RX") != NULL && strstr(buf, "RRX") == NULL && strstr(buf, "BWRX") == NULL)
		{
			sscanf(buf, "RX %"SCNu32" GAIN %"SCNi32" DEC %"SCNu32"", &rx->rx, &rx->rx_gain, &rx->rx_dec);
		}
		else if (strstr(buf, "RTX") != NULL)
		{
			sscanf(buf, "RTX %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32"",
				&tx->tx_path_clks[0],
				&tx->tx_path_clks[1],
				&tx->tx_path_clks[2],
				&tx->tx_path_clks[3],
				&tx->tx_path_clks[4],
				&tx->tx_path_clks[5]
			);
		}
		else if (strstr(buf, "RRX") != NULL)
		{
			sscanf(buf, "RRX %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32" %"SCNu32"",
				&rx->rx_path_clks[0],
				&rx->rx_path_clks[1],
				&rx->rx_path_clks[2],
				&rx->rx_path_clks[3],
				&rx->rx_path_clks[4],
				&rx->rx_path_clks[5]
			);
		}
		else if (strstr(buf, "BWTX ") != NULL)
		{
			sscanf(buf, "BWTX %"SCNu32"", &tx->tx_bandwidth);
		}
		else if (strstr(buf, "BWRX ") != NULL)
		{
			sscanf(buf, "BWRX %"SCNu32"", &rx->rx_bandwidth);
		}
		else if (strstr(buf, ",") != NULL)
		{
			sscanf(buf, "%"SCNi16",%"SCNi16"", &tx->tx_coef[index], &rx->rx_coef[index]);
			index++;
		}
	}

	if (index <= MAX_NUM_TAPS_INTERPOLATION_1)
	{
		index = MAX_NUM_TAPS_INTERPOLATION_1;
	}
	else if (index > MAX_NUM_TAPS_INTERPOLATION_1 && index <= MAX_NUM_TAPS_INTERPOLATION_2)
	{
		index = MAX_NUM_TAPS_INTERPOLATION_2;
	}
	else
	{
		return -1;
	}

	tx->tx_coef_size = index;
	rx->rx_coef_size = index;

	init_param->tx_path_clock_frequencies[0] = tx->tx_path_clks[0];
	init_param->tx_path_clock_frequencies[1] = tx->tx_path_clks[1];
	init_param->tx_path_clock_frequencies[2] = tx->tx_path_clks[2];
	init_param->tx_path_clock_frequencies[3] = tx->tx_path_clks[3];
	init_param->tx_path_clock_frequencies[4] = tx->tx_path_clks[4];
	init_param->tx_path_clock_frequencies[5] = tx->tx_path_clks[5];

	init_param->rx_path_clock_frequencies[0] = rx->rx_path_clks[0];
	init_param->rx_path_clock_frequencies[1] = rx->rx_path_clks[1];
	init_param->rx_path_clock_frequencies[2] = rx->rx_path_clks[2];
	init_param->rx_path_clock_frequencies[3] = rx->rx_path_clks[3];
	init_param->rx_path_clock_frequencies[4] = rx->rx_path_clks[4];
	init_param->rx_path_clock_frequencies[5] = rx->rx_path_clks[5];

	init_param->rf_tx_bandwidth_hz = tx->tx_bandwidth;
	init_param->rf_rx_bandwidth_hz = rx->rx_bandwidth;

	free(buf);
	fclose(fh);

	return 0;
}

/***************************************************************************//**
 * @brief parse_key
*******************************************************************************/
void parse_keys(char *tk, uint32_t *key)
{
	/* Identification number */
	if (!strcmp(tk, "id_no"))
	{
		*key = id_no_key;
	}
	/* Reference Clock */
	else if (!strcmp(tk, "reference_clk_rate"))
	{
		*key = reference_clk_rate_key;
	}
	/* Base Configuration */
	else if (!strcmp(tk, "two_rx_two_tx_mode_enable"))
	{
		*key = two_rx_two_tx_mode_enable_key;
	}
	else if (!strcmp(tk, "one_rx_one_tx_mode_use_rx_num"))
	{
		*key = one_rx_one_tx_mode_use_rx_num_key;
	}
	else if (!strcmp(tk, "one_rx_one_tx_mode_use_tx_num"))
	{
		*key = one_rx_one_tx_mode_use_tx_num_key;
	}
	else if (!strcmp(tk, "frequency_division_duplex_mode_enable"))
	{
		*key = frequency_division_duplex_mode_enable_key;
	}
	else if (!strcmp(tk, "frequency_division_duplex_independent_mode_enable"))
	{
		*key = frequency_division_duplex_independent_mode_enable_key;
	}
	else if (!strcmp(tk, "tdd_use_dual_synth_mode_enable"))
	{
		*key = tdd_use_dual_synth_mode_enable_key;
	}
	else if (!strcmp(tk, "tdd_skip_vco_cal_enable"))
	{
		*key = tdd_skip_vco_cal_enable_key;
	}
	else if (!strcmp(tk, "tx_fastlock_delay_ns"))
	{
		*key = tx_fastlock_delay_ns_key;
	}
	else if (!strcmp(tk, "rx_fastlock_delay_ns"))
	{
		*key = rx_fastlock_delay_ns_key;
	}
	else if (!strcmp(tk, "rx_fastlock_pincontrol_enable"))
	{
		*key = rx_fastlock_pincontrol_enable_key;
	}
	else if (!strcmp(tk, "tx_fastlock_pincontrol_enable"))
	{
		*key = tx_fastlock_pincontrol_enable_key;
	}
	else if (!strcmp(tk, "external_rx_lo_enable"))
	{
		*key = external_rx_lo_enable_key;
	}
	else if (!strcmp(tk, "external_tx_lo_enable"))
	{
		*key = external_tx_lo_enable_key;
	}
	else if (!strcmp(tk, "dc_offset_tracking_update_event_mask"))
	{
		*key = dc_offset_tracking_update_event_mask_key;
	}
	else if (!strcmp(tk, "dc_offset_attenuation_high_range"))
	{
		*key = dc_offset_attenuation_high_range_key;
	}
	else if (!strcmp(tk, "dc_offset_attenuation_low_range"))
	{
		*key = dc_offset_attenuation_low_range_key;
	}
	else if (!strcmp(tk, "dc_offset_count_high_range"))
	{
		*key = dc_offset_count_high_range_key;
	}
	else if (!strcmp(tk, "dc_offset_count_low_range"))
	{
		*key = dc_offset_count_low_range_key;
	}
	else if (!strcmp(tk, "split_gain_table_mode_enable"))
	{
		*key = split_gain_table_mode_enable_key;
	}
	else if (!strcmp(tk, "trx_synthesizer_target_fref_overwrite_hz"))
	{
		*key = trx_synthesizer_target_fref_overwrite_hz_key;
	}
	else if (!strcmp(tk, "qec_tracking_slow_mode_enable"))
	{
		*key = qec_tracking_slow_mode_enable_key;
	}
	/* ENSM Control */
	else if (!strcmp(tk, "ensm_enable_pin_pulse_mode_enable"))
	{
		*key = ensm_enable_pin_pulse_mode_enable_key;
	}
	else if (!strcmp(tk, "ensm_enable_txnrx_control_enable"))
	{
		*key = ensm_enable_txnrx_control_enable_key;
	}
	/* LO Control */
	else if (!strcmp(tk, "rx_synthesizer_frequency_hz"))
	{
		*key = rx_synthesizer_frequency_hz_key;
	}
	else if (!strcmp(tk, "tx_synthesizer_frequency_hz"))
	{
		*key = tx_synthesizer_frequency_hz_key;
	}
	/* RF Port Control */
	else if (!strcmp(tk, "rx_rf_port_input_select"))
	{
		*key = rx_rf_port_input_select_key;
	}
	else if (!strcmp(tk, "tx_rf_port_input_select"))
	{
		*key = tx_rf_port_input_select_key;
	}
	/* TX Attenuation Control */
	else if (!strcmp(tk, "tx_attenuation_mdB"))
	{
		*key = tx_attenuation_mdB_key;
	}
	else if (!strcmp(tk, "update_tx_gain_in_alert_enable"))
	{
		*key = update_tx_gain_in_alert_enable_key;
	}
	/* Reference Clock Control */
	else if (!strcmp(tk, "xo_disable_use_ext_refclk_enable"))
	{
		*key = xo_disable_use_ext_refclk_enable_key;
	}
	else if (!strcmp(tk, "dcxo_coarse_and_fine_tune[0]"))
	{
		*key = dcxo_coarse_and_fine_tune_0_key;
	}
	else if (!strcmp(tk, "dcxo_coarse_and_fine_tune[1]"))
	{
		*key = dcxo_coarse_and_fine_tune_1_key;
	}
	else if (!strcmp(tk, "clk_output_mode_select"))
	{
		*key = clk_output_mode_select_key;
	}
	/* Gain Control */
	else if (!strcmp(tk, "gc_rx1_mode"))
	{
		*key = gc_rx1_mode_key;
	}
	else if (!strcmp(tk, "gc_rx2_mode"))
	{
		*key = gc_rx2_mode_key;
	}
	else if (!strcmp(tk, "gc_adc_large_overload_thresh"))
	{
		*key = gc_adc_large_overload_thresh_key;
	}
	else if (!strcmp(tk, "gc_adc_ovr_sample_size"))
	{
		*key = gc_adc_ovr_sample_size_key;
	}
	else if (!strcmp(tk, "gc_adc_small_overload_thresh"))
	{
		*key = gc_adc_small_overload_thresh_key;
	}
	else if (!strcmp(tk, "gc_dec_pow_measurement_duration"))
	{
		*key = gc_dec_pow_measurement_duration_key;
	}
	else if (!strcmp(tk, "gc_dig_gain_enable"))
	{
		*key = gc_dig_gain_enable_key;
	}
	else if (!strcmp(tk, "gc_lmt_overload_high_thresh"))
	{
		*key = gc_lmt_overload_high_thresh_key;
	}
	else if (!strcmp(tk, "gc_lmt_overload_low_thresh"))
	{
		*key = gc_lmt_overload_low_thresh_key;
	}
	else if (!strcmp(tk, "gc_low_power_thresh"))
	{
		*key = gc_low_power_thresh_key;
	}
	else if (!strcmp(tk, "gc_max_dig_gain"))
	{
		*key = gc_max_dig_gain_key;
	}
	/* Gain MGC Control */
	else if (!strcmp(tk, "mgc_dec_gain_step"))
	{
		*key = mgc_dec_gain_step_key;
	}
	else if (!strcmp(tk, "mgc_inc_gain_step"))
	{
		*key = mgc_inc_gain_step_key;
	}
	else if (!strcmp(tk, "mgc_rx1_ctrl_inp_enable"))
	{
		*key = mgc_rx1_ctrl_inp_enable_key;
	}
	else if (!strcmp(tk, "mgc_rx2_ctrl_inp_enable"))
	{
		*key = mgc_rx2_ctrl_inp_enable_key;
	}
	else if (!strcmp(tk, "mgc_split_table_ctrl_inp_gain_mode"))
	{
		*key = mgc_split_table_ctrl_inp_gain_mode_key;
	}
	/* Gain AGC Control */
	else if (!strcmp(tk, "agc_adc_large_overload_exceed_counter"))
	{
		*key = agc_adc_large_overload_exceed_counter_key;
	}
	else if (!strcmp(tk, "agc_adc_large_overload_inc_steps"))
	{
		*key = agc_adc_large_overload_inc_steps_key;
	}
	else if (!strcmp(tk, "agc_adc_lmt_small_overload_prevent_gain_inc_enable"))
	{
		*key = agc_adc_lmt_small_overload_prevent_gain_inc_enable_key;
	}
	else if (!strcmp(tk, "agc_adc_small_overload_exceed_counter"))
	{
		*key = agc_adc_small_overload_exceed_counter_key;
	}
	else if (!strcmp(tk, "agc_dig_gain_step_size"))
	{
		*key = agc_dig_gain_step_size_key;
	}
	else if (!strcmp(tk, "agc_dig_saturation_exceed_counter"))
	{
		*key = agc_dig_saturation_exceed_counter_key;
	}
	else if (!strcmp(tk, "agc_gain_update_interval_us"))
	{
		*key = agc_gain_update_interval_us_key;
	}
	else if (!strcmp(tk, "agc_immed_gain_change_if_large_adc_overload_enable"))
	{
		*key = agc_immed_gain_change_if_large_adc_overload_enable_key;
	}
	else if (!strcmp(tk, "agc_immed_gain_change_if_large_lmt_overload_enable"))
	{
		*key = agc_immed_gain_change_if_large_lmt_overload_enable_key;
	}
	else if (!strcmp(tk, "agc_inner_thresh_high"))
	{
		*key = agc_inner_thresh_high_key;
	}
	else if (!strcmp(tk, "agc_inner_thresh_high_dec_steps"))
	{
		*key = agc_inner_thresh_high_dec_steps_key;
	}
	else if (!strcmp(tk, "agc_inner_thresh_low"))
	{
		*key = agc_inner_thresh_low_key;
	}
	else if (!strcmp(tk, "agc_inner_thresh_low_inc_steps"))
	{
		*key = agc_inner_thresh_low_inc_steps_key;
	}
	else if (!strcmp(tk, "agc_lmt_overload_large_exceed_counter"))
	{
		*key = agc_lmt_overload_large_exceed_counter_key;
	}
	else if (!strcmp(tk, "agc_lmt_overload_large_inc_steps"))
	{
		*key = agc_lmt_overload_large_inc_steps_key;
	}
	else if (!strcmp(tk, "agc_lmt_overload_small_exceed_counter"))
	{
		*key = agc_lmt_overload_small_exceed_counter_key;
	}
	else if (!strcmp(tk, "agc_outer_thresh_high"))
	{
		*key = agc_outer_thresh_high_key;
	}
	else if (!strcmp(tk, "agc_outer_thresh_high_dec_steps"))
	{
		*key = agc_outer_thresh_high_dec_steps_key;
	}
	else if (!strcmp(tk, "agc_outer_thresh_low"))
	{
		*key = agc_outer_thresh_low_key;
	}
	else if (!strcmp(tk, "agc_outer_thresh_low_inc_steps"))
	{
		*key = agc_outer_thresh_low_inc_steps_key;
	}
	else if (!strcmp(tk, "agc_attack_delay_extra_margin_us"))
	{
		*key = agc_attack_delay_extra_margin_us_key;
	}
	else if (!strcmp(tk, "agc_sync_for_gain_counter_enable"))
	{
		*key = agc_sync_for_gain_counter_enable_key;
	}
	/* Fast AGC */
	else if (!strcmp(tk, "fagc_dec_pow_measuremnt_duration"))
	{
		*key = fagc_dec_pow_measuremnt_duration_key;
	}
	else if (!strcmp(tk, "fagc_state_wait_time_ns"))
	{
		*key = fagc_state_wait_time_ns_key;
	}
	/* Fast AGC - Low Power */
	else if (!strcmp(tk, "fagc_allow_agc_gain_increase"))
	{
		*key = fagc_allow_agc_gain_increase_key;
	}
	else if (!strcmp(tk, "fagc_lp_thresh_increment_time"))
	{
		*key = fagc_lp_thresh_increment_time_key;
	}
	else if (!strcmp(tk, "fagc_lp_thresh_increment_steps"))
	{
		*key = fagc_lp_thresh_increment_steps_key;
	}
	/* Fast AGC - Lock Level */
	else if (!strcmp(tk, "fagc_lock_level_lmt_gain_increase_en"))
	{
		*key = fagc_lock_level_lmt_gain_increase_en_key;
	}
	else if (!strcmp(tk, "fagc_lock_level_gain_increase_upper_limit"))
	{
		*key = fagc_lock_level_gain_increase_upper_limit_key;
	}
	/* Fast AGC - Peak Detectors and Final Settling */
	else if (!strcmp(tk, "fagc_lpf_final_settling_steps"))
	{
		*key = fagc_lpf_final_settling_steps_key;
	}
	else if (!strcmp(tk, "fagc_lmt_final_settling_steps"))
	{
		*key = fagc_lmt_final_settling_steps_key;
	}
	else if (!strcmp(tk, "fagc_final_overrange_count"))
	{
		*key = fagc_final_overrange_count_key;
	}
	/* Fast AGC - Final Power Test */
	else if (!strcmp(tk, "fagc_gain_increase_after_gain_lock_en"))
	{
		*key = fagc_gain_increase_after_gain_lock_en_key;
	}
	/* Fast AGC - Unlocking the Gain */
	else if (!strcmp(tk, "fagc_gain_index_type_after_exit_rx_mode"))
	{
		*key = fagc_gain_index_type_after_exit_rx_mode_key;
	}
	else if (!strcmp(tk, "fagc_use_last_lock_level_for_set_gain_en"))
	{
		*key = fagc_use_last_lock_level_for_set_gain_en_key;
	}
	else if (!strcmp(tk, "fagc_rst_gla_stronger_sig_thresh_exceeded_en"))
	{
		*key = fagc_rst_gla_stronger_sig_thresh_exceeded_en_key;
	}
	else if (!strcmp(tk, "fagc_optimized_gain_offset"))
	{
		*key = fagc_optimized_gain_offset_key;
	}
	else if (!strcmp(tk, "fagc_rst_gla_stronger_sig_thresh_above_ll"))
	{
		*key = fagc_rst_gla_stronger_sig_thresh_above_ll_key;
	}
	else if (!strcmp(tk, "fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en"))
	{
		*key = fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en_key;
	}
	else if (!strcmp(tk, "fagc_rst_gla_engergy_lost_goto_optim_gain_en"))
	{
		*key = fagc_rst_gla_engergy_lost_goto_optim_gain_en_key;
	}
	else if (!strcmp(tk, "fagc_rst_gla_engergy_lost_sig_thresh_below_ll"))
	{
		*key = fagc_rst_gla_engergy_lost_sig_thresh_below_ll_key;
	}
	else if (!strcmp(tk, "fagc_energy_lost_stronger_sig_gain_lock_exit_cnt"))
	{
		*key = fagc_energy_lost_stronger_sig_gain_lock_exit_cnt_key;
	}
	else if (!strcmp(tk, "fagc_rst_gla_large_adc_overload_en"))
	{
		*key = fagc_rst_gla_large_adc_overload_en_key;
	}
	else if (!strcmp(tk, "fagc_rst_gla_large_lmt_overload_en"))
	{
		*key = fagc_rst_gla_large_lmt_overload_en_key;
	}
	else if (!strcmp(tk, "fagc_rst_gla_en_agc_pulled_high_en"))
	{
		*key = fagc_rst_gla_en_agc_pulled_high_en_key;
	}
	else if (!strcmp(tk, "fagc_rst_gla_if_en_agc_pulled_high_mode"))
	{
		*key = fagc_rst_gla_if_en_agc_pulled_high_mode_key;
	}
	else if (!strcmp(tk, "fagc_power_measurement_duration_in_state5"))
	{
		*key = fagc_power_measurement_duration_in_state5_key;
	}
	/* RSSI Control */
	else if (!strcmp(tk, "rssi_delay"))
	{
		*key = rssi_delay_key;
	}
	else if (!strcmp(tk, "rssi_duration"))
	{
		*key = rssi_duration_key;
	}
	else if (!strcmp(tk, "rssi_restart_mode"))
	{
		*key = rssi_restart_mode_key;
	}
	else if (!strcmp(tk, "rssi_unit_is_rx_samples_enable"))
	{
		*key = rssi_unit_is_rx_samples_enable_key;
	}
	else if (!strcmp(tk, "rssi_wait"))
	{
		*key = rssi_wait_key;
	}
	/* Aux ADC Control */
	else if (!strcmp(tk, "aux_adc_decimation"))
	{
		*key = aux_adc_decimation_key;
	}
	else if (!strcmp(tk, "aux_adc_rate"))
	{
		*key = aux_adc_rate_key;
	}
	/* AuxDAC Control */
	else if (!strcmp(tk, "aux_dac_manual_mode_enable"))
	{
		*key = aux_dac_manual_mode_enable_key;
	}
	else if (!strcmp(tk, "aux_dac1_default_value_mV"))
	{
		*key = aux_dac1_default_value_mV_key;
	}
	else if (!strcmp(tk, "aux_dac1_active_in_rx_enable"))
	{
		*key = aux_dac1_active_in_rx_enable_key;
	}
	else if (!strcmp(tk, "aux_dac1_active_in_tx_enable"))
	{
		*key = aux_dac1_active_in_tx_enable_key;
	}
	else if (!strcmp(tk, "aux_dac1_active_in_alert_enable"))
	{
		*key = aux_dac1_active_in_alert_enable_key;
	}
	else if (!strcmp(tk, "aux_dac1_rx_delay_us"))
	{
		*key = aux_dac1_rx_delay_us_key;
	}
	else if (!strcmp(tk, "aux_dac1_tx_delay_us"))
	{
		*key = aux_dac1_tx_delay_us_key;
	}
	else if (!strcmp(tk, "aux_dac2_default_value_mV"))
	{
		*key = aux_dac2_default_value_mV_key;
	}
	else if (!strcmp(tk, "aux_dac2_active_in_rx_enable"))
	{
		*key = aux_dac2_active_in_rx_enable_key;
	}
	else if (!strcmp(tk, "aux_dac2_active_in_tx_enable"))
	{
		*key = aux_dac2_active_in_tx_enable_key;
	}
	else if (!strcmp(tk, "aux_dac2_active_in_alert_enable"))
	{
		*key = aux_dac2_active_in_alert_enable_key;
	}
	else if (!strcmp(tk, "aux_dac2_rx_delay_us"))
	{
		*key = aux_dac2_rx_delay_us_key;
	}
	else if (!strcmp(tk, "aux_dac2_tx_delay_us"))
	{
		*key = aux_dac2_tx_delay_us_key;
	}
	/* Temperature Sensor Control */
	else if (!strcmp(tk, "temp_sense_decimation"))
	{
		*key = temp_sense_decimation_key;
	}
	else if (!strcmp(tk, "temp_sense_measurement_interval_ms"))
	{
		*key = temp_sense_measurement_interval_ms_key;
	}
	else if (!strcmp(tk, "temp_sense_offset_signed"))
	{
		*key = temp_sense_offset_signed_key;
	}
	else if (!strcmp(tk, "temp_sense_periodic_measurement_enable"))
	{
		*key = temp_sense_periodic_measurement_enable_key;
	}
	/* Control Out Setup */
	else if (!strcmp(tk, "ctrl_outs_enable_mask"))
	{
		*key = ctrl_outs_enable_mask_key;
	}
	else if (!strcmp(tk, "ctrl_outs_index"))
	{
		*key = ctrl_outs_index_key;
	}
	/* External LNA Control */
	else if (!strcmp(tk, "elna_settling_delay_ns"))
	{
		*key = elna_settling_delay_ns_key;
	}
	else if (!strcmp(tk, "elna_gain_mdB"))
	{
		*key = elna_gain_mdB_key;
	}
	else if (!strcmp(tk, "elna_bypass_loss_mdB"))
	{
		*key = elna_bypass_loss_mdB_key;
	}
	else if (!strcmp(tk, "elna_rx1_gpo0_control_enable"))
	{
		*key = elna_rx1_gpo0_control_enable_key;
	}
	else if (!strcmp(tk, "elna_rx2_gpo1_control_enable"))
	{
		*key = elna_rx2_gpo1_control_enable_key;
	}
	else if (!strcmp(tk, "elna_gaintable_all_index_enable"))
	{
		*key = elna_gaintable_all_index_enable_key;
	}
	/* Digital Interface Control */
	else if (!strcmp(tk, "digital_interface_tune_skip_mode"))
	{
		*key = digital_interface_tune_skip_mode_key;
	}
	else if (!strcmp(tk, "digital_interface_tune_fir_disable"))
	{
		*key = digital_interface_tune_fir_disable_key;
	}
	else if (!strcmp(tk, "pp_tx_swap_enable"))
	{
		*key = pp_tx_swap_enable_key;
	}
	else if (!strcmp(tk, "pp_rx_swap_enable"))
	{
		*key = pp_rx_swap_enable_key;
	}
	else if (!strcmp(tk, "tx_channel_swap_enable"))
	{
		*key = tx_channel_swap_enable_key;
	}
	else if (!strcmp(tk, "rx_channel_swap_enable"))
	{
		*key = rx_channel_swap_enable_key;
	}
	else if (!strcmp(tk, "rx_frame_pulse_mode_enable"))
	{
		*key = rx_frame_pulse_mode_enable_key;
	}
	else if (!strcmp(tk, "two_t_two_r_timing_enable"))
	{
		*key = two_t_two_r_timing_enable_key;
	}
	else if (!strcmp(tk, "invert_data_bus_enable"))
	{
		*key = invert_data_bus_enable_key;
	}
	else if (!strcmp(tk, "invert_data_clk_enable"))
	{
		*key = invert_data_clk_enable_key;
	}
	else if (!strcmp(tk, "fdd_alt_word_order_enable"))
	{
		*key = fdd_alt_word_order_enable_key;
	}
	else if (!strcmp(tk, "invert_rx_frame_enable"))
	{
		*key = invert_rx_frame_enable_key;
	}
	else if (!strcmp(tk, "fdd_rx_rate_2tx_enable"))
	{
		*key = fdd_rx_rate_2tx_enable_key;
	}
	else if (!strcmp(tk, "swap_ports_enable"))
	{
		*key = swap_ports_enable_key;
	}
	else if (!strcmp(tk, "single_data_rate_enable"))
	{
		*key = single_data_rate_enable_key;
	}
	else if (!strcmp(tk, "lvds_mode_enable"))
	{
		*key = lvds_mode_enable_key;
	}
	else if (!strcmp(tk, "half_duplex_mode_enable"))
	{
		*key = half_duplex_mode_enable_key;
	}
	else if (!strcmp(tk, "single_port_mode_enable"))
	{
		*key = single_port_mode_enable_key;
	}
	else if (!strcmp(tk, "full_port_enable"))
	{
		*key = full_port_enable_key;
	}
	else if (!strcmp(tk, "full_duplex_swap_bits_enable"))
	{
		*key = full_duplex_swap_bits_enable_key;
	}
	else if (!strcmp(tk, "delay_rx_data"))
	{
		*key = delay_rx_data_key;
	}
	else if (!strcmp(tk, "rx_data_clock_delay"))
	{
		*key = rx_data_clock_delay_key;
	}
	else if (!strcmp(tk, "rx_data_delay"))
	{
		*key = rx_data_delay_key;
	}
	else if (!strcmp(tk, "tx_fb_clock_delay"))
	{
		*key = tx_fb_clock_delay_key;
	}
	else if (!strcmp(tk, "tx_data_delay"))
	{
		*key = tx_data_delay_key;
	}
	else if (!strcmp(tk, "lvds_bias_mV"))
	{
		*key = lvds_bias_mV_key;
	}
	else if (!strcmp(tk, "lvds_rx_onchip_termination_enable"))
	{
		*key = lvds_rx_onchip_termination_enable_key;
	}
	else if (!strcmp(tk, "rx1rx2_phase_inversion_en"))
	{
		*key = rx1rx2_phase_inversion_en_key;
	}
	else if (!strcmp(tk, "lvds_invert1_control"))
	{
		*key = lvds_invert1_control_key;
	}
	else if (!strcmp(tk, "lvds_invert2_control"))
	{
		*key = lvds_invert2_control_key;
	}
	/* GPO Control */
	else if (!strcmp(tk, "gpo0_inactive_state_high_enable"))
	{
		*key = gpo0_inactive_state_high_enable_key;
	}
	else if (!strcmp(tk, "gpo1_inactive_state_high_enable"))
	{
		*key = gpo1_inactive_state_high_enable_key;
	}
	else if (!strcmp(tk, "gpo2_inactive_state_high_enable"))
	{
		*key = gpo2_inactive_state_high_enable_key;
	}
	else if (!strcmp(tk, "gpo3_inactive_state_high_enable"))
	{
		*key = gpo3_inactive_state_high_enable_key;
	}
	else if (!strcmp(tk, "gpo0_slave_rx_enable"))
	{
		*key = gpo0_slave_rx_enable_key;
	}
	else if (!strcmp(tk, "gpo0_slave_tx_enable"))
	{
		*key = gpo0_slave_tx_enable_key;
	}
	else if (!strcmp(tk, "gpo1_slave_rx_enable"))
	{
		*key = gpo1_slave_rx_enable_key;
	}
	else if (!strcmp(tk, "gpo1_slave_tx_enable"))
	{
		*key = gpo1_slave_tx_enable_key;
	}
	else if (!strcmp(tk, "gpo2_slave_rx_enable"))
	{
		*key = gpo2_slave_rx_enable_key;
	}
	else if (!strcmp(tk, "gpo2_slave_tx_enable"))
	{
		*key = gpo2_slave_tx_enable_key;
	}
	else if (!strcmp(tk, "gpo3_slave_rx_enable"))
	{
		*key = gpo3_slave_rx_enable_key;
	}
	else if (!strcmp(tk, "gpo3_slave_tx_enable"))
	{
		*key = gpo3_slave_tx_enable_key;
	}
	else if (!strcmp(tk, "gpo0_rx_delay_us"))
	{
		*key = gpo0_rx_delay_us_key;
	}
	else if (!strcmp(tk, "gpo0_tx_delay_us"))
	{
		*key = gpo0_tx_delay_us_key;
	}
	else if (!strcmp(tk, "gpo1_rx_delay_us"))
	{
		*key = gpo1_rx_delay_us_key;
	}
	else if (!strcmp(tk, "gpo1_tx_delay_us"))
	{
		*key = gpo1_tx_delay_us_key;
	}
	else if (!strcmp(tk, "gpo2_rx_delay_us"))
	{
		*key = gpo2_rx_delay_us_key;
	}
	else if (!strcmp(tk, "gpo2_tx_delay_us"))
	{
		*key = gpo2_tx_delay_us_key;
	}
	else if (!strcmp(tk, "gpo3_rx_delay_us"))
	{
		*key = gpo3_rx_delay_us_key;
	}
	else if (!strcmp(tk, "gpo3_tx_delay_us"))
	{
		*key = gpo3_tx_delay_us_key;
	}
	/* Tx Monitor Control */
	else if (!strcmp(tk, "low_high_gain_threshold_mdB"))
	{
		*key = low_high_gain_threshold_mdB_key;
	}
	else if (!strcmp(tk, "low_gain_dB"))
	{
		*key = low_gain_dB_key;
	}
	else if (!strcmp(tk, "high_gain_dB"))
	{
		*key = high_gain_dB_key;
	}
	else if (!strcmp(tk, "tx_mon_track_en"))
	{
		*key = tx_mon_track_en_key;
	}
	else if (!strcmp(tk, "one_shot_mode_en"))
	{
		*key = one_shot_mode_en_key;
	}
	else if (!strcmp(tk, "tx_mon_delay"))
	{
		*key = tx_mon_delay_key;
	}
	else if (!strcmp(tk, "tx_mon_duration"))
	{
		*key = tx_mon_duration_key;
	}
	else if (!strcmp(tk, "tx1_mon_front_end_gain"))
	{
		*key = tx1_mon_front_end_gain_key;
	}
	else if (!strcmp(tk, "tx2_mon_front_end_gain"))
	{
		*key = tx2_mon_front_end_gain_key;
	}
	else if (!strcmp(tk, "tx1_mon_lo_cm"))
	{
		*key = tx1_mon_lo_cm_key;
	}
	else if (!strcmp(tk, "tx2_mon_lo_cm"))
	{
		*key = tx2_mon_lo_cm_key;
	}
	/* GPIO definitions */
	else if (!strcmp(tk, "gpio_resetb"))
	{
		*key = gpio_resetb_key;
	}
	/* MCS Sync */
	else if (!strcmp(tk, "gpio_sync"))
	{
		*key = gpio_sync_key;
	}
	else if (!strcmp(tk, "gpio_cal_sw1"))
	{
		*key = gpio_cal_sw1_key;
	}
	else if (!strcmp(tk, "gpio_cal_sw2"))
	{
		*key = gpio_cal_sw2_key;
	}
	else
	{
		/* printf("Unrecognised key: %s\n", tk); */
	}
}

/***************************************************************************//**
 * @brief fill_ad9361_st
*******************************************************************************/
void fill_ad9361_st(AD9361_InitParam *init_param, char *tk, uint32_t *key)
{
	/* Identification number */
	if (*key == id_no_key)
	{
		init_param->id_no = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* Reference Clock */
	else if (*key == reference_clk_rate_key)
	{
		init_param->reference_clk_rate = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* Base Configuration */
	else if (*key == two_rx_two_tx_mode_enable_key)
	{
		init_param->two_rx_two_tx_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == one_rx_one_tx_mode_use_rx_num_key)
	{
		init_param->one_rx_one_tx_mode_use_rx_num = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == one_rx_one_tx_mode_use_tx_num_key)
	{
		init_param->one_rx_one_tx_mode_use_tx_num = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == frequency_division_duplex_mode_enable_key)
	{
		init_param->frequency_division_duplex_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == frequency_division_duplex_independent_mode_enable_key)
	{
		init_param->frequency_division_duplex_independent_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tdd_use_dual_synth_mode_enable_key)
	{
		init_param->tdd_use_dual_synth_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tdd_skip_vco_cal_enable_key)
	{
		init_param->tdd_skip_vco_cal_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx_fastlock_delay_ns_key)
	{
		init_param->tx_fastlock_delay_ns = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rx_fastlock_delay_ns_key)
	{
		init_param->rx_fastlock_delay_ns = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rx_fastlock_pincontrol_enable_key)
	{
		init_param->rx_fastlock_pincontrol_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx_fastlock_pincontrol_enable_key)
	{
		init_param->tx_fastlock_pincontrol_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == external_rx_lo_enable_key)
	{
		init_param->external_rx_lo_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == external_tx_lo_enable_key)
	{
		init_param->external_tx_lo_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == dc_offset_tracking_update_event_mask_key)
	{
		init_param->dc_offset_tracking_update_event_mask = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == dc_offset_attenuation_high_range_key)
	{
		init_param->dc_offset_attenuation_high_range = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == dc_offset_attenuation_low_range_key)
	{
		init_param->dc_offset_attenuation_low_range = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == dc_offset_count_high_range_key)
	{
		init_param->dc_offset_count_high_range = (uint8_t)strtol(strdup(tk), NULL, 16);
	}
	else if (*key == dc_offset_count_low_range_key)
	{
		init_param->dc_offset_count_low_range = (uint8_t)strtol(strdup(tk), NULL, 16);
	}
	else if (*key == split_gain_table_mode_enable_key)
	{
		init_param->split_gain_table_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == trx_synthesizer_target_fref_overwrite_hz_key)
	{
		init_param->trx_synthesizer_target_fref_overwrite_hz = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == qec_tracking_slow_mode_enable_key)
	{
		init_param->qec_tracking_slow_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* ENSM Control */
	else if (*key == ensm_enable_pin_pulse_mode_enable_key)
	{
		init_param->ensm_enable_pin_pulse_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == ensm_enable_txnrx_control_enable_key)
	{
		init_param->ensm_enable_txnrx_control_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* LO Control */
	else if (*key == rx_synthesizer_frequency_hz_key)
	{
		init_param->rx_synthesizer_frequency_hz = (uint64_t)strtoll(strdup(tk), NULL, 10);
	}
	else if (*key == tx_synthesizer_frequency_hz_key)
	{
		init_param->tx_synthesizer_frequency_hz = (uint64_t)strtoll(strdup(tk), NULL, 10);
	}
	/* RF Port Control */
	else if (*key == rx_rf_port_input_select_key)
	{
		init_param->rx_rf_port_input_select = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx_rf_port_input_select_key)
	{
		init_param->tx_rf_port_input_select = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* TX Attenuation Control */
	else if (*key == tx_attenuation_mdB_key)
	{
		init_param->tx_attenuation_mdB = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == update_tx_gain_in_alert_enable_key)
	{
		init_param->update_tx_gain_in_alert_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* Reference Clock Control */
	else if (*key == xo_disable_use_ext_refclk_enable_key)
	{
		init_param->xo_disable_use_ext_refclk_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == dcxo_coarse_and_fine_tune_0_key)
	{
		init_param->dcxo_coarse_and_fine_tune[0] = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == dcxo_coarse_and_fine_tune_1_key)
	{
		init_param->dcxo_coarse_and_fine_tune[1] = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == clk_output_mode_select_key)
	{
		init_param->clk_output_mode_select = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* Gain Control */
	else if (*key == gc_rx1_mode_key)
	{
		init_param->gc_rx1_mode = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_rx2_mode_key)
	{
		init_param->gc_rx2_mode = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_adc_large_overload_thresh_key)
	{
		init_param->gc_adc_large_overload_thresh = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_adc_ovr_sample_size_key)
	{
		init_param->gc_adc_ovr_sample_size = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_adc_small_overload_thresh_key)
	{
		init_param->gc_adc_small_overload_thresh = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_dec_pow_measurement_duration_key)
	{
		init_param->gc_dec_pow_measurement_duration = (uint16_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_dig_gain_enable_key)
	{
		init_param->gc_dig_gain_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_lmt_overload_high_thresh_key)
	{
		init_param->gc_lmt_overload_high_thresh = (uint16_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_lmt_overload_low_thresh_key)
	{
		init_param->gc_lmt_overload_low_thresh = (uint16_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_low_power_thresh_key)
	{
		init_param->gc_low_power_thresh = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gc_max_dig_gain_key)
	{
		init_param->gc_max_dig_gain = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* Gain MGC Control */
	else if (*key == mgc_dec_gain_step_key)
	{
		init_param->mgc_dec_gain_step = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == mgc_inc_gain_step_key)
	{
		init_param->mgc_inc_gain_step = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == mgc_rx1_ctrl_inp_enable_key)
	{
		init_param->mgc_rx1_ctrl_inp_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == mgc_rx2_ctrl_inp_enable_key)
	{
		init_param->mgc_rx2_ctrl_inp_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == mgc_split_table_ctrl_inp_gain_mode_key)
	{
		init_param->mgc_split_table_ctrl_inp_gain_mode = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* Gain AGC Control */
	else if (*key == agc_adc_large_overload_exceed_counter_key)
	{
		init_param->agc_adc_large_overload_exceed_counter = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_adc_large_overload_inc_steps_key)
	{
		init_param->agc_adc_large_overload_inc_steps = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_adc_lmt_small_overload_prevent_gain_inc_enable_key)
	{
		init_param->agc_adc_lmt_small_overload_prevent_gain_inc_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_adc_small_overload_exceed_counter_key)
	{
		init_param->agc_adc_small_overload_exceed_counter = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_dig_gain_step_size_key)
	{
		init_param->agc_dig_gain_step_size = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_dig_saturation_exceed_counter_key)
	{
		init_param->agc_dig_saturation_exceed_counter = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_gain_update_interval_us_key)
	{
		init_param->agc_gain_update_interval_us = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_immed_gain_change_if_large_adc_overload_enable_key)
	{
		init_param->agc_immed_gain_change_if_large_adc_overload_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_immed_gain_change_if_large_lmt_overload_enable_key)
	{
		init_param->agc_immed_gain_change_if_large_lmt_overload_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_inner_thresh_high_key)
	{
		init_param->agc_inner_thresh_high = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_inner_thresh_high_dec_steps_key)
	{
		init_param->agc_inner_thresh_high_dec_steps = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_inner_thresh_low_key)
	{
		init_param->agc_inner_thresh_low = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_inner_thresh_low_inc_steps_key)
	{
		init_param->agc_inner_thresh_low_inc_steps = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_lmt_overload_large_exceed_counter_key)
	{
		init_param->agc_lmt_overload_large_exceed_counter = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_lmt_overload_large_inc_steps_key)
	{
		init_param->agc_lmt_overload_large_inc_steps = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_lmt_overload_small_exceed_counter_key)
	{
		init_param->agc_lmt_overload_small_exceed_counter = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_outer_thresh_high_key)
	{
		init_param->agc_outer_thresh_high = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_outer_thresh_high_dec_steps_key)
	{
		init_param->agc_outer_thresh_high_dec_steps = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_outer_thresh_low_key)
	{
		init_param->agc_outer_thresh_low = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_outer_thresh_low_inc_steps_key)
	{
		init_param->agc_outer_thresh_low_inc_steps = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_attack_delay_extra_margin_us_key)
	{
		init_param->agc_attack_delay_extra_margin_us = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == agc_sync_for_gain_counter_enable_key)
	{
		init_param->agc_sync_for_gain_counter_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* Fast AGC */
	else if (*key == fagc_dec_pow_measuremnt_duration_key)
	{
		init_param->fagc_dec_pow_measuremnt_duration = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_state_wait_time_ns_key)
	{
		init_param->fagc_state_wait_time_ns = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* Fast AGC - Low Power */
	else if (*key == fagc_allow_agc_gain_increase_key)
	{
		init_param->fagc_allow_agc_gain_increase = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_lp_thresh_increment_time_key)
	{
		init_param->fagc_lp_thresh_increment_time = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_lp_thresh_increment_steps_key)
	{
		init_param->fagc_lp_thresh_increment_steps = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* Fast AGC - Lock Level */
	else if (*key == fagc_lock_level_lmt_gain_increase_en_key)
	{
		init_param->fagc_lock_level_lmt_gain_increase_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_lock_level_gain_increase_upper_limit_key)
	{
		init_param->fagc_lock_level_gain_increase_upper_limit = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* Fast AGC - Peak Detectors and Final Settling */
	else if (*key == fagc_lpf_final_settling_steps_key)
	{
		init_param->fagc_lpf_final_settling_steps = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_lmt_final_settling_steps_key)
	{
		init_param->fagc_lmt_final_settling_steps = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_final_overrange_count_key)
	{
		init_param->fagc_final_overrange_count = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* Fast AGC - Final Power Test */
	else if (*key == fagc_gain_increase_after_gain_lock_en_key)
	{
		init_param->fagc_gain_increase_after_gain_lock_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* Fast AGC - Unlocking the Gain */
	else if (*key == fagc_gain_index_type_after_exit_rx_mode_key)
	{
		init_param->fagc_gain_index_type_after_exit_rx_mode = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_use_last_lock_level_for_set_gain_en_key)
	{
		init_param->fagc_use_last_lock_level_for_set_gain_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_rst_gla_stronger_sig_thresh_exceeded_en_key)
	{
		init_param->fagc_rst_gla_stronger_sig_thresh_exceeded_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_optimized_gain_offset_key)
	{
		init_param->fagc_optimized_gain_offset = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_rst_gla_stronger_sig_thresh_above_ll_key)
	{
		init_param->fagc_rst_gla_stronger_sig_thresh_above_ll = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en_key)
	{
		init_param->fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_rst_gla_engergy_lost_goto_optim_gain_en_key)
	{
		init_param->fagc_rst_gla_engergy_lost_goto_optim_gain_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_rst_gla_engergy_lost_sig_thresh_below_ll_key)
	{
		init_param->fagc_rst_gla_engergy_lost_sig_thresh_below_ll = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_energy_lost_stronger_sig_gain_lock_exit_cnt_key)
	{
		init_param->fagc_energy_lost_stronger_sig_gain_lock_exit_cnt = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_rst_gla_large_adc_overload_en_key)
	{
		init_param->fagc_rst_gla_large_adc_overload_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_rst_gla_large_lmt_overload_en_key)
	{
		init_param->fagc_rst_gla_large_lmt_overload_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_rst_gla_en_agc_pulled_high_en_key)
	{
		init_param->fagc_rst_gla_en_agc_pulled_high_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_rst_gla_if_en_agc_pulled_high_mode_key)
	{
		init_param->fagc_rst_gla_if_en_agc_pulled_high_mode = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fagc_power_measurement_duration_in_state5_key)
	{
		init_param->fagc_power_measurement_duration_in_state5 = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* RSSI Control */
	else if (*key == rssi_delay_key)
	{
		init_param->rssi_delay = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rssi_duration_key)
	{
		init_param->rssi_duration = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rssi_restart_mode_key)
	{
		init_param->rssi_restart_mode = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rssi_unit_is_rx_samples_enable_key)
	{
		init_param->rssi_unit_is_rx_samples_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rssi_wait_key)
	{
		init_param->rssi_wait = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* Aux ADC Control */
	else if (*key == aux_adc_decimation_key)
	{
		init_param->aux_adc_decimation = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_adc_rate_key)
	{
		init_param->aux_adc_rate = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* AuxDAC Control */
	else if (*key == aux_dac_manual_mode_enable_key)
	{
		init_param->aux_dac_manual_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac1_default_value_mV_key)
	{
		init_param->aux_dac1_default_value_mV = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac1_active_in_rx_enable_key)
	{
		init_param->aux_dac1_active_in_rx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac1_active_in_tx_enable_key)
	{
		init_param->aux_dac1_active_in_tx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}

	else if (*key == aux_dac1_active_in_alert_enable_key)
	{
		init_param->aux_dac1_active_in_alert_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac1_rx_delay_us_key)
	{
		init_param->aux_dac1_rx_delay_us = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac1_tx_delay_us_key)
	{
		init_param->aux_dac1_tx_delay_us = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac2_default_value_mV_key)
	{
		init_param->aux_dac2_default_value_mV = (uint32_t)strtol(strdup(tk), NULL, 10);
	}

	else if (*key == aux_dac2_active_in_rx_enable_key)
	{
		init_param->aux_dac2_active_in_rx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac2_active_in_tx_enable_key)
	{
		init_param->aux_dac2_active_in_tx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac2_active_in_alert_enable_key)
	{
		init_param->aux_dac2_active_in_alert_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac2_rx_delay_us_key)
	{
		init_param->aux_dac2_rx_delay_us = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == aux_dac2_tx_delay_us_key)
	{
		init_param->aux_dac2_tx_delay_us = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* Temperature Sensor Control */
	else if (*key == temp_sense_decimation_key)
	{
		init_param->temp_sense_decimation = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == temp_sense_measurement_interval_ms_key)
	{
		init_param->temp_sense_measurement_interval_ms = (uint16_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == temp_sense_offset_signed_key)
	{
		init_param->temp_sense_offset_signed = (int8_t)strtol(strdup(tk), NULL, 16);
	}
	else if (*key == temp_sense_periodic_measurement_enable_key)
	{
		init_param->temp_sense_periodic_measurement_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* Control Out Setup */
	else if (*key == ctrl_outs_enable_mask_key)
	{
		init_param->ctrl_outs_enable_mask = (uint8_t)strtol(strdup(tk), NULL, 16);
	}
	else if (*key == ctrl_outs_index_key)
	{
		init_param->ctrl_outs_index = (uint8_t)strtol(strdup(tk), NULL, 16);
	}
	/* External LNA Control */
	else if (*key == elna_settling_delay_ns_key)
	{
		init_param->elna_settling_delay_ns = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == elna_gain_mdB_key)
	{
		init_param->elna_gain_mdB = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == elna_bypass_loss_mdB_key)
	{
		init_param->elna_bypass_loss_mdB = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == elna_rx1_gpo0_control_enable_key)
	{
		init_param->elna_rx1_gpo0_control_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == elna_rx2_gpo1_control_enable_key)
	{
		init_param->elna_rx2_gpo1_control_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == elna_gaintable_all_index_enable_key)
	{
		init_param->elna_gaintable_all_index_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* Digital Interface Control */
	else if (*key == digital_interface_tune_skip_mode_key)
	{
		init_param->digital_interface_tune_skip_mode = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == digital_interface_tune_fir_disable_key)
	{
		init_param->digital_interface_tune_fir_disable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == pp_tx_swap_enable_key)
	{
		init_param->pp_tx_swap_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == pp_rx_swap_enable_key)
	{
		init_param->pp_rx_swap_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx_channel_swap_enable_key)
	{
		init_param->tx_channel_swap_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rx_channel_swap_enable_key)
	{
		init_param->rx_channel_swap_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rx_frame_pulse_mode_enable_key)
	{
		init_param->rx_frame_pulse_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == two_t_two_r_timing_enable_key)
	{
		init_param->two_t_two_r_timing_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == invert_data_bus_enable_key)
	{
		init_param->invert_data_bus_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == invert_data_clk_enable_key)
	{
		init_param->invert_data_clk_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fdd_alt_word_order_enable_key)
	{
		init_param->fdd_alt_word_order_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == invert_rx_frame_enable_key)
	{
		init_param->invert_rx_frame_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == fdd_rx_rate_2tx_enable_key)
	{
		init_param->fdd_rx_rate_2tx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == swap_ports_enable_key)
	{
		init_param->swap_ports_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == single_data_rate_enable_key)
	{
		init_param->single_data_rate_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == lvds_mode_enable_key)
	{
		init_param->lvds_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == half_duplex_mode_enable_key)
	{
		init_param->half_duplex_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == single_port_mode_enable_key)
	{
		init_param->single_port_mode_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == full_port_enable_key)
	{
		init_param->full_port_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == full_duplex_swap_bits_enable_key)
	{
		init_param->full_duplex_swap_bits_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == delay_rx_data_key)
	{
		init_param->delay_rx_data = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rx_data_clock_delay_key)
	{
		init_param->rx_data_clock_delay = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rx_data_delay_key)
	{
		init_param->rx_data_delay = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx_fb_clock_delay_key)
	{
		init_param->tx_fb_clock_delay = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx_data_delay_key)
	{
		init_param->tx_data_delay = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == lvds_bias_mV_key)
	{
		init_param->lvds_bias_mV = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == lvds_rx_onchip_termination_enable_key)
	{
		init_param->lvds_rx_onchip_termination_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == rx1rx2_phase_inversion_en_key)
	{
		init_param->rx1rx2_phase_inversion_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == lvds_invert1_control_key)
	{
		init_param->lvds_invert1_control = (uint8_t)strtol(strdup(tk), NULL, 16);
	}
	else if (*key == lvds_invert2_control_key)
	{
		init_param->lvds_invert2_control = (uint8_t)strtol(strdup(tk), NULL, 16);
	}
	/* GPO Control */
	else if (*key == gpo0_inactive_state_high_enable_key)
	{
		init_param->gpo0_inactive_state_high_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo1_inactive_state_high_enable_key)
	{
		init_param->gpo1_inactive_state_high_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo2_inactive_state_high_enable_key)
	{
		init_param->gpo2_inactive_state_high_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo3_inactive_state_high_enable_key)
	{
		init_param->gpo3_inactive_state_high_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo0_slave_rx_enable_key)
	{
		init_param->gpo0_slave_rx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo0_slave_tx_enable_key)
	{
		init_param->gpo0_slave_tx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo1_slave_rx_enable_key)
	{
		init_param->gpo1_slave_rx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo1_slave_tx_enable_key)
	{
		init_param->gpo1_slave_tx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo2_slave_rx_enable_key)
	{
		init_param->gpo2_slave_rx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo2_slave_tx_enable_key)
	{
		init_param->gpo2_slave_tx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo3_slave_rx_enable_key)
	{
		init_param->gpo3_slave_rx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo3_slave_tx_enable_key)
	{
		init_param->gpo3_slave_tx_enable = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo0_rx_delay_us_key)
	{
		init_param->gpo0_rx_delay_us = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo0_tx_delay_us_key)
	{
		init_param->gpo0_tx_delay_us = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo1_rx_delay_us_key)
	{
		init_param->gpo1_rx_delay_us = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo1_tx_delay_us_key)
	{
		init_param->gpo1_tx_delay_us = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo2_rx_delay_us_key)
	{
		init_param->gpo2_rx_delay_us = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo2_tx_delay_us_key)
	{
		init_param->gpo2_tx_delay_us = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo3_rx_delay_us_key)
	{
		init_param->gpo3_rx_delay_us = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpo3_tx_delay_us_key)
	{
		init_param->gpo3_tx_delay_us = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	/* Tx Monitor Control */
	else if (*key == low_high_gain_threshold_mdB_key)
	{
		init_param->low_high_gain_threshold_mdB = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == low_gain_dB_key)
	{
		init_param->low_gain_dB = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == high_gain_dB_key)
	{
		init_param->high_gain_dB = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx_mon_track_en_key)
	{
		init_param->tx_mon_track_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == one_shot_mode_en_key)
	{
		init_param->one_shot_mode_en = (uint8_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx_mon_delay_key)
	{
		init_param->tx_mon_delay = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx_mon_duration_key)
	{
		init_param->tx_mon_duration = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx1_mon_front_end_gain_key)
	{
		init_param->tx1_mon_front_end_gain = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx2_mon_front_end_gain_key)
	{
		init_param->tx2_mon_front_end_gain = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx1_mon_lo_cm_key)
	{
		init_param->tx1_mon_lo_cm = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == tx2_mon_lo_cm_key)
	{
		init_param->tx2_mon_lo_cm = (uint32_t)strtol(strdup(tk), NULL, 10);
	}
	/* GPIO definitions */
	else if (*key == gpio_resetb_key)
	{
		init_param->gpio_resetb = (int32_t)strtol(strdup(tk), NULL, 10);
	}
	/* MCS Sync */
	else if (*key == gpio_sync_key)
	{
		init_param->gpio_sync = (int32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpio_cal_sw1_key)
	{
		init_param->gpio_cal_sw1 = (int32_t)strtol(strdup(tk), NULL, 10);
	}
	else if (*key == gpio_cal_sw2_key)
	{
		init_param->gpio_cal_sw2 = (int32_t)strtol(strdup(tk), NULL, 10);
	}
	else
	{
		/* printf("Unrecognised value: %s\n", tk); */
	}
}
