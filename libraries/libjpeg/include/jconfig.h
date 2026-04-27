/*
 * jconfig.h
 *
 * Hand-crafted configuration for Xbox 360 XDK (PowerPC, big-endian).
 * libjpeg-turbo 3.1.4
 *
 * Generated from jconfig.h.in with appropriate values for the platform.
 */

/* Version ID for the JPEG library.
 * Might be useful for tests like "#if JPEG_LIB_VERSION >= 60".
 */
#define JPEG_LIB_VERSION  62

/* libjpeg-turbo version */
#define LIBJPEG_TURBO_VERSION  "3.1.4"

/* libjpeg-turbo version in integer form */
#define LIBJPEG_TURBO_VERSION_NUMBER  3001004

/* Support arithmetic encoding when using 8-bit samples */
#define C_ARITH_CODING_SUPPORTED  1

/* Support arithmetic decoding when using 8-bit samples */
#define D_ARITH_CODING_SUPPORTED  1

/* Support in-memory source/destination managers */
#define MEM_SRCDST_SUPPORTED  1

/* Do NOT use SIMD - no PowerPC SIMD support in libjpeg-turbo */
/* #undef WITH_SIMD */

/* Data precision */
#ifndef BITS_IN_JSAMPLE
#define BITS_IN_JSAMPLE  8
#endif

/* Xbox 360 XDK platform types - always active (no #ifdef guard).
 * The XDK C compiler does not define _WIN32, but we still need these
 * type overrides for consistent struct layout between C and C++ code.
 */
#undef RIGHT_SHIFT_IS_UNSIGNED

/* Define "boolean" as unsigned char, not int, for consistent ABI */
#ifndef __RPCNDR_H__
typedef unsigned char boolean;
#endif
#define HAVE_BOOLEAN

/* Define "INT32" as int, not long */
#if !(defined(_BASETSD_H_) || defined(_BASETSD_H))
typedef short INT16;
typedef signed int INT32;
#endif
#define XMD_H

/*
 * Symbol prefix to avoid collisions with the XDK's built-in libjpeg
 * in xmedia2.lib / xmediad2.lib (same approach as OG xbmc4xbox's tb_ prefix).
 */
#define jpeg_std_error            xb_jpeg_std_error
#define jpeg_CreateCompress       xb_jpeg_CreateCompress
#define jpeg_CreateDecompress     xb_jpeg_CreateDecompress
#define jpeg_destroy_compress     xb_jpeg_destroy_compress
#define jpeg_destroy_decompress   xb_jpeg_destroy_decompress
#define jpeg_stdio_dest           xb_jpeg_stdio_dest
#define jpeg_stdio_src            xb_jpeg_stdio_src
#define jpeg_mem_dest             xb_jpeg_mem_dest
#define jpeg_mem_src              xb_jpeg_mem_src
#define jpeg_set_defaults         xb_jpeg_set_defaults
#define jpeg_set_colorspace       xb_jpeg_set_colorspace
#define jpeg_default_colorspace   xb_jpeg_default_colorspace
#define jpeg_set_quality          xb_jpeg_set_quality
#define jpeg_set_linear_quality   xb_jpeg_set_linear_quality
#define jpeg_default_qtables      xb_jpeg_default_qtables
#define jpeg_add_quant_table      xb_jpeg_add_quant_table
#define jpeg_quality_scaling      xb_jpeg_quality_scaling
#define jpeg_enable_lossless      xb_jpeg_enable_lossless
#define jpeg_simple_progression   xb_jpeg_simple_progression
#define jpeg_suppress_tables      xb_jpeg_suppress_tables
#define jpeg_alloc_quant_table    xb_jpeg_alloc_quant_table
#define jpeg_alloc_huff_table     xb_jpeg_alloc_huff_table
#define jpeg_start_compress       xb_jpeg_start_compress
#define jpeg_write_scanlines      xb_jpeg_write_scanlines
#define jpeg_finish_compress      xb_jpeg_finish_compress
#define jpeg_calc_jpeg_dimensions xb_jpeg_calc_jpeg_dimensions
#define jpeg_write_raw_data       xb_jpeg_write_raw_data
#define jpeg_write_marker         xb_jpeg_write_marker
#define jpeg_write_m_header       xb_jpeg_write_m_header
#define jpeg_write_m_byte         xb_jpeg_write_m_byte
#define jpeg_write_tables         xb_jpeg_write_tables
#define jpeg_write_icc_profile    xb_jpeg_write_icc_profile
#define jpeg_read_header          xb_jpeg_read_header
#define jpeg_start_decompress     xb_jpeg_start_decompress
#define jpeg_read_scanlines       xb_jpeg_read_scanlines
#define jpeg_skip_scanlines       xb_jpeg_skip_scanlines
#define jpeg_crop_scanline        xb_jpeg_crop_scanline
#define jpeg_finish_decompress    xb_jpeg_finish_decompress
#define jpeg_read_raw_data        xb_jpeg_read_raw_data
#define jpeg_has_multiple_scans   xb_jpeg_has_multiple_scans
#define jpeg_start_output         xb_jpeg_start_output
#define jpeg_finish_output        xb_jpeg_finish_output
#define jpeg_input_complete       xb_jpeg_input_complete
#define jpeg_new_colormap         xb_jpeg_new_colormap
#define jpeg_consume_input        xb_jpeg_consume_input
#define jpeg_core_output_dimensions  xb_jpeg_core_output_dimensions
#define jpeg_calc_output_dimensions  xb_jpeg_calc_output_dimensions
#define jpeg_save_markers         xb_jpeg_save_markers
#define jpeg_set_marker_processor xb_jpeg_set_marker_processor
#define jpeg_read_coefficients    xb_jpeg_read_coefficients
#define jpeg_write_coefficients   xb_jpeg_write_coefficients
#define jpeg_copy_critical_parameters  xb_jpeg_copy_critical_parameters
#define jpeg_abort_compress       xb_jpeg_abort_compress
#define jpeg_abort_decompress     xb_jpeg_abort_decompress
#define jpeg_abort                xb_jpeg_abort
#define jpeg_destroy              xb_jpeg_destroy
#define jpeg_resync_to_restart    xb_jpeg_resync_to_restart
#define jpeg_read_icc_profile     xb_jpeg_read_icc_profile
/* 12/16-bit variants */
#define jpeg12_write_scanlines    xb_jpeg12_write_scanlines
#define jpeg16_write_scanlines    xb_jpeg16_write_scanlines
#define jpeg12_write_raw_data     xb_jpeg12_write_raw_data
#define jpeg12_read_scanlines     xb_jpeg12_read_scanlines
#define jpeg16_read_scanlines     xb_jpeg16_read_scanlines
#define jpeg12_skip_scanlines     xb_jpeg12_skip_scanlines
#define jpeg12_crop_scanline      xb_jpeg12_crop_scanline
#define jpeg12_read_raw_data      xb_jpeg12_read_raw_data
/* Internal global data symbols (also in xmediad2.lib) */
#define jpeg_natural_order        xb_jpeg_natural_order
#define jpeg_aritab               xb_jpeg_aritab
/* Internal utility functions */
#define jcopy_block_row           xb_jcopy_block_row
#define jcopy_sample_rows         xb_jcopy_sample_rows
#define jdiv_round_up             xb_jdiv_round_up
#define jround_up                 xb_jround_up
#define jinit_memory_mgr          xb_jinit_memory_mgr
