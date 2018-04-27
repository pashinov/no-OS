/***************************************************************************//**
 *   @file   ad9361_get_config.h
 *   @brief  AD9361 API Driver.
 *   @author A.Pashinov (pashinov@outlook.com)
********************************************************************************
*******************************************************************************/
#ifndef AD9361_GET_CONFIG_H_
#define AD9361_GET_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <inttypes.h>
#include "ad9361_api.h"

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
#define MAX_NUM_TAPS_INTERPOLATION_1		64
#define MAX_NUM_TAPS_INTERPOLATION_2		128

#define AD9361_CONF_FILE	"/etc/ad9361/config.yaml"
#define AD9361_CONF_FIR_FILE	"/etc/ad9361/ad9361_fir.conf"

enum keys {	/* Identification number */
			id_no_key = 1,

			/* Reference Clock */
			reference_clk_rate_key,

			/* Base Configuration */
			two_rx_two_tx_mode_enable_key,
			one_rx_one_tx_mode_use_rx_num_key,
			one_rx_one_tx_mode_use_tx_num_key,
			frequency_division_duplex_mode_enable_key,
			frequency_division_duplex_independent_mode_enable_key,
			tdd_use_dual_synth_mode_enable_key,
			tdd_skip_vco_cal_enable_key,
			tx_fastlock_delay_ns_key,
			rx_fastlock_delay_ns_key,
			rx_fastlock_pincontrol_enable_key,
			tx_fastlock_pincontrol_enable_key,
			external_rx_lo_enable_key,
			external_tx_lo_enable_key,
			dc_offset_tracking_update_event_mask_key,
			dc_offset_attenuation_high_range_key,
			dc_offset_attenuation_low_range_key,
			dc_offset_count_high_range_key,
			dc_offset_count_low_range_key,
			split_gain_table_mode_enable_key,
			trx_synthesizer_target_fref_overwrite_hz_key,
			qec_tracking_slow_mode_enable_key,

			/* ENSM Control */
			ensm_enable_pin_pulse_mode_enable_key,
			ensm_enable_txnrx_control_enable_key,

			/* LO Control */
			rx_synthesizer_frequency_hz_key,
			tx_synthesizer_frequency_hz_key,

			/* RF Port Control */
			rx_rf_port_input_select_key,
			tx_rf_port_input_select_key,

			/* TX Attenuation Control */
			tx_attenuation_mdB_key,
			update_tx_gain_in_alert_enable_key,

			/* Reference Clock Control */
			xo_disable_use_ext_refclk_enable_key,
			dcxo_coarse_and_fine_tune_0_key,
			dcxo_coarse_and_fine_tune_1_key,
			clk_output_mode_select_key,

			/* Gain Control */
			gc_rx1_mode_key,
			gc_rx2_mode_key,
			gc_adc_large_overload_thresh_key,
			gc_adc_ovr_sample_size_key,
			gc_adc_small_overload_thresh_key,
			gc_dec_pow_measurement_duration_key,
			gc_dig_gain_enable_key,
			gc_lmt_overload_high_thresh_key,
			gc_lmt_overload_low_thresh_key,
			gc_low_power_thresh_key,
			gc_max_dig_gain_key,

			/* Gain MGC Control */
			mgc_dec_gain_step_key,
			mgc_inc_gain_step_key,
			mgc_rx1_ctrl_inp_enable_key,
			mgc_rx2_ctrl_inp_enable_key,
			mgc_split_table_ctrl_inp_gain_mode_key,

			/* Gain AGC Control */
			agc_adc_large_overload_exceed_counter_key,
			agc_adc_large_overload_inc_steps_key,
			agc_adc_lmt_small_overload_prevent_gain_inc_enable_key,
			agc_adc_small_overload_exceed_counter_key,
			agc_dig_gain_step_size_key,
			agc_dig_saturation_exceed_counter_key,
			agc_gain_update_interval_us_key,
			agc_immed_gain_change_if_large_adc_overload_enable_key,
			agc_immed_gain_change_if_large_lmt_overload_enable_key,
			agc_inner_thresh_high_key,
			agc_inner_thresh_high_dec_steps_key,
			agc_inner_thresh_low_key,
			agc_inner_thresh_low_inc_steps_key,
			agc_lmt_overload_large_exceed_counter_key,
			agc_lmt_overload_large_inc_steps_key,
			agc_lmt_overload_small_exceed_counter_key,
			agc_outer_thresh_high_key,
			agc_outer_thresh_high_dec_steps_key,
			agc_outer_thresh_low_key,
			agc_outer_thresh_low_inc_steps_key,
			agc_attack_delay_extra_margin_us_key,
			agc_sync_for_gain_counter_enable_key,

			/* Fast AGC */
			fagc_dec_pow_measuremnt_duration_key,
			fagc_state_wait_time_ns_key,

			/* Fast AGC - Low Power */
			fagc_allow_agc_gain_increase_key,
			fagc_lp_thresh_increment_time_key,
			fagc_lp_thresh_increment_steps_key,

			/* Fast AGC - Lock Level */
			fagc_lock_level_lmt_gain_increase_en_key,
			fagc_lock_level_gain_increase_upper_limit_key,

			/* Fast AGC - Peak Detectors and Final Settling */
			fagc_lpf_final_settling_steps_key,
			fagc_lmt_final_settling_steps_key,
			fagc_final_overrange_count_key,

			/* Fast AGC - Final Power Test */
			fagc_gain_increase_after_gain_lock_en_key,

			/* Fast AGC - Unlocking the Gain */
			fagc_gain_index_type_after_exit_rx_mode_key,
			fagc_use_last_lock_level_for_set_gain_en_key,
			fagc_rst_gla_stronger_sig_thresh_exceeded_en_key,
			fagc_optimized_gain_offset_key,
			fagc_rst_gla_stronger_sig_thresh_above_ll_key,
			fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en_key,
			fagc_rst_gla_engergy_lost_goto_optim_gain_en_key,
			fagc_rst_gla_engergy_lost_sig_thresh_below_ll_key,
			fagc_energy_lost_stronger_sig_gain_lock_exit_cnt_key,
			fagc_rst_gla_large_adc_overload_en_key,
			fagc_rst_gla_large_lmt_overload_en_key,
			fagc_rst_gla_en_agc_pulled_high_en_key,
			fagc_rst_gla_if_en_agc_pulled_high_mode_key,
			fagc_power_measurement_duration_in_state5_key,

			/* RSSI Control */
			rssi_delay_key,
			rssi_duration_key,
			rssi_restart_mode_key,
			rssi_unit_is_rx_samples_enable_key,
			rssi_wait_key,

			/* Aux ADC Control */
			aux_adc_decimation_key,
			aux_adc_rate_key,

			/* AuxDAC Control */
			aux_dac_manual_mode_enable_key,
			aux_dac1_default_value_mV_key,
			aux_dac1_active_in_rx_enable_key,
			aux_dac1_active_in_tx_enable_key,
			aux_dac1_active_in_alert_enable_key,
			aux_dac1_rx_delay_us_key,
			aux_dac1_tx_delay_us_key,
			aux_dac2_default_value_mV_key,
			aux_dac2_active_in_rx_enable_key,
			aux_dac2_active_in_tx_enable_key,
			aux_dac2_active_in_alert_enable_key,
			aux_dac2_rx_delay_us_key,
			aux_dac2_tx_delay_us_key,

			/* Temperature Sensor Control */
			temp_sense_decimation_key,
			temp_sense_measurement_interval_ms_key,
			temp_sense_offset_signed_key,
			temp_sense_periodic_measurement_enable_key,

			/* Control Out Setup */
			ctrl_outs_enable_mask_key,
			ctrl_outs_index_key,

			/* External LNA Control */
			elna_settling_delay_ns_key,
			elna_gain_mdB_key,
			elna_bypass_loss_mdB_key,
			elna_rx1_gpo0_control_enable_key,
			elna_rx2_gpo1_control_enable_key,
			elna_gaintable_all_index_enable_key,

			/* Digital Interface Control */
			digital_interface_tune_skip_mode_key,
			digital_interface_tune_fir_disable_key,
			pp_tx_swap_enable_key,
			pp_rx_swap_enable_key,
			tx_channel_swap_enable_key,
			rx_channel_swap_enable_key,
			rx_frame_pulse_mode_enable_key,
			two_t_two_r_timing_enable_key,
			invert_data_bus_enable_key,
			invert_data_clk_enable_key,
			fdd_alt_word_order_enable_key,
			invert_rx_frame_enable_key,
			fdd_rx_rate_2tx_enable_key,
			swap_ports_enable_key,
			single_data_rate_enable_key,
			lvds_mode_enable_key,
			half_duplex_mode_enable_key,
			single_port_mode_enable_key,
			full_port_enable_key,
			full_duplex_swap_bits_enable_key,
			delay_rx_data_key,
			rx_data_clock_delay_key,
			rx_data_delay_key,
			tx_fb_clock_delay_key,
			tx_data_delay_key,
			lvds_bias_mV_key,
			lvds_rx_onchip_termination_enable_key,
			rx1rx2_phase_inversion_en_key,
			lvds_invert1_control_key,
			lvds_invert2_control_key,

			/* GPO Control */
			gpo0_inactive_state_high_enable_key,
			gpo1_inactive_state_high_enable_key,
			gpo2_inactive_state_high_enable_key,
			gpo3_inactive_state_high_enable_key,
			gpo0_slave_rx_enable_key,
			gpo0_slave_tx_enable_key,
			gpo1_slave_rx_enable_key,
			gpo1_slave_tx_enable_key,
			gpo2_slave_rx_enable_key,
			gpo2_slave_tx_enable_key,
			gpo3_slave_rx_enable_key,
			gpo3_slave_tx_enable_key,
			gpo0_rx_delay_us_key,
			gpo0_tx_delay_us_key,
			gpo1_rx_delay_us_key,
			gpo1_tx_delay_us_key,
			gpo2_rx_delay_us_key,
			gpo2_tx_delay_us_key,
			gpo3_rx_delay_us_key,
			gpo3_tx_delay_us_key,

			/* Tx Monitor Control */
			low_high_gain_threshold_mdB_key,
			low_gain_dB_key,
			high_gain_dB_key,
			tx_mon_track_en_key,
			one_shot_mode_en_key,
			tx_mon_delay_key,
			tx_mon_duration_key,
			tx1_mon_front_end_gain_key,
			tx2_mon_front_end_gain_key,
			tx1_mon_lo_cm_key,
			tx2_mon_lo_cm_key,

			/* GPIO definitions */
			gpio_resetb_key,

			/* MCS Sync */
			gpio_sync_key,
			gpio_cal_sw1_key,
			gpio_cal_sw2_key
		};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Get user configuration for ad9361 from config.yaml file */
int32_t ad9361_get_config(AD9361_InitParam *init_param);

/* Get user configuration fir filter for ad9361 from ad9361_fir.conf file*/
int32_t ad9361_get_config_fir(AD9361_InitParam *init_param, AD9361_TXFIRConfig *tx, AD9361_RXFIRConfig *rx);

/* Parsing keys */
void parse_keys(char *tk, uint32_t *key);

/* Filling structure AD9361_InitParam */
void fill_ad9361_st(AD9361_InitParam *init_param, char *tk, uint32_t *key);

#endif //AD9361_GET_CONFIG_H_
